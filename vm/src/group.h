#ifndef GROUP_H
#define GROUP_H

#include "types.h"

// Special group address
#define ORACLE_ADDR 0x00

// Node flags
#define CANBEORACLE 0x01

// Group flags. ** not used yet

// These flags come in two flavors.  Some are "set once", when the group is first created and for those we don't need to worry about locking before testing.
// But the others are turned on and off and on them, you must obtain the GroupFlagsLock before modifying or reading.
// These are the "set once" flags:
#define G_ISLARGE 0x01                  // Group is large
#define G_USEUNICAST 0x02               // Unicast only
#define G_USEIPMC 0x04                  // Use IPMC if possible
#define G_HASUAGG 0x08                  // Contains one or more user defined aggregators, run tokens faster
#define G_TRACE 0x10                    // Trace events in this group
#define G_SECURE 0x20                   // True for a secure group
#define G_ISRAW 0x40                    // Disables vsync flush

// These are the ones for which you must obtain the GroupFlagsLock before access
#define G_WEDGED 0x40                   // True while group is wedged for membership changes
#define G_SENDINGSTABILITY 0x080        // True if the group is in the process of sending its stability report
#define G_NEEDSTATEXFER 0x100           // Wait until state xfer completes before delivering first message

// Events
#define G_EVENT_CREATE 0x01
#define G_EVENT_JOIN 0x02
#define G_EVENT_CANBEORACLE 0x03
#define G_EVENT_QUERY 0x04
#define G_EVENT_UPDATE 0x05

// View update types
#define V_UPDATE_JOINER 0x06
#define V_UPDATE_LEAVER 0x07

// State transfer types
#define T_JOIN_EXIST 0x08
#define T_JOIN_NEW 0x09

// Max
#define MAX_GROUP_SIZE 8
#define MAX_GROUP_LIST_SIZE 8

#define MAX_VIEW_SIZE 8
#define MAX_VIEW_LIST_SIZE 8

#define MAX_VIEW_MEMBER_SIZE 8

// View id type
#define VIEW_UNINITIALIZED -1

typedef struct view {
    int view_id;

    // list of the current members of this group
    address_t members[MAX_VIEW_MEMBER_SIZE];
    int member_size;

    // list of the processes that joined this group since the last view was reported
    address_t joiners[MAX_VIEW_MEMBER_SIZE];
    int joiner_size;

    // list of processes that departed this group since the last view was reported
    address_t leavers[MAX_VIEW_MEMBER_SIZE];
    int leaver_size;

    address_t coordinator;
    address_t next_group;
} view;

/// Group structure
// group_name is the name/identifier of this group
// group_addr is a unique addr for group
// views contains a sequence of views ordered by its index (lower == older -> higher == newer)
typedef struct group {
    char group_name[32];
    address_t group_addr;
    view views[MAX_VIEW_LIST_SIZE];
    int view_size;
} group;

//// View
/// Obtains the rank of a specified process within the current view, returning an index into the current view even if the process has failed.  
/// Rank is defined from
/// 0 (oldest) to n-1 (youngest) for a group with n members.  Ties are broken by using the original order in which Isis was
/// told to add a set of members: if the system was asked to add x, y and z all at once, x will rank lower than y, etc.
/// Returns -1 if the specified process is not a member of the view.
extern int get_rank_of(view* vi, address_t node_id);

/// True if the caller is the leader (oldest, lowest-ranked) member of this view.
/// This member is often used as the leader in tasks where a single unambiguous leader is needed.
/// returns true if this member is the leader, false if not
extern bool_t am_i_leader(group* g);

//// Group
// node flags, defined in the beginning of this file
extern int group_init(u08_t flag);

extern int create_group(const char* group_name, group* g);
extern int join_group(group* g);
extern int leave_group(group* g);
extern int terminate_group(group* g);

/// True if this node is part of oracle group
/// False otherwise
extern bool_t am_i_oracle();

/// Set can_be_oracle to true
extern void set_can_be_oracle();

/// Set can_be_oracle to false
extern void set_cannot_be_oracle();

extern int terminate_group_member(group* g, u08_t node_id); // Send poison pill to member node_id

extern u08_t* query(group *g);
extern u08_t* update_group_members(group* g);

extern void group_handle_message(u08_t nvmcomm_command, u08_t *payload, u08_t *response_size, u08_t *response_cmd);

#endif // GROUP_H
