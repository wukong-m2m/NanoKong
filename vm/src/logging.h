#ifndef LOGGING_H
#define LOGGING_H

#include "nvmcomm.h"
#include "wkpf_config.h"

#ifdef LOGGING

#define LOGF_HEAP(...) nvmcomm_send(wkpf_config_get_master_node_id(), NVMCOMM_LOGGING, (u08_t*)__VA_ARGS__) // for heap operations
#define LOGF_COMM(...) nvmcomm_send(wkpf_config_get_master_node_id(), NVMCOMM_LOGGING, (u08_t*)__VA_ARGS__) // for nvmcomm
#define LOGF_GROUP(...) nvmcomm_send(wkpf_config_get_master_node_id(), NVMCOMM_LOGGING, (u08_t*)__VA_ARGS__) // for group
#define LOGF_ZWAVETRACE(...) nvmcomm_send(wkpf_config_get_master_node_id(), NVMCOMM_LOGGING, (u08_t*)__VA_ARGS__) // for incoming zwave traffic per byte
#define LOGF_FLASH(...) nvmcomm_send(wkpf_config_get_master_node_id(), NVMCOMM_LOGGING, (u08_t*)__VA_ARGS__) // for avr_flash.c
#define LOGF_USART(...) nvmcomm_send(wkpf_config_get_master_node_id(), NVMCOMM_LOGGING, (u08_t*)__VA_ARGS__) // Sending message to master node to log
#define LOGF_XBEE(...) nvmcomm_send(wkpf_config_get_master_node_id(), NVMCOMM_LOGGING, (u08_t*)__VA_ARGS__) // XBEE
#define LOGF_WKPF(...) nvmcomm_send(wkpf_config_get_master_node_id(), NVMCOMM_LOGGING, (u08_t*)__VA_ARGS__) // wukong framework
#define LOGF_WKPFUPDATE(...) nvmcomm_send(wkpf_config_get_master_node_id(), NVMCOMM_LOGGING, (u08_t*)__VA_ARGS__) // wukong framework
#define LOGF(...) nvmcomm_send(wkpf_config_get_master_node_id(), NVMCOMM_LOGGING, (u08_t*)__VA_ARGS__) // wukong framework

#endif

#endif
