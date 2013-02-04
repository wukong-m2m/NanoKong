#ifndef GROUP_H
#define GROUP_H

#include "types.h"

extern void send_heartbeat();
extern void handle_failure();
extern void group_handle_update(address_t src, u08_t nvmcomm_command, u08_t *payload, u08_t *response_size, u08_t *response_cmd);
extern void group_handle_message(address_t src, u08_t nvmcomm_command, u08_t *payload, u08_t *response_size, u08_t *response_cmd);
extern void group_handle_heartbeat_message(address_t src);
extern void group_handle_probe_message(address_t src, u08_t nvmcomm_command, u08_t *payload, u08_t *response_size, u08_t *response_cmd);
extern void group_add_node_to_watch(address_t node_id);
extern void group_remove_node_from_watchlist(address_t node_id);
extern void group_setup_watch_list(uint8_t shift, uint16_t length, address_t* node_ids);
#endif // GROUP_H
