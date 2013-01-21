#ifndef GROUP_H
#define GROUP_H

#include "types.h"

extern void send_heartbeat();
extern void handle_failure();
extern void group_handle_update_component(address_t src, u08_t nvmcomm_command, u08_t *payload, u08_t *response_size, u08_t *response_cmd);
extern void group_handle_message(address_t src, u08_t nvmcomm_command, u08_t *payload, u08_t *response_size, u08_t *response_cmd);
extern void group_handle_heartbeat_message(address_t src);
extern void group_add_node_to_watch(address_t node_id);
#endif // GROUP_H
