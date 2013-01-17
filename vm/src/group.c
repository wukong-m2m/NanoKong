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

#ifdef NVM_USE_GROUP

static bool_t group_loaded = false;
static bool_t group_inited = false;
static address_t my_address = NVMCOMM_ADDRESS_NULL;
static u08_t prepared_proposal[NVMCOMM_MESSAGE_SIZE]; // Client of a oracle representative
static group oracle; // GMS service
static group local_groups[MAX_GROUP_LIST_SIZE]; // GMS service

static bool_t EEPROM eeprom_can_be_oracle = false;

static address_t EEPROM eeprom_found_oracle = NVMCOMM_ADDRESS_NULL; // Don't use this directly!
static address_t EEPROM eeprom_client_of = NVMCOMM_ADDRESS_NULL; // Don't use this directly!

static bool_t EEPROM eeprom_has_proposal = false; // Don't use this directly!
static u08_t EEPROM eeprom_prepared_proposal[NVMCOMM_MESSAGE_SIZE]; // Don't use this directly!
static u08_t EEPROM eeprom_prepared_proposal_size = 0; // Don't use this directly!

static group EEPROM eeprom_oracle; // Don't use this directly!
static group EEPROM eeprom_local_groups[MAX_GROUP_LIST_SIZE]; // Don't use this directly!
static u08_t EEPROM eeprom_local_group_size; // Don't use this directly!

// INSIGHT: use update varient of write is to extend lifespan of EEPROM by making a
// check on the content, only write if they differ
#define can_be_oracle (bool_t)eeprom_read_byte((u08_t*)&eeprom_can_be_oracle)
#define update_can_be_oracle(x) eeprom_update_byte((u08_t*)&eeprom_can_be_oracle, (u08_t)x)


// INSIGHT: use update varient of write is to extend lifespan of EEPROM by making a
// check on the content, only write if they differ
#define found_oracle eeprom_read_byte((u08_t*)&eeprom_found_oracle)
#define update_found_oracle(x) eeprom_update_byte((u08_t*)&eeprom_found_oracle, (u08_t)x)


// INSIGHT: use update varient of write is to extend lifespan of EEPROM by making a
// check on the content, only write if they differ
#define client_of eeprom_read_byte((u08_t*)&eeprom_client_of)
#define update_client_of(x) eeprom_update_byte((u08_t*)&eeprom_client_of, (u08_t)x)


// INSIGHT: use update varient of write is to extend lifespan of EEPROM by making a
// check on the content, only write if they differ
#define load_prepared_proposal() eeprom_read_block((void*)&prepared_proposal, (const void*)&eeprom_prepared_proposal, NVMCOMM_MESSAGE_SIZE)
#define save_prepared_proposal() eeprom_update_block((const void*)&prepared_proposal, (void*)&eeprom_prepared_proposal, NVMCOMM_MESSAGE_SIZE)


// INSIGHT: use update varient of write is to extend lifespan of EEPROM by making a
// check on the content, only write if they differ
#define prepared_proposal_size eeprom_read_byte((u08_t*)&eeprom_prepared_proposal_size)
#define update_prepared_proposal_size(x) eeprom_update_byte((u08_t*)&eeprom_prepared_proposal_size, (u08_t)x)


// INSIGHT: use update varient of write is to extend lifespan of EEPROM by making a
// check on the content, only write if they differ
#define has_proposal (bool_t)eeprom_read_byte((u08_t*)&eeprom_has_proposal)
#define update_has_proposal(x) eeprom_update_byte((u08_t*)&eeprom_has_proposal, (u08_t)x)

// INSIGHT: use update varient of write is to extend lifespan of EEPROM by making a
// check on the content, only write if they differ
#define load_oracle() eeprom_read_block((void*)&oracle, (const void*)&eeprom_oracle, sizeof(group))
#define save_oracle() eeprom_update_block((const void*)&oracle, (void*)&eeprom_oracle, sizeof(group))

// INSIGHT: use update varient of write is to extend lifespan of EEPROM by making a
// check on the content, only write if they differ
#define load_local_groups() eeprom_read_block((void*)&local_groups, (const void*)&eeprom_local_groups, MAX_GROUP_LIST_SIZE * sizeof(group))
#define save_local_groups() eeprom_update_block((const void*)&local_groups, (void*)&eeprom_local_groups, MAX_GROUP_LIST_SIZE * sizeof(group))

// INSIGHT: use update varient of write is to extend lifespan of EEPROM by making a
// check on the content, only write if they differ
#define local_group_size eeprom_read_byte((u08_t*)&eeprom_local_group_size)
#define update_local_group_size(x) eeprom_update_byte((u08_t*)&eeprom_local_group_size, (u08_t)x)

void group_load();
void load_groups();

// new view for group g, and pointer to view v
void new_view(
        group* g, 
        view* v);
void initial_view(
        group* g,
        address_t joiner_addr);
void update_view(
        group* g,
        view* vi);
void copy_view(
        view* destination,
        view* source);
view* current_view(group* g);

int prepare_update(
        u08_t* payload, 
        u08_t payload_size);
int commit_update(u08_t* payload);

int setup_group(const char* group_name, group* g);
int add_to_group_list(group* g);
group* find_group_from_gaddr(u08_t gaddr);

int two_phase_commit(
        group* g,
        nvmcomm_message* message);
int phase_send(
        group* g,
        nvmcomm_message* message);

void note_failure(
        group* g,
        bool_t* checklist);

void construct_join_proposal(
        u08_t* message_payload, 
        u08_t* payload, 
        u08_t* payload_size, 
        group* g, 
        address_t member);
void construct_leave_proposal(
        u08_t* message_payload, 
        u08_t* payload, 
        u08_t* payload_size, 
        group* g, 
        address_t member);

void construct_commit_message(
        nvmcomm_message* message, 
        group* g);
void construct_oracle_find_message(
        nvmcomm_message* message);
void construct_join_message(
        nvmcomm_message* message);
void construct_leave_message(
        nvmcomm_message* message);

int group_send(
        group* g,
        nvmcomm_message* message);
int group_p2psend(
        group* g,
        nvmcomm_message* message);

void load_proposal();
void save_proposal(
        u08_t* payload, 
        u08_t payload_size);
void delete_proposal();


int find_oracle();
int create_oracle();
int join_oracle();
int leave_oracle();

int multicast_members(
        nvmcomm_message* message,
        group* g);


// Reply Handler
void
group_handle_message(address_t src, u08_t nvmcomm_command,
        u08_t *payload,
        u08_t *response_size,
        u08_t *response_cmd)
{
    if (nvm_runlevel != NVM_RUNLVL_VM)
        return;

    switch (nvmcomm_command) {
        /*
         * Respond if it is an oracle
         * */
        case NVMCOMM_GROUP_EVENT_ORACLE_FIND:
        {
            if (am_i_oracle()) {
                payload[2] = my_address;
                *response_cmd = nvmcomm_command+1;
                *response_size = 3;
            } else {
                *response_size = 0;
            }
            break;
        }
        /*
         * Coordinator will initiate 2pc
         * PROPOSE from a leader will multicast to its members (including itself)
         * and next_group leader, the subsequent leaders will also do the same
         * so the PROPOSE will propagate until the last component group
         * But when replying, non-leaders will return based on prepare_update
         * return value
         * Leaders will return based on both phase_send and prepare_update values
         * So the replies will then propagate back to the previous leader, and so on
         * CAUTION: Coordinator essentially will reply to itself when it is done
         * */
        case NVMCOMM_GROUP_PROPOSE:
        {
            int phase;
            u08_t gaddr = payload[3];
            nvmcomm_message message;
            group* g = find_group_from_gaddr(gaddr);
            view* v = current_view(g);

            if (g != NULL && v != NULL) { // I'm fine
                // Acknowledge
                if (am_i_leader(g)) {
                    // Leader
                    int ret = prepare_update(payload, *response_size);

                    construct_commit_message(&message, g);
                    message.command = NVMCOMM_GROUP_PROPOSE;
                    phase = phase_send(g, &message);

                    if (phase < 0 || ret < 0) {
                        *response_cmd = NVMCOMM_GROUP_ERROR_R;
                    } else if (phase == 1) { // Missing replies
                        // TODONR: Fault handling
                    } else {
                        if (phase == 2 || ret == 1) // Full
                            *response_cmd = NVMCOMM_GROUP_EVENT_FULL_R;
                        else
                            *response_cmd = nvmcomm_command+1;
                    }
                } else {
                    // Non leader
                    // Store proposal first
                    int ret = prepare_update(payload, *response_size);
                    if (ret < 0) {
                        *response_cmd = NVMCOMM_GROUP_ERROR_R;
                    } else {
                        if (ret == 1)
                            *response_cmd = NVMCOMM_GROUP_EVENT_FULL_R;
                        else
                            *response_cmd = nvmcomm_command+1;
                    }
                }
            } else {
                *response_size = 0;
            }
            break;
        }
        /*
         * Coordinator will initiate the second round of 2pc
         * COMMIT will be sent from a leader, and the members (including itself)
         * plus the next_group leader
         * This command does not need replies as it is assumed all nodes are alive
         * that replied the first PROPOSE
         * */
        case NVMCOMM_GROUP_COMMIT:
        {
            u08_t gaddr = payload[3];
            nvmcomm_message message;
            group* g = find_group_from_gaddr(gaddr);

            if (g != NULL) {
                commit_update(payload);

                construct_commit_message(&message, g);
                message.command = NVMCOMM_GROUP_COMMIT;
                multicast_members(&message, g);
            }
            *response_size = 0;
            break;
        }
        /*
         * This receives message of following format:
         * [seq][seq]
         * [gaddr][member address]
         * */
        case NVMCOMM_GROUP_EVENT_JOIN:
        {
            u08_t gaddr = payload[2];
            address_t member = payload[3];
            group* g = find_group_from_gaddr(gaddr);
            nvmcomm_message message;
            construct_commit_message(&message, g);

            construct_join_proposal(message.payload, payload, response_size, g, member);

            if (am_i_oracle()) {
                if (am_i_leader(&oracle)) {
                    int ret = two_phase_commit(g, &message);
                    if (ret < 0) {
                        *response_cmd = NVMCOMM_GROUP_ERROR_R;
                    }
                    else if (ret == 0) {
                        // TODONR: State transfer
                        // Format should be as follows:
                        // [seq][seq]
                        // [state transfer type][member size][member][member]...
                        // T_JOIN_EXIST
                        *response_cmd = nvmcomm_command+1;
                        *response_size = *response_size;
                    }
                    else if (ret == 1) {
                        // TODONR: Special state transfer
                        // Format should be as follows:
                        // [seq][seq]
                        // [state transfer type][member size=1][member]
                        // T_JOIN_NEW
                        *response_cmd = nvmcomm_command+1;
                        *response_size = *response_size;
                    }
                } else {
                    // TODONR: Forward message?, probing coordinator
                    // TODONR: Fault tolerance
                }
            }

            // No response
            *response_size = 0;
            break;
        }
        /*
         * This receives message of following format:
         * [seq][seq]
         * [gaddr][member address]
         * */
        case NVMCOMM_GROUP_EVENT_LEAVE:
        {
            u08_t gaddr = payload[2];
            address_t member = payload[3];
            group* g = find_group_from_gaddr(gaddr);
            nvmcomm_message message;
            construct_commit_message(&message, g);

            construct_leave_proposal(message.payload, payload, response_size, g, member);

            if (am_i_oracle()) {
                if (am_i_leader(&oracle)) {
                    int ret = two_phase_commit(g, &message);
                    if (ret < 0)
                        *response_cmd = NVMCOMM_GROUP_ERROR_R;
                } else {
                    // TODONR: Forward message?, probing coordinator
                    // TODONR: Fault tolerance
                }
            }

            // No response
            *response_size = 0;
            break;
        }
    }
}

// View
int
get_rank_of(view* vi, address_t node_id)
{
    if (vi == NULL
            || node_id == NVMCOMM_ADDRESS_NULL)
        return -1;

    int i;
    for (i=0; i<vi->member_size; ++i) {
        if (vi->members[i] == node_id) {
            return i;
        }
    }

    return -1;
}

bool_t
am_i_leader(group* g)
{
    return get_rank_of(current_view(g),
                nvmcomm_get_node_id()) == 0;
}


// Group
int
join_group(group* g)
{
    // TODONR: Supposed to have thrashing check
    // to avoid applications that thrash by
    // leaving and joining multiple times
    // in a short burst
    //thrashing_check();

    return 0;
}

int
create_group(const char* group_name, group* g)
{
    // Setup group
    if (setup_group(group_name, g) < 0)
        return -1;

    // Add to group_list
    if (add_to_group_list(g) < 0)
        return -1;

    // Actually trying to join or create the group
    int ret;
    int retries =  10;
    while (retries >= 0) {
        if ((ret = join_group(g)) < 0) {
            continue;
        } else {
            break;
        }

        retries--;
    }

    if (ret < 0)
        return -1;

    return 0;
}

bool_t
am_i_oracle()
{
    return (can_be_oracle
            && client_of == NVMCOMM_ADDRESS_NULL
            && get_rank_of(current_view(&oracle),
                my_address) != -1
            && group_inited);
}

void
set_can_be_oracle()
{
    update_can_be_oracle(true);
    join_oracle();
}

void
set_cannot_be_oracle()
{
    update_can_be_oracle(false);
    leave_oracle();
}

int
group_init(u08_t flag)
{
    int i;
    if (flag & CANBEORACLE)
        update_can_be_oracle(true);
    my_address = nvmcomm_get_node_id();


    group_load();

    if (!group_loaded) {
        // Join/Create oracle group
        for (i=0; i<3
                && client_of == NVMCOMM_ADDRESS_NULL;
                ++i) {
            join_oracle();
            if (client_of != NVMCOMM_ADDRESS_NULL
                    || (oracle.view_size > 0
                    && get_rank_of(&oracle.views[0],
                        my_address) != -1)) {
    #ifdef DEBUG
                DEBUGF_GROUP("Joined ORACLE\n");
    #endif
                break;
            }

        }
        if (!can_be_oracle) {
            if (found_oracle != NVMCOMM_ADDRESS_NULL
                    && client_of == NVMCOMM_ADDRESS_NULL) {
                for (i=0; i<20
                        && (oracle.view_size == 0
                            || get_rank_of(&oracle.views[0],
                                my_address) == -1);
                        ++i) {
#ifdef DEBUG
                    DEBUGF_GROUP("Found the ORACLE node %x \
                            but still trying to connect \
                            to it\n", found_oracle);
#endif
                    if (client_of == NVMCOMM_ADDRESS_NULL)
                        join_oracle();
                    else
                        break;
                }
                if (client_of == NVMCOMM_ADDRESS_NULL
                        && (oracle.view_size == 0
                            || get_rank_of(&oracle.views[0],
                                my_address) == -1)) {
#ifdef DEBUG
                    DEBUGF_GROUP("Found the ORACLE node %x \
                            but can't connect to \
                            it\n", found_oracle);
#endif
                    return -1;
                }
            }
        }
    }

    group_inited = true;
#ifdef DEBUG
    DEBUGF_GROUP("Group initialized\n");
#endif

    // Proposal from last failure
    if (has_proposal) {
        // continue from last state
        commit_update(prepared_proposal);
    }

    return 0;
}


//////////
#define HEARTBEAT_INTERVAL 1000
#define HEARTBEAT_TIMEOUT 2500
#define INITIALISATION_TIMEOUT 15000
#define NOTIFY_TIMEOUT 30000
#define JITTERING_TIMEOUT 500

#define MAX_NUMBER_OF_WATCHED_NODES 10

typedef struct {
  address_t node_id;
  nvmtime_t expect_next_timestamp_before; // Initialise to currenttime + INITIALISATION_TIMEOUT
} node_to_watch;

nvmtime_t next_time_to_notify = 0;
node_to_watch watch_list[MAX_NUMBER_OF_WATCHED_NODES];
uint8_t watch_list_count;
nvmtime_t next_heartbeat_broadcast = 0; // Initialise to 0 to start sending heartbeats straight away.

// To be called periodically (at least as often as HEARTBEAT_INTERVAL)
void group_heartbeat() {
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
  // Check all nodes we're supposed to watch to see if we've received a heartbeat in the last HEARTBEAT_TIMEOUT ms.
  for(uint8_t i=0; i<watch_list_count; i++) {
    if (nvm_current_time > watch_list[i].expect_next_timestamp_before) {
      // Tell the master we didn't receive the heartbeat in time
      address_t master_node_id = wkpf_config_get_master_node_id();
      // Do we need a reply here? Maybe not for now. If the message isn't received, it will be sent again after a second.
      // For future versions we may want to stop sending when we know the master got the message, but since we're going
      // to do a full reconfiguration anyway, it doesn't really matter for now.
      if (next_time_to_notify + JITTERING_TIMEOUT <= nvm_current_time) {
#ifdef DEBUG
        DEBUGF_GROUP("notify master the failure of node %x\n", watch_list[i].node_id);
#endif
        nvmcomm_send(master_node_id, NVMCOMM_GROUP_NOTIFY_NODE_FAILURE, &watch_list[i].node_id, sizeof(address_t));
        next_time_to_notify = nvm_current_time + NOTIFY_TIMEOUT;
        blink_twice(LED5);
#ifdef LOGGING
        char message[25];
        uint8_t n = sprintf(message, "node %d was suspected failure", watch_list[i].node_id);
        LOGF_GROUP(message, n);
#endif
      }
    }
  }
}

void group_handle_heartbeat_message(address_t src) {
#ifdef DEBUG
  DEBUGF_GROUP("getting heartbeat message from node %x\n", src);
#endif
#ifdef LOGGING
  char message[25];
  uint8_t n = sprintf(message, "got heartbeat from node %d", src);
  LOGF_GROUP(message, n);
#endif
  for(uint8_t i=0; i<watch_list_count; i++)
    if (watch_list[i].node_id == src) {
#ifdef DEBUG
      DEBUGF_GROUP("handling heartbeat message\n", src);
#endif
      watch_list[i].expect_next_timestamp_before = nvm_current_time + HEARTBEAT_TIMEOUT;
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

// Private
void
load_groups()
{
    eeprom_busy_wait();

    if (can_be_oracle)
        load_oracle();

    if (local_group_size > 0)
        load_local_groups();
}

void
group_load()
{
    // TODONR: Load saved group membership and prepared_proposal from EEPROM
    // load group, views
    load_groups();

    // load prepared_proposal
    load_proposal();

    group_loaded = true;
}

int
setup_group(const char* group_name, group* g)
{
    strcpy(g->group_name, group_name);
    /*g->group_addr = ;*/
    return 0;
}

int
add_to_group_list(group* g)
{
    /*
    int i;
    u08_t group_addr;

    if (current_group_list_size == MAX_GROUP_SIZE) {
        return -2; // Exceeding the maximum group list size
    }

    for (i=0; i<current_group_list_size; ++i) {
        group_addr = current_group_list[i].group_address;
        if (strcmp(gaddr, group_addr) == 0) {
            return -1; // Already created in the list
        }
    }

    view view = {};
    group new_group;
    new_group.group_address = gaddr;
    new_group.view_size = 1;
    new_group.views[0] = view;

    current_group_list[++current_group_list_size] = new_group;
    g = &new_group;
    */
    return 0;
}

group*
find_group_from_gaddr(u08_t gaddr)
{
    if (gaddr == 0x00) { // Oracle
        return &oracle;
    }

    return NULL;
}

void
note_failure(group* g, bool_t* checklist)
{
}

void
new_view(group* g, view* the_view)
{
    view* v = current_view(g);
    if (v != NULL) {
        the_view->view_id = v->view_id+1;
        the_view->member_size = 0;

        the_view->joiner_size = 0;
        the_view->leaver_size = 0;

        the_view->coordinator = NVMCOMM_ADDRESS_NULL;
        the_view->next_group = NVMCOMM_ADDRESS_NULL;
    }
}

void
initial_view(group* g, address_t joiner_addr)
{
    view the_view;
    the_view.view_id = 0;
    the_view.members[0] = joiner_addr;
    the_view.member_size = 1;
    the_view.joiners[0] = joiner_addr;
    the_view.joiner_size = 1;
    the_view.leaver_size = 0;
    the_view.coordinator = NVMCOMM_ADDRESS_NULL;
    the_view.next_group = NVMCOMM_ADDRESS_NULL;

    g->views[0] = the_view;
    g->view_size = 1;
}

void
update_view(group* g, view* vi)
{
#ifdef DEBUG
    DEBUGF_GROUP("update_view: view id %x, for group %s\n", vi->view_id, g->group_name);
#endif
    if (g->view_size < MAX_VIEW_LIST_SIZE) {
        view* next_view = g->views + g->view_size;
        *next_view = *vi;
        g->view_size++;
        /*copy_view(&g->views[g->view_size++], vi);*/
    } else {
#ifdef DEBUG
        DEBUGF_GROUP("update_view: views exceeding maximum limit\n", vi->view_id, g->group_name);
#endif
    }
}

view*
current_view(group* g)
{
    if (g->view_size > 0) {
        return &g->views[g->view_size-1];
    }
    return NULL;
}

void
copy_view(view* destination, view* source)
{
    destination->view_id = source->view_id;
    memcpy(destination->members, source->members, source->member_size);
    destination->member_size = source->member_size;
    memcpy(destination->joiners, source->joiners, source->joiner_size);
    destination->joiner_size = source->joiner_size;
    memcpy(destination->leavers, source->leavers, source->leaver_size);
    destination->leaver_size = source->leaver_size;
}

/// Create an oracle group, if it is allowed to
int
create_oracle()
{
#ifdef DEBUG
            DEBUGF_GROUP("Create ORACLE group\n");
#endif
    if (!group_inited) {
        strcpy(oracle.group_name, "ORACLE");
        oracle.group_addr = ORACLE_ADDR; // Oracle group addr
        initial_view(&oracle, my_address);
        update_found_oracle(my_address);
    }

    return 0;
}

int
leave_oracle()
{
    if (group_inited) {
    }
    return 0;
}

/// Join oracle group, if it is allowed to
int
join_oracle()
{
    // TODONR: Random delay, to prevent conflict
    // as much as possible

    // Has to find a oracle member first
    find_oracle();

    if (found_oracle == NVMCOMM_ADDRESS_NULL) {
        if (!can_be_oracle) {
#ifdef DEBUG
            DEBUGF_GROUP("Node %x cannot find oracle, and cannot create one\n", my_address);
#endif
            return -1;
        }
    } else {
        if (found_oracle == my_address) {
            return 0;
        }
#ifdef DEBUG
        DEBUGF_GROUP("Sending ORACLE a JOIN request from %x\n", my_address);
        DEBUGF_GROUP("Oracle is found %x, trying to join\n", found_oracle);
#endif
        nvmcomm_message message;
        construct_join_message(&message);
        return group_p2psend(&oracle, &message);
    }

    return create_oracle();
}

int
find_oracle()
{
#ifdef DEBUG
    DEBUGF_GROUP("Finding oracle\n");
#endif
    int ret;

    if (found_oracle == NVMCOMM_ADDRESS_NULL) {
        nvmcomm_message message;
        construct_oracle_find_message(&message);
        ret = group_send(&oracle, &message);
        return ret;
    }

#ifdef DEBUG
    DEBUGF_GROUP("Oracle already found %x\n", found_oracle);
#endif
    return 0;
}


/*
 * Constructing a join proposal in bytes
 * */
void
construct_join_proposal(u08_t* message_payload, u08_t* payload, u08_t* payload_size, group* g, address_t member)
{
    if (g != NULL) {
        message_payload[0] = payload[0];
        message_payload[1] = payload[1];
        message_payload[2] = V_UPDATE_JOINER;
        message_payload[3] = g->group_addr;
        message_payload[4] = member;

        *payload_size = 5;
    }
}

/*
 * Constructing a join proposal in bytes
 * */
void
construct_leave_proposal(u08_t* message_payload, u08_t* payload, u08_t* payload_size, group* g, address_t member)
{
    if (g == NULL) {
        message_payload[0] = payload[0];
        message_payload[1] = payload[1];
        message_payload[2] = V_UPDATE_LEAVER;
        message_payload[3] = g->group_addr;
        message_payload[4] = member;

        *payload_size = 5;
    }
}

void
construct_commit_message(nvmcomm_message* message, group* g)
{
    message->from = my_address;
    message->to = NVMCOMM_ADDRESS_NULL;
    // command doesn't matter because it will be changed
    set_message_sequence_number(message->payload,
            &message->payload_length);
    message->payload[2] = g->group_addr; // content here
    message->payload_length += 1;
}

void
construct_oracle_find_message(nvmcomm_message* message)
{
    message->from = my_address;
    if (!am_i_oracle()
            && client_of != NVMCOMM_ADDRESS_NULL)
        message->to = client_of;
    else
        message->to = NVMCOMM_ADDRESS_NULL;
    message->command =
        NVMCOMM_GROUP_EVENT_ORACLE_FIND;
    set_message_sequence_number(message->payload,
            &message->payload_length);
}

void
construct_leave_message(nvmcomm_message* message)
{
    message->from = my_address;
    if (!am_i_oracle()
            && client_of != NVMCOMM_ADDRESS_NULL)
        message->to = client_of;
    else
        message->to = NVMCOMM_ADDRESS_NULL;
    message->command = NVMCOMM_GROUP_EVENT_LEAVE;
    set_message_sequence_number(message->payload,
            &message->payload_length);
    message->payload[2] = 0; // content here
    message->payload_length += 1;
}

void
construct_join_message(nvmcomm_message* message)
{
    message->from = my_address;
    if (!am_i_oracle() && client_of != NVMCOMM_ADDRESS_NULL)
        message->to = client_of;
    else
        message->to = NVMCOMM_ADDRESS_NULL;
    message->command = NVMCOMM_GROUP_EVENT_JOIN;
    set_message_sequence_number(message->payload, &message->payload_length);
    message->payload[2] = my_address;
    message->payload_length += 1;
}

// Returns -1 for error
// 0 for all replies
// 1 for missing nodes
// 2 for full membership
int
phase_send(group* g, nvmcomm_message* message)
{
    if (g == NULL || message == NULL)
#ifdef DEBUG
        DEBUGF_GROUP("phase_send: \
                group or message is null\n");
#endif
        return -1;

#ifdef DEBUG
    DEBUGF_GROUP("phase_send: \
            initiating phase commit for group \
            %s\n", g->group_name);
#endif

    int i;
    view* the_view = current_view(g);
    address_t* members = the_view->members;
    address_t next_group = current_view(g)->next_group;
    int member_size = the_view->member_size;

    int alive_count = 0;
    bool_t checklist[MAX_VIEW_MEMBER_SIZE];
    int checklist_size;

    checklist_size = member_size;

    if (nvmcomm_multicast(
                members,
                member_size,
                message->command,
                message->payload,
                message->payload_length) != 0)
        return -1;

    if (next_group != NVMCOMM_ADDRESS_NULL) {
        checklist_size++;
        nvmcomm_send(
                next_group,
                message->command,
                message->payload,
                message->payload_length);
    }

    for (i=0; i<checklist_size; ++i) {
        checklist[i] = false;
    }

    bool_t is_full = false;
    nvmcomm_message *reply = NULL;
    uint32_t timeout = nvm_current_time + 100;
    while (nvm_current_time < timeout) {
        reply = nvmcomm_wait(
                10,
                (u08_t[]){message->command+1, NVMCOMM_GROUP_EVENT_FULL_R, NVMCOMM_GROUP_ERROR_R},
                1);

        if (reply != NULL // Check sequence number because an old message could be received: the right type, but not the reply to our last sent message
                  && check_sequence_number(reply->payload, message->payload)) {
            // This message a reply to our last sent message
            if(reply->command == message->command+1) {
                for (i=0; i<member_size; ++i) {
                    if (reply->from == members[i]) {
                        checklist[i] = true;
                        alive_count++;
                    }
                }
            } else if (reply->command == NVMCOMM_GROUP_EVENT_FULL_R) {
                is_full = true;
            } else if (reply->command == NVMCOMM_GROUP_ERROR_R) {
            } else {
                // Could be other reasons (faults)
                DEBUGF_COMM("phase_send: \
                        received unexpected reply from node %x\n", reply->from);
            }
        }
    }

    // Note failure nodes, Fault handling
    note_failure(g, checklist);

    if (alive_count == checklist_size) {
        if (is_full)
            return 2;
        return 0;
    } else {
        return 1;
    }
}

// Used by Oracle leader
// Assuming message has sequence number
// Return -2, message too large
// Return -1, error
// Return 0, all replies,
// Return 1, missing replies,
// Return 2, full membership
int
two_phase_commit(group* g, nvmcomm_message* message)
{
    if (message->payload_length
            > NVMCOMM_MESSAGE_SIZE) {
        DEBUGF_COMM("two_phase_commit: \
                message oversized\n");
        return -2; // Message too large
    }

#ifdef DEBUG
    DEBUGF_GROUP("two_phase_commit: \
            initiating 2pc protocol for group \
            %s\n", g->group_name);
#endif
    if (g->view_size == 0) {
#ifdef DEBUG
        DEBUGF_GROUP("two_phase_commit: \
                failed initiating, as there is \
                no view for the group \
                %s\n", g->group_name);
#endif
        return -1;
    }

    int phase;

    // First phase
    message->command = NVMCOMM_GROUP_PROPOSE;
    phase = phase_send(g, message);

    if (phase < 0) {
#ifdef DEBUG
        DEBUGF_GROUP("two_phase_commit: \
                2pc protocol for group \
                %s has failed\n", g->group_name);
#endif
        return -1;
    } else {
        // Second phase
        message->command = NVMCOMM_GROUP_COMMIT;
        multicast_members(message, g);
        return phase;
    }
}

// Assuming message has sequence number
int
group_send(group* g, nvmcomm_message* message)
{
    if (message->payload_length
            > NVMCOMM_MESSAGE_SIZE) {
        DEBUGF_COMM("message oversized\n");
        return -2; // Message too large
    }

    if (nvmcomm_broadcast(
                message->command,
                message->payload,
                message->payload_length) != 0)
        return -1;

    // Maybe it should wait longer
    nvmcomm_message *reply = NULL;
    uint32_t timeout = nvm_current_time + 100;
    while (nvm_current_time
            < timeout && reply == NULL) {
        reply = nvmcomm_wait(
                10,
                (u08_t[]){message->command+1, NVMCOMM_GROUP_ERROR_R},
                1);

        // Pick the first reply
        if (reply != NULL // Check sequence number because an old message could be received: the right type, but not the reply to our last sent message
                  && check_sequence_number(reply->payload, message->payload)) {
            // This message a reply to our last sent message
            if(reply->command == message->command+1) {
                u08_t* new_view; // Declared outside switch statement
                switch (message->command) {
                    case NVMCOMM_GROUP_EVENT_JOIN:
                        // Should update INITIALVIEW here
                        new_view = reply->payload + 2;
                        update_view(g, (view*)new_view);
                        return 0;
                    break;
                    case NVMCOMM_GROUP_EVENT_ORACLE_FIND:
                        // Update found_oracle
                        update_found_oracle(reply->payload[2]);
                        return 0;
                    break;
                }
            } else if (reply->command == NVMCOMM_GROUP_ERROR_R) {
            } else {
                // Could be other reasons (faults)
                return -1;
            }
        }
    }

    return -1;
}

// Assuming message has sequence number
int
group_p2psend(group* g, nvmcomm_message* message)
{
    if (message->payload_length
            > NVMCOMM_MESSAGE_SIZE) {
        DEBUGF_COMM("message oversized\n");
        return -2; // Message too large
    }
#ifdef DEBUG
    DEBUGF_GROUP("group_p2psend: sending to %x for group %s\n", message->to, g->group_name);
#endif

    if (nvmcomm_send(
                message->to,
                message->command,
                message->payload,
                message->payload_length) != 0)
        return -1;

    // Maybe it should wait longer
    nvmcomm_message *reply = NULL;
    uint32_t timeout = nvm_current_time + 100;
    while (nvm_current_time < timeout
            && reply == NULL) {
        reply = nvmcomm_wait(
                10,
                (u08_t[]){message->command+1, NVMCOMM_GROUP_ERROR_R},
                1);

        if (reply != NULL // Check sequence number because an old message could be received: the right type, but not the reply to our last sent message
                  && check_sequence_number(reply->payload, message->payload)) {
            // This message a reply to our last sent message
            if(reply->command == message->command+1) {
                u08_t* new_view; // Declared outside switch statement
                switch (reply->command) {
                    /*
                     * State transfer could come in two flavors
                     * The normal one is simply joining one component group
                     * The special one creates a new component group
                     * */
                    case NVMCOMM_GROUP_EVENT_JOIN_R:
                        // TODONR: INITIALVIEW
                        new_view = reply->payload + 2;
                        update_view(g, (view*)new_view);
                        return 0;
                    break;
                }
            } else if (reply->command == NVMCOMM_GROUP_ERROR_R) {
            }
        }
    }

    return -1;
}

void load_proposal()
{
    eeprom_busy_wait();
    load_prepared_proposal();
}

void save_proposal(u08_t* payload, u08_t payload_size)
{
    // store to prepared_proposal
    memcpy(prepared_proposal, payload, payload_size);

    // store to EEPROM
    // Waiting for EEPROM to be ready for IO
    eeprom_busy_wait();
    save_prepared_proposal(); // save to eeprom version of prepared_proposal
    update_prepared_proposal_size(payload_size); // save to eeprom version of prepared_proposal
    update_has_proposal(true); // update has_proposal bool in eeprom
}

void delete_proposal()
{
    update_has_proposal(false);
    memset(prepared_proposal, 0, prepared_proposal_size);
    update_prepared_proposal_size(0);
}

// Return -1, error
// Return 0, normal
// Return 1, full membership
// payload format should be
// [seq][seq]
// [type][gaddr][member]
int prepare_update(u08_t* payload, u08_t payload_size)
{
    group* g = find_group_from_gaddr(payload[3]);

    if (g == NULL) {
#ifdef DEBUG
        DEBUGF_GROUP("[ERROR]prepare_update: group not found\n");
#endif
        return -1;
    }

#ifdef DEBUG
    DEBUGF_GROUP("prepare_update: for group %s\n", g->group_name);
#endif

    save_proposal(payload, payload_size);

    view* v = current_view(g);
    if (v == NULL) {
#ifdef DEBUG
        DEBUGF_GROUP("[ERROR]prepare_update: no view in group\n");
#endif
        return -1;
    }

    if (v->member_size == MAX_VIEW_MEMBER_SIZE)
        return 1;

    return 0;
}

/*
 * Proposal format
 * [seq][seq]
 * [type][gaddr][member]
 * */
int commit_update(u08_t* payload)
{
    if (!has_proposal) {
#ifdef DEBUG
        DEBUGF_GROUP("[ERROR]commit_update: no proposal found\n");
#endif
        return -1;
    }

    if (memcmp(prepared_proposal, payload, sizeof(prepared_proposal)) != 0) {
#ifdef DEBUG
        DEBUGF_GROUP("[ERROR]commit_update: proposal are different\n");
#endif
        return -1;
    }

    group* g = find_group_from_gaddr(payload[1]);

#ifdef DEBUG
    DEBUGF_GROUP("commit_update: for group %s\n", g->group_name);
#endif

    view* v = current_view(g);
    u08_t type = payload[2];
    u08_t member = payload[3];
    view the_view; new_view(g, &the_view);

    if (type == V_UPDATE_JOINER) {
        if (v->member_size == MAX_VIEW_MEMBER_SIZE) {
            // FULL
            if (v->next_group == NVMCOMM_ADDRESS_NULL)
                v->next_group = member;
        }
        else {
            memcpy(the_view.members, v->members, v->member_size);
            the_view.member_size += v->member_size;

            memcpy(the_view.members+the_view.member_size, &member, 1);
            the_view.member_size += 1;

            memcpy(the_view.joiners+the_view.joiner_size, &member, 1);
            the_view.joiner_size += 1;
        }
    }
    else if (type == V_UPDATE_LEAVER) {
        // TODONR: Remove members
        // TODONR: Or remove component group ref
        // TODONR: If itself is a weak link, 
        // initiate weak link elimination algorithm
    }
    update_view(g, &the_view);

    // erase proposal
    delete_proposal();

    // TODONR:update node state to EEPROM

    return 0;
}

int
multicast_members(nvmcomm_message* message, group* g)
{
    view* the_view = current_view(g);
    address_t* members = the_view->members;
    address_t next_group = current_view(g)->next_group;
    int member_size = the_view->member_size;

    if (nvmcomm_multicast(
                members,
                member_size,
                message->command,
                message->payload,
                message->payload_length) != 0)
        return -1;

    if (next_group != NVMCOMM_ADDRESS_NULL) {
        nvmcomm_send(
                next_group,
                message->command,
                message->payload,
                message->payload_length);
    }
    return 0;
}

#endif // NVM_USE_GROUP
