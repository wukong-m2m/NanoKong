//# vim: ts=2 sw=2
#include <eeprom.h>
#include <string.h>
#include <stdio.h>
#include "types.h"
#include "delay.h"
#include "nvmcomm.h"
#include "debug.h"
#include "logging.h"
#include "group.h"
#include "led.h"
#include "vm.h"
#include "wkpf_config.h"
#include "wkpf_links.h"

#ifdef NVM_USE_GROUP
//////////

// Update command
#define GROUP_COMPONENT_INSERT 0x06
#define GROUP_COMPONENT_REMOVE 0x07

// Timeout
#define HEARTBEAT_INTERVAL 1000
#define HEARTBEAT_TIMEOUT 2500
#define INITIALISATION_TIMEOUT 15000
#define NOTIFY_TIMEOUT 30000
#define JITTERING_TIMEOUT 1000

#define MAX_NUMBER_OF_WATCHED_NODES 10

typedef struct {
    address_t node_id;
    nvmtime_t expect_next_timestamp_before; // Initialise to currenttime + INITIALISATION_TIMEOUT
} node_to_watch;

bool necessary_for_reconfiguration = false;
nvmtime_t next_time_to_notify = 0;
node_to_watch watch_list[MAX_NUMBER_OF_WATCHED_NODES];
uint8_t watch_list_count;
nvmtime_t next_heartbeat_broadcast = 0; // Initialise to 0 to start sending heartbeats straight away.

void group_update_component_for_all_endpoints(uint8_t cmd, int position, address_t node_id, uint8_t component_id, remote_endpoints* component) {
    /* Reliable Multicast or Unicasts */
    for (int i=0; i<component->number_of_endpoints; ++i) {
        address_t dest = component->endpoints[i].node_id;
        // TODO: send!
        uint8_t message[NVMCOMM_MESSAGE_SIZE];
        set_message_sequence_number(message, NULL);
        message[2] = cmd;
        message[3] = component_id;
        message[4] = position;
        message[5] = node_id;
        nvmcomm_send(dest, NVMCOMM_GROUP_UPDATE_COMPONENT, message, 6);
    }
}

// To be called periodically (at least as often as HEARTBEAT_INTERVAL)
void send_heartbeat() {
    if (watch_list_count > 0) {
        // Send a heartbeat if it is due.
        if (nvm_current_time > next_heartbeat_broadcast) {
#ifdef DEBUG
            DEBUGF_GROUP("sending heartbeat\n");
#endif
            /*nvmcomm_broadcast(NVMCOMM_GROUP_HEARTBEAT, NULL, 0);*/
            for(uint8_t i=0; i<watch_list_count; i++) {
                nvmcomm_send(watch_list[i].node_id, NVMCOMM_GROUP_HEARTBEAT, NULL, 0);
            }
            next_heartbeat_broadcast = nvm_current_time + HEARTBEAT_INTERVAL;
            blink_once(LED5);
        }
    }
}

void handle_failure() {
    // Check all nodes we're supposed to watch to see if we've received a heartbeat in the last HEARTBEAT_TIMEOUT ms.
    for(uint8_t i=0; i<watch_list_count; i++) {
        if (nvm_current_time > watch_list[i].expect_next_timestamp_before) {
            // Tell the master we didn't receive the heartbeat in time
            address_t master_node_id = wkpf_config_get_master_node_id();
            // Do we need a reply here? Maybe not for now. If the message isn't received, it will be sent again after a second.
            // For future versions we may want to stop sending when we know the master got the message, but since we're going
            // to do a full reconfiguration anyway, it doesn't really matter for now.
            if (next_time_to_notify + JITTERING_TIMEOUT <= nvm_current_time) {

#ifdef LOGGING
                char message[25];
                uint8_t n = sprintf(message, "node %d was suspected failure", watch_list[i].node_id);
                LOGF_GROUP(message, n);
#endif

                // Assuming the node is surly dead by this point
                // Update all nodes in recovery chains that were on the dead node
                remote_endpoints* component;
                int component_id = 0;
                while (wkpf_get_component_for_node(watch_list[i].node_id, component_id, &component)) {
                    for (int j=0; j<component->number_of_endpoints; ++j) {
                        if (component->endpoints[j].node_id == watch_list[i].node_id) {
                            // Remove
                            wkpf_remove_endpoint_from_component(j, component);

                            // Notify and update nodes that contains this
                            // component
                            group_update_component_for_all_endpoints(GROUP_COMPONENT_REMOVE, j, watch_list[i].node_id, component_id, component);
                        }
                    }

                    component_id++;
                }

                // Update and repair application link (not necessary given our
                // link table implementation)

                // Repair heartbeat group and watchlist

                // Notify master if necessary
                if (necessary_for_reconfiguration) {
                    nvmcomm_send(master_node_id, NVMCOMM_GROUP_NOTIFY_NODE_FAILURE, &watch_list[i].node_id, sizeof(address_t));
#ifdef DEBUG
                    DEBUGF_GROUP("notify master the failure of node %x\n", watch_list[i].node_id);
#endif
                }

                // Update time
                next_time_to_notify = nvm_current_time + NOTIFY_TIMEOUT;

                // Blink
                blink_twice(LED5);
            }
        }
    }
}

void group_handle_update_component(address_t src, u08_t nvmcomm_command, u08_t *payload, u08_t *response_size, u08_t *response_cmd) {
    uint8_t operation = (uint8_t)payload[2];
    uint8_t component_id = (uint8_t)payload[3];
    uint8_t position = (uint8_t)payload[4];
    address_t node_id = (address_t)payload[5];

    /* TODO: create a new wuobject for insert */
    uint8_t port_number = 0;
    remote_endpoint endpoint = {node_id, port_number};

    switch (operation) {
        case GROUP_COMPONENT_INSERT:
            wkpf_insert_endpoint_for_component_id(endpoint, position, component_id);
            *response_cmd = NVMCOMM_GROUP_UPDATE_COMPONENT_R;
            *response_size = 2;
        break;
        case GROUP_COMPONENT_REMOVE:
            wkpf_remove_endpoint_from_component_id(position, component_id);
            *response_cmd = NVMCOMM_GROUP_UPDATE_COMPONENT_R;
            *response_size = 2;
        break;
    }
}

void group_handle_heartbeat_message(address_t src) {
    for(uint8_t i=0; i<watch_list_count; i++) {
        if (watch_list[i].node_id == src) {
#ifdef DEBUG
            DEBUGF_GROUP("handling heartbeat message from node %x\n", src);
#endif
            watch_list[i].expect_next_timestamp_before = nvm_current_time + HEARTBEAT_TIMEOUT;
        }
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
//////////
#endif // NVM_USE_GROUP
