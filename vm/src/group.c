#include <string.h>
#include "types.h"
#include "delay.h"
#include "nvmcomm.h"
#include "debug.h"
#include "group.h"
#include "vm.h"

static bool_t group_loaded = false;
static bool_t group_inited = false;
static bool_t can_be_oracle = false;
static address_t my_address = NVMCOMM_ADDRESS_NULL;
static address_t found_oracle = NVMCOMM_ADDRESS_NULL; // For client
static address_t client_of = NVMCOMM_ADDRESS_NULL; // Client of a oracle representative
static u08_t prepared_proposal[NVMCOMM_MESSAGE_SIZE]; // Client of a oracle representative
static group oracle; // GMS service

void group_load();

void initial_view(
        group* g, 
        address_t joiner_addr);
void update_view(
        group* g, 
        view* vi);
void copy_view(view* destination, 
        view* source);
view* current_view(group* g);

int prepare_update(u08_t* payload, group* g);
int commit_update(u08_t* payload, group* g);

int setup_group(group* g);
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

void construct_commit_message(
        nvmcomm_message* message, group* g);
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

int find_oracle();
int create_oracle();
int join_oracle();
int leave_oracle();


// Reply Handler
void 
group_handle_message(u08_t nvmcomm_command, 
        u08_t *payload, 
        u08_t *response_size, 
        u08_t *response_cmd)
{
    if (nvm_runlevel != NVM_RUNLVL_VM)
        return;

    switch (nvmcomm_command) {
        case NVMCOMM_GROUP_PROPOSE:
        {
            /*int i;*/
            u08_t gaddr = payload[2];
            nvmcomm_message message;
            group* g = find_group_from_gaddr(gaddr);
            view* v = current_view(g);

            if (g != NULL) {
                // Acknowledge
                if (am_i_leader(g)) {
                    construct_commit_message(&message, g);
                    message->command = NVMCOMM_GROUP_PROPOSE;
                    phase = phase_send(g, message);

                    if (v->next_group != NVMCOMM_ADDRESS_NULL) {
                        if (phase < 0) {
                            *response_cmd = NVMCOMM_GROUP_ERROR_R;
                            *response_size = 3;
                        } else if (phase == 1) {
                            *response_cmd = NVMCOMM_GROUP_FULL_R;
                            *response_size = 3;
                        } else {
                            *response_cmd = nvmcomm_command+1;
                            *response_size = 3;
                        }
                    } else {
                        // Last component group
                        if (v->member_size == MAX_VIEW_MEMBER_SIZE) {
                            // Full membership
                            // Special algorithm
                            if (prepare_update(payload, g) == 0) {
                                *response_cmd = NVMCOMM_GROUP_FULL_R;
                                *response_size = 3;
                            } else {
                                *response_cmd = NVMCOMM_GROUP_ERROR_R;
                                *response_size = 3;
                            }
                        } else {
                            if (phase < 0) {
                                *response_cmd = NVMCOMM_GROUP_ERROR_R;
                                *response_size = 3;
                            } else {
                                if (prepare_update(payload, g) == 0) {
                                    *response_cmd = nvmcomm_command+1;
                                    *response_size = 3;
                                } else {
                                    *response_cmd = NVMCOMM_GROUP_ERROR_R;
                                    *response_size = 3;
                                }
                            }
                        }
                    }
                } else {
                    // Non leader
                    // Last component group
                    if (v->next_group == NVMCOMM_ADDRESS_NULL) {
                        if (prepare_update(payload, g) == 0) {
                            *response_cmd = nvmcomm_command+1;
                            *response_size = 3;
                        } else {
                            *response_cmd = NVMCOMM_GROUP_ERROR_R;
                            *response_size = 3;
                        }
                    }
                }
            } else {
                *response_size = 0;
            }
            break;
        }
        case NVMCOMM_GROUP_COMMIT:
        {
            u08_t gaddr = payload[2];
            nvmcomm_message message;
            group* g = find_group_from_gaddr(gaddr);
            view* v = current_view(g);

            if (g != NULL) {
                // Acknowledge
                if (am_i_leader(g)) {
                    construct_commit_message(&message, g);
                    message->command = NVMCOMM_GROUP_COMMIT;
                    phase = phase_send(g, message);

                    if (v->next_group != NVMCOMM_ADDRESS_NULL) {
                        if (phase < 0) {
                            *response_cmd = NVMCOMM_GROUP_ERROR_R;
                            *response_size = 3;
                        } else {
                            *response_cmd = nvmcomm_command+1;
                            *response_size = 3;
                        }
                    } else {
                        // Last component group
                        if (phase < 0) {
                            *response_cmd = NVMCOMM_GROUP_ERROR_R;
                            *response_size = 3;
                        } else {
                            if (commit_update(payload, g) == 0) {
                                *response_cmd = nvmcomm_command+1;
                                *response_size = 3;
                            } else {
                                *response_cmd = NVMCOMM_GROUP_ERROR_R;
                                *response_size = 3;
                            }
                        }
                    }
                } else {
                    // Non leader
                    // Last component group
                    if (v->next_group == NVMCOMM_ADDRESS_NULL) {
                        if (commit_update(payload, g) == 0) {
                            *response_cmd = nvmcomm_command+1;
                            *response_size = 3;
                        } else {
                            *response_cmd = NVMCOMM_GROUP_ERROR_R;
                            *response_size = 3;
                        }
                    }
                }
            } else {
                *response_size = 0;
            }
            break;
        }
        case NVMCOMM_GROUP_EVENT_JOIN:
        {
            u08_t gaddr = payload[2];
            group* g = find_group_from_gaddr(gaddr);
            nvmcomm_message message;
            construct_commit_message(&message, g);

            if (am_i_oracle()) {
                if (am_i_leader(&oracle)) {
                    int ret = two_phase_commit(g, &message);
                    if (ret == 0) {
                        *response_cmd = nvmcomm_command+1;
                        *response_size = 3;
                    }
                    else if (ret == 1) {
                    }
                    else {
                        *response_cmd = NVMCOMM_GROUP_ERROR_R;
                        *response_size = 3;
                    }
                } else {
                    // Forward message?
                    // Temporary
                    *response_size = 0;
                    
                }
            } else {
                // No response
                *response_size = 0;
            }
            break;
        }
        case NVMCOMM_GROUP_EVENT_LEAVE:
        {
            if (am_i_oracle()) {
                if (am_i_leader(&oracle)) {
                    // Note failure
                } else {
                    // Forward message?
                }
            } else {
                *response_size = 0;
            }
            break;
        }
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
    if (setup_group(g) < 0)
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
    can_be_oracle = true;
    join_oracle();
}

void 
set_cannot_be_oracle()
{
    can_be_oracle = false;
    leave_oracle();
}

int 
group_init(u08_t flag)
{
    int i;
    if (flag & CANBEORACLE)
        can_be_oracle = true;
    my_address = nvmcomm_get_node_id();


    // Load saved group membership from EEPROM and prepared_proposal
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
                    if (client_of == 0)
                        join_oracle();
                    else
                        break;
                }
                if (client_of == 0 
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
    if (prepared_proposal) {
        // TODONR: continue from last state
    }

    return 0;
}

// Private

void
group_load()
{
    // load group, views
    // load prepared_proposal
    group_loaded = true;
}

int 
setup_group(group* g)
{
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
initial_view(group* g, address_t joiner_addr)
{
    int i;
    view new_view;
    new_view.view_id = 0;
    new_view.members[0] = joiner_addr;
    new_view.member_size = 1;
    for (i=0; i<MAX_VIEW_MEMBER_SIZE; ++i) {
        new_view.has_failed[i] = false;
    }
    new_view.joiners[0] = joiner_addr;
    new_view.joiner_size = 1;
    new_view.leaver_size = 0;
    new_view.coordinator = NVMCOMM_ADDRESS_NULL;
    new_view.next_group = NVMCOMM_ADDRESS_NULL;

    g->views[0] = new_view;
    g->view_size = 1;
}

void 
update_view(group* g, view* vi)
{
#ifdef DEBUG
    DEBUGF_GROUP("update_view: view id %x, for group %s\n", vi->view_id, g->group_name);
#endif
    copy_view(&g->views[g->view_size++], vi);
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
    memcpy(destination->has_failed, source->has_failed, source->fail_size);
    destination->fail_size = source->fail_size;
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
        found_oracle = my_address;
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

// Returns -1 for missing replies or error
// 0 for all replies
// 1 for full membership
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

    if (next_group != NVMCOMM_ADDRESS_NULL) {
        nvmcomm_send(
                next_group,
                message->command,
                message->payload,
                message->payload_length);
    } else {
        // Last component group
        address_t members[10];
        // Multicast to members
        if (nvmcomm_multicast(
                    members,
                    message->command, 
                    message->payload, 
                    message->payload_length) != 0)
            return -1;

        checklist_size = member_size;
        for (i=0; i<checklist_size; ++i) {
            checklist[i] = false;
        }
    }

    nvmcomm_message *reply = NULL;
    uint32_t timeout = nvm_current_time + 100;
    while (nvm_current_time < timeout 
            && reply == NULL) {
        reply = nvmcomm_wait(
                10, 
                (u08_t[]){message->command+1, NVMCOMM_GROUP_FULL_R, NVMCOMM_GROUP_ERROR_R}, 
                1);

        if (reply != NULL // Check sequence number because an old message could be received: the right type, but not the reply to our last sent message
                  && check_sequence_number(reply->payload, message->payload)) {
            // This message a reply to our last sent message
            if(reply->command == message->command+1) {
                if (next_group != NVMCOMM_ADDRESS_NULL) {
                    return 0;
                } else {
                    for (i=0; i<member_size; ++i) {
                        if (reply->from == members[i]) {
                            checklist[i] = true;
                            alive_count++;
                        }
                    }
                }
            } else if (reply->command == NVMCOMM_GROUP_FULL_R) {
                return 1;
            } else if (reply->command == NVMCOMM_GROUP_ERROR_R) {
            } else {
                // Could be other reasons (faults)
                return -1;
            }
        }
    }

    // Note failure nodes
    note_failure(g, checklist);

    if (alive_count == checklist_size) {
        return 0;
    } else {
        return 1;
    }
}

// Used by Oracle leader
// Assuming message has sequence number
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
        phase = phase_send(g, message);
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
                        found_oracle = reply->payload[2];
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
                switch (message->command) {
                    case NVMCOMM_GROUP_EVENT_JOIN:
                        // Should update INITIALVIEW here
                        new_view = reply->payload + 2;
                        update_view(g, (view*)new_view);
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

int prepare_update(u08_t* payload, group* g)
{
#ifdef DEBUG
    DEBUGF_GROUP("prepare_update: for group %s\n", g->group_name);
#endif
    // store to prepared_proposal
    strcpy(prepared_proposal, payload);

    // TODONR:store to EEPROM

    if (memcmp(prepared_proposal, payload, sizeof(prepared_proposal)) == 0)
        return 0;
    return -1;
}

int commit_update(u08_t* payload, group* g)
{
    if (prepared_proposal == NULL) {
#ifdef DEBUG
        DEBUGF_GROUP("commit_update: can't find proposal\n");
#endif
        return -1;
    } else if (memcmp(prepared_proposal, payload, sizeof(prepared_proposal)) != 0) {
#ifdef DEBUG
        DEBUGF_GROUP("commit_update: proposal are different\n");
#endif
        return -1;
    }

#ifdef DEBUG
    DEBUGF_GROUP("commit_update: for group %s\n", g->group_name);
#endif
    
    int i;
    u08_t number_of_args = payload[3];
    u08_t offset = 0;
    for (i=0; i<number_of_args; ++i) {
        u08_t type = payload[4+offset];
        u08_t number_of = payload[4+offset+1];
        u08_t* data = payload + 4+offset+2;
        if (type == V_UPDATE_JOINER) {
            view* v = current_view(g);
            if (v->member_size == MAX_VIEW_MEMBER_SIZE) {
                // FULL
                v->next_group = data;
            }
            else {
                view new_view = DEFAULT_VIEW;
                if (v != NULL) {
                    // Copy members
                    memcpy(new_view.members, v->members, v->member_size);
                    new_view.member_size += v->member_size;
                }
                memcpy(new_view.members+new_view.member_size, data, number_of);
                new_view.member_size += number_of;
                memcpy(new_view.joiners+new_view.joiner_size, data, number_of);
                new_view.joiner_size += number_of;
                update_view(g, &new_view);
            }
        }
        else if (type == V_UPDATE_LEAVER) {
        }
        else if (type == V_UPDATE_HASFAILED) {
        }

        offset += number_of + 2; // 0 count, and also 1 for number_of
    }

    // TODONR:store to EEPROM

    return 0;
}
