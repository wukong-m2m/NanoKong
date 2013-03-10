//# vim: ts=2 sw=2
#include <eeprom.h>
#include <string.h>
#include <stdio.h>
#include "wkpf.h"
#include "types.h"
#include "delay.h"
#include "nvmcomm.h"
#include "debug.h"
#include "heap.h"
#include "array.h"
#include "logging.h"
#include "group.h"
#include "led.h"
#include "vm.h"
#include "wkpf_config.h"
#include "wkpf_links.h"

#ifdef NVM_USE_GROUP
//////////

// Update command
#define GROUP_COMPONENT_NODE_REMOVE 0x06
#define GROUP_HEARTBEAT_NODE_REMOVE 0x07

// Timeout
#define INITIALISATION_TIMEOUT 15000
#define NOTIFY_TIMEOUT 10000

#define MAX_NUMBER_OF_WATCHED_NODES 10

typedef struct {
    address_t node_id;
    nvmtime_t expect_next_timestamp_before; // Initialise to currenttime + INITIALISATION_TIMEOUT
} node_to_watch;

uint32_t heartbeat_interval = 1000;
uint32_t heartbeat_timeout = 2500;
nvmtime_t next_time_to_notify = 0;
// Watch list is now a list of nodes in order of heartbeat importance
// The node at left_offset_to_monitor is the node to monitor
// The node at right_offset_to_send_heartbeat is the node to send heartbeat to
node_to_watch watch_list[MAX_NUMBER_OF_WATCHED_NODES];
uint8_t watch_list_count;
// This sets the front offset from watch list to monitor
// e.g. 0 will make the first node in watch list the node to monitor and expect
// heartbeat from
uint8_t front_offset_to_monitor = 0;
// This sets the back offset from watch list to send heartbeat to
// e.g. 0 will make the last node in watch list the node to send heartbeat to
uint8_t back_offset_to_send_heartbeat = 0;
nvmtime_t next_heartbeat_broadcast = 0; // Initialise to 0 to start sending heartbeats straight away.
uint8_t group_index = -1;

node_to_watch* monitored_node() {
  if (watch_list_count > 0 && watch_list_count > front_offset_to_monitor) {
    return &watch_list[front_offset_to_monitor];
  }

#ifdef DEBUG
      DEBUGF_GROUP("no node to monitor\n");
      DEBUGF_GROUP("watch_list_count %x front_offset %x\n", watch_list_count, front_offset_to_monitor);
#endif

  return NULL;
}

node_to_watch* monitor_node() {
  if (watch_list_count > 0 && watch_list_count > back_offset_to_send_heartbeat) {
    return &watch_list[watch_list_count-1-back_offset_to_send_heartbeat];
  }

#ifdef DEBUG
      DEBUGF_GROUP("no node to send heartbeat to\n");
      DEBUGF_GROUP("watch_list_count %x back_offset %x\n", watch_list_count, back_offset_to_send_heartbeat);
#endif

  return NULL;
}

void initialize_heartbeat_interval(uint32_t period) {
  heartbeat_interval = period;
  heartbeat_timeout = 2*heartbeat_interval + 500;
  DEBUGF_GROUP("GROUP: heartbeat_interval and timeout are updated to %x, %x\n", heartbeat_interval, heartbeat_timeout);
}

uint8_t group_load_heartbeat_to_node_map(heap_id_t heartbeat_map_heap_id) {
#ifdef NVM_USE_GROUP
  uint16_t number_of_groups = array_length(heartbeat_map_heap_id)/sizeof(heap_id_t);

  DEBUGF_GROUP("GROUP: Scanning %x heartbeat groups (%x bytes)\n\n", number_of_groups, array_length(heartbeat_map_heap_id));

  /* No restrictions on the size of heartbeat group yet
  if (number_of_groups>MAX_NUMBER_OF_COMPONENTS)
    return WKPF_ERR_OUT_OF_MEMORY;
  */

  group_index = -1;
  for(int i=0; i<number_of_groups; i++) {
    heap_id_t nodes_heap_id = *((uint8_t *)heap_get_addr(heartbeat_map_heap_id)+1+(2*i));
    uint16_t number_of_nodes = array_length(nodes_heap_id)/sizeof(address_t);
    address_t *nodes = (address_t *)((uint8_t *)heap_get_addr(nodes_heap_id)+1); // +1 to skip type byte

    DEBUGF_GROUP("GROUP: Scanning heartbeat group with %x nodes\n", number_of_nodes);
    for (int j=0; j<number_of_nodes; j++) {
      if (nodes[j] == nvmcomm_get_node_id()) {
        DEBUGF_GROUP("GROUP: Found itself\n");
        group_index = i;
        group_setup_watch_list(j, number_of_nodes, nodes);
        break;
      }
    }
    if (group_index != -1) {
      break;
    }
  }
#endif // NVM_USE_GROUP
  return WKPF_OK;
}

uint8_t group_load_heartbeat_periods(heap_id_t periods_heap_id) {
  uint16_t number_of_entries = array_length(periods_heap_id)/sizeof(uint32_t);
  uint32_t* periods = (uint32_t *)((uint8_t *)heap_get_addr(periods_heap_id)+1); // +1 to skip type byte
  // TODO: there is some problem with loading heartbeat period, seems 
  // like it is not the same as loading form byte array

  DEBUGF_GROUP("GROUP: Loading %x periods\n\n", number_of_entries);
  
  if (group_index != -1 && group_index < number_of_entries) {
    DEBUGF_GROUP("GROUP: Loading period %x for heartbeat group %x\n",
                periods[group_index],
                group_index
                );
    initialize_heartbeat_interval(periods[group_index]);
  } else {
    return WKPF_ERR_SHOULDNT_HAPPEN;
  }
  return WKPF_OK;
}

uint8_t group_probe_node(address_t node_id) {
  address_t dest = node_id;

  uint8_t message[NVMCOMM_MESSAGE_SIZE];
  set_message_sequence_number(message, NULL);
  if (nvmcomm_send(dest, NVMCOMM_GROUP_PROBE_NODE, message, 2) != 0) {
#ifdef DEBUG
    DEBUGF_GROUP("Probe failed to send to node %x\n", dest);
#endif
    return WKPF_ERR_NVMCOMM_SEND_ERROR;
  }

  // Wait for a reply
  uint32_t timeout = nvm_current_time + 100;
  while(nvm_current_time < timeout) {
    nvmcomm_message *reply = nvmcomm_wait(100, (u08_t[]){NVMCOMM_GROUP_PROBE_NODE+1 /* the reply to this command */, NVMCOMM_GROUP_TRIAGE_R, NVMCOMM_WKPF_ERROR_R}, 3);
    if (reply != NULL // Check sequence number because an old message could be received: the right type, but not the reply to our last sent message
          && check_sequence_number(reply->payload, message)) {
#ifdef DEBUG
      DEBUGF_GROUP("got a reply\n");
#endif
      if (reply->command == NVMCOMM_GROUP_TRIAGE_R) {
        return WKPF_ERR_NETWORK_TRIAGE;
      } else if (reply->command == NVMCOMM_GROUP_PROBE_NODE_R) {
        return WKPF_OK;
      } else {
        return WKPF_ERR_NVMCOMM_SEND_ERROR;
      }
    }
  }

  return WKPF_ERR_NVMCOMM_NO_REPLY;
}

void group_update_component_for_all_other_endpoints(uint8_t cmd, address_t node_id, uint16_t component_id, remote_endpoints* component) {
  /* Reliable Multicast or Unicasts */
  for (int i=0; i<component->number_of_endpoints; ++i) {
    address_t dest = component->endpoints[i].node_id;
    if (dest == nvmcomm_get_node_id()) {
      continue;
    }
    // TODO: send!
    uint8_t message[NVMCOMM_MESSAGE_SIZE];
    set_message_sequence_number(message, NULL);
    message[2] = cmd;
    message[3] = (uint8_t)(component_id >> 8);
    message[4] = (uint8_t)component_id;
    message[5] = node_id;
    if (nvmcomm_send(dest, NVMCOMM_GROUP_UPDATE_COMPONENT, message, 6) != 0) {
#ifdef DEBUG
      DEBUGF_GROUP("cannot send component update message to node %x\n", dest);
#endif
    }
#ifdef DEBUG
    DEBUGF_GROUP("sent component update message to node %x\n", dest);
#endif
    // Wait for a reply
    uint32_t timeout = nvm_current_time + 100;
    while(nvm_current_time < timeout) {
      nvmcomm_message *reply = nvmcomm_wait(100, (u08_t[]){cmd+1 /* the reply to this command */, NVMCOMM_WKPF_ERROR_R}, 2);
      if (reply != NULL // Check sequence number because an old message could be received: the right type, but not the reply to our last sent message
            && check_sequence_number(reply->payload, message)) {
#ifdef DEBUG
        DEBUGF_GROUP("got a reply\n");
#endif
      }
    }
  }
}

void group_update_nodes_in_watchlist(uint8_t cmd, address_t node_id) {
  for (int i=0; i<watch_list_count; ++i) {
    address_t dest = watch_list[i].node_id;
    if (dest == nvmcomm_get_node_id()) {
      continue;
    }
    // TODO: send!
    uint8_t message[NVMCOMM_MESSAGE_SIZE];
    set_message_sequence_number(message, NULL);
    message[2] = cmd;
    message[3] = node_id;
    if (nvmcomm_send(dest, NVMCOMM_GROUP_UPDATE_HEARTBEAT, message, 4) != 0) {
#ifdef DEBUG
      DEBUGF_GROUP("cannot send heartbeat group update message to node %x\n", dest);
#endif
    }
#ifdef DEBUG
    DEBUGF_GROUP("send heartbeat group update message to node %x\n", dest);
#endif
    // Wait for a reply
    uint32_t timeout = nvm_current_time + 100;
    while(nvm_current_time < timeout) {
      nvmcomm_message *reply = nvmcomm_wait(100, (u08_t[]){cmd+1 /* the reply to this command */, NVMCOMM_WKPF_ERROR_R}, 2);
      if (reply != NULL // Check sequence number because an old message could be received: the right type, but not the reply to our last sent message
            && check_sequence_number(reply->payload, message)) {
#ifdef DEBUG
        DEBUGF_GROUP("got a reply\n");
#endif
      }
    }
  }
}

bool group_node_in_watchlist(address_t node) {
  for (int i=0; i<watch_list_count; i++) {
    if (node == watch_list[i].node_id)
      return true;
  }
  return false;
}

void group_handle_probe_message(address_t src, u08_t nvmcomm_command, u08_t *payload, u08_t *response_size, u08_t *response_cmd) {
  if (nvmcomm_command == NVMCOMM_GROUP_PROBE_NODE) {
    if (group_node_in_watchlist(src)) {
      *response_cmd = nvmcomm_command+1;
      *response_size = 2;
    } else {
      *response_cmd = NVMCOMM_GROUP_TRIAGE_R;
      *response_size = 2;
    }
  }
}


void group_handle_update(address_t src, u08_t nvmcomm_command, u08_t *payload, u08_t *response_size, u08_t *response_cmd) {
    uint8_t operation;
    uint8_t component_id;
    address_t node_id;
#ifdef DEBUG
    if (nvmcomm_command == NVMCOMM_GROUP_UPDATE_COMPONENT)
      DEBUGF_GROUP("got component update message from node %x\n", src);
    else if (nvmcomm_command == NVMCOMM_GROUP_UPDATE_HEARTBEAT)
      DEBUGF_GROUP("got heartbeat group update message from node %x\n", src);
#endif

    /* TODO: create a new wuobject for insert */
    /*uint8_t port_number = 0;*/
    /*remote_endpoint endpoint = {node_id, port_number};*/

    switch (nvmcomm_command) {
      case NVMCOMM_GROUP_UPDATE_COMPONENT:
        operation = (uint8_t)payload[2];
        component_id = (uint16_t)(payload[3]<<8) + (uint16_t)(payload[4]);
        node_id = (address_t)payload[5];
        switch (operation) {
          case GROUP_COMPONENT_NODE_REMOVE:
            wkpf_remove_endpoint_from_component(node_id, wkpf_get_component_from_component_id(component_id));
#ifdef DEBUG
            DEBUGF_GROUP("local component id %x now has endpoints\n", component_id);
            for (int i=0; i<wkpf_get_component_from_component_id(component_id)->number_of_endpoints; ++i) {
              DEBUGF_GROUP(" (%x) ", wkpf_get_component_from_component_id(component_id)->endpoints[i].node_id);
            }
            DEBUGF_GROUP("\n");
#endif

            *response_cmd = NVMCOMM_GROUP_UPDATE_COMPONENT_R;
            *response_size = 2;
          break;
        }
      break;
      case NVMCOMM_GROUP_UPDATE_HEARTBEAT:
        operation = (uint8_t)payload[2];
        node_id = (address_t)payload[3];
        switch (operation) {
          case GROUP_HEARTBEAT_NODE_REMOVE:
            group_remove_node_from_watchlist(node_id);
            *response_cmd = NVMCOMM_GROUP_UPDATE_HEARTBEAT_R;
            *response_size = 2;
          break;
        }
      break;
    }

}

void group_handle_heartbeat_message(address_t src) {
#ifdef DEBUG
  DEBUGF_GROUP("got a heartbeat message from node %x\n", src);
#endif

  if (monitored_node()->node_id == src) {
#ifdef DEBUG
    DEBUGF_GROUP("expected\n");
#endif
    monitored_node()->expect_next_timestamp_before = nvm_current_time + heartbeat_timeout;
    blink_once(LED6);
  }
}

// To be called periodically (at least as often as heartbeat_interval)
void send_heartbeat() {
    if (watch_list_count > 0 && monitor_node() != NULL) {
        // Send a heartbeat if it is due.
        if (nvm_current_time > next_heartbeat_broadcast) {
#ifdef DEBUG
            DEBUGF_GROUP("sending heartbeat to %x\n", monitor_node()->node_id);
#endif
            /*nvmcomm_broadcast(NVMCOMM_GROUP_HEARTBEAT, NULL, 0);*/
            nvmcomm_send(monitor_node()->node_id, NVMCOMM_GROUP_HEARTBEAT, NULL, 0);
            next_heartbeat_broadcast = nvm_current_time + heartbeat_interval;
            blink_once(LED5);
        }
    }
    nvmcomm_poll(); // Process incoming messages
}

void group_reconfiguration() {
  // Notify master if necessary
  if (nvm_current_time > next_time_to_notify) {
    address_t master_node_id = wkpf_config_get_master_node_id();
    // Do we need a reply here? Maybe not for now. If the message isn't received, it will be sent again after a second.
    // For future versions we may want to stop sending when we know the master got the message, but since we're going
    // to do a full reconfiguration anyway, it doesn't really matter for now.
    nvmcomm_send(master_node_id, NVMCOMM_GROUP_NOTIFY_NODE_FAILURE, NULL, 0);
#ifdef DEBUG
    DEBUGF_GROUP("reconfiguration msg sent\n");
#endif

    // Slow down Master notification
    next_time_to_notify = nvm_current_time + NOTIFY_TIMEOUT;
  }
}

void handle_failure() {
  // Check all nodes we're supposed to watch to see if we've received a heartbeat in the last heartbeat_timeout ms.
  if (monitored_node() != NULL && nvm_current_time > monitored_node()->expect_next_timestamp_before) {
    address_t dead_node_id = monitored_node()->node_id;

    // A probe, not used
    /*
    uint8_t ret = group_probe_node(dead_node_id);
    if (ret == WKPF_OK) {
      // Like a heartbeat
      monitored_node()->expect_next_timestamp_before = nvm_current_time + heartbeat_timeout;
      return;
    } else if (ret == WKPF_ERR_NETWORK_TRIAGE) {
      // Reconfiguration or do nothing here
      group_reconfiguration();
      return;
    }
    // Otherwise, initiate recovery algorithm
    */


#ifdef DEBUG
    DEBUGF_GROUP("node %x was suspected of failure\n", dead_node_id);
#endif

    // Assuming the node is surly dead by this point
    // Update all nodes in recovery chains that were on the dead node
    remote_endpoints* component;
    uint16_t component_id = 0;
    while (wkpf_get_component_from_component_id(component_id)) {
      nvmcomm_poll(); // Process incoming messages
      if (wkpf_get_component_for_node(dead_node_id, component_id, &component)) {
        // Remove dead node from local component map
        wkpf_remove_endpoint_from_component(dead_node_id, component);

#ifdef DEBUG
        DEBUGF_GROUP("local component id %x now has endpoints\n", component_id);
        for (int i=0; i<component->number_of_endpoints; ++i) {
          DEBUGF_GROUP(" (%x) ", component->endpoints[i].node_id);
        }
        DEBUGF_GROUP("\n");
#endif

#ifdef LOGGING
        char message[40];
        sprintf(message, "component id %x endpoints ", component_id);
        for (int i=0; i<component->number_of_endpoints; ++i) {
          char buf[30];
          sprintf(buf, " %x ", component->endpoints[i].node_id);
          strcat(message, buf);
        }
        LOGF_GROUP(message, strlen(message));
#endif

        // Notify and update nodes that contains this
        // component
        group_update_component_for_all_other_endpoints(GROUP_COMPONENT_NODE_REMOVE, 
            dead_node_id, component_id, component);

        // Update and repair application link (for other node that
        // don't necessary use the component but is connected)
        uint8_t start_from = 0;
        remote_endpoints* connected_component;
        while (wkpf_get_connected_component_for_component(component_id, &start_from, &connected_component)) {
          nvmcomm_poll(); // Process incoming messages

          // Don't need to remove local since it is already been taken cared of
          // in outer loop

          // Notify and update nodes that contains this
          // component
          group_update_component_for_all_other_endpoints(GROUP_COMPONENT_NODE_REMOVE, 
              dead_node_id, component_id, connected_component);
        }
      }

      component_id++;
    }

    // Repair heartbeat group and watchlist
    nvmcomm_poll(); // Process incoming messages
    group_remove_node_from_watchlist(dead_node_id);
    group_update_nodes_in_watchlist(GROUP_HEARTBEAT_NODE_REMOVE, dead_node_id);
    monitored_node()->expect_next_timestamp_before = nvm_current_time + heartbeat_timeout; // Initialization, it's ok to miss one if not lucky

    // Blink
    blink_twice(LED5);
  }
}

void group_remove_node_from_watchlist(address_t node_id) {
  int index = watch_list_count;
  for (int i=0; i<watch_list_count; ++i) {
    if (watch_list[i].node_id == node_id) {
        index = i;
        break;
    }
  }

  if (index < watch_list_count) {
    for (int i=index; i<watch_list_count-1; ++i) {
      watch_list[i] = watch_list[i+1];
    }
    watch_list_count--;

#ifdef DEBUG
    DEBUGF_GROUP("Removing node %x from watchlist\n", node_id);
    DEBUGF_GROUP("watch list count %x\n", watch_list_count);
#endif
  }
}

void group_add_node_to_watch(address_t node_id) {
  if (watch_list_count < MAX_NUMBER_OF_WATCHED_NODES) {
#ifdef DEBUG
    DEBUGF_GROUP("adding node %x to watch\n", node_id);
#endif
    for (uint8_t i=0; i<watch_list_count; i++) {
      if (watch_list[i].node_id == node_id)
        return;
    }
    watch_list[watch_list_count].node_id = node_id;
    watch_list[watch_list_count].expect_next_timestamp_before = nvm_current_time + INITIALISATION_TIMEOUT;
    watch_list_count++;
  }
}

void group_setup_watch_list(uint8_t shift, uint16_t length, address_t* node_ids) {
  if (length > MAX_NUMBER_OF_WATCHED_NODES)
    length = MAX_NUMBER_OF_WATCHED_NODES;

#ifdef DEBUG
  DEBUGF_GROUP("reading from node_ids\n");
  for (int i=0; i<length; ++i) {
    DEBUGF_GROUP(" (id: %x) ", node_ids[i]);
  }
  DEBUGF_GROUP("\n");
#endif


  // Construct buffer without itself
  node_to_watch b[MAX_NUMBER_OF_WATCHED_NODES];
  nvmtime_t expect_next_timestamp_before = nvm_current_time + INITIALISATION_TIMEOUT;
  for (int i=0; i<length; ++i) {
    b[i].node_id = node_ids[i];
    b[i].expect_next_timestamp_before = expect_next_timestamp_before;
  }

  for (int i=shift; i<length-1; ++i) {
    b[i].node_id = b[i+1].node_id;
    b[i].expect_next_timestamp_before = b[i+1].expect_next_timestamp_before;
  }
  watch_list_count = length-1;

#ifdef DEBUG
  DEBUGF_GROUP("reading from b\n");
  for (int i=0; i<watch_list_count; ++i) {
    DEBUGF_GROUP(" (id: %x) ", b[i].node_id);
  }
  DEBUGF_GROUP("\n");
#endif

  // Left Shift
  memcpy(watch_list, &b[shift], (watch_list_count - shift) * sizeof(node_to_watch));
  memcpy(&watch_list[watch_list_count - shift], b, shift * sizeof(node_to_watch));


#ifdef DEBUG
  DEBUGF_GROUP("watch list\n");
  for (int i=0; i<watch_list_count; ++i) {
    DEBUGF_GROUP(" (id: %x) ", watch_list[i].node_id);
  }
  DEBUGF_GROUP("\n");
#endif
}
//////////
#endif // NVM_USE_GROUP
