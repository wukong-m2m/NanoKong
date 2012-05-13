/**
 * Copyright (c) 2009 Andrew Rapp. All rights reserved.
 *
 * This file is part of XBee-Arduino.
 *
 * XBee-Arduino is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * XBee-Arduino is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with XBee-Arduino.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NVMCOMM_XBEE_H
#define NVMCOMM_XBEE_H

//#include <inttypes.h>
#include "types.h"

//#define SERIES_1
#define SERIES_2

// set to ATAP value of XBee. AP=2 is recommended
#define ATAP 2

#define START_BYTE 0x7e
#define ESCAPE 0x7d
#define XON 0x11
#define XOFF 0x13

// This value determines the size of the byte array for receiving RX packets
// Most users won't be dealing with packets this large so you can adjust this
// value to reduce memory consumption. But, remember that
// if a RX packet exceeds this size, it cannot be parsed!

// This value is determined by the largest packet size (100 byte payload + 64-bit address + option byte and rssi byte) of a series 1 radio
#define MAX_FRAME_DATA_SIZE 110

#define BROADCAST_ADDRESS 0xffff
#define ZB_BROADCAST_ADDRESS 0xfffe

// the non-variable length of the frame data (not including frame id or api id or variable data size (e.g. payload, at command set value)
#define ZB_TX_API_LENGTH 12
#define TX_16_API_LENGTH 3
#define TX_64_API_LENGTH 9
#define AT_COMMAND_API_LENGTH 2
#define REMOTE_AT_COMMAND_API_LENGTH 13
// start/length(2)/api/frameid/checksum bytes
#define PACKET_OVERHEAD_LENGTH 6
// api is always the third byte in packet
#define API_ID_INDEX 3

// frame position of rssi byte
#define RX_16_RSSI_OFFSET 2
#define RX_64_RSSI_OFFSET 8

#define DEFAULT_FRAME_ID 1
#define NO_RESPONSE_FRAME_ID 0

// TODO put in tx16 class
#define ACK_OPTION 0
#define DISABLE_ACK_OPTION 1
#define BROADCAST_OPTION 4

// RX options
#define ZB_PACKET_ACKNOWLEDGED 0x01
#define ZB_BROADCAST_PACKET 0x02

// not everything is implemented!
/**
 * Api Id constants
 */
#define TX_64_REQUEST 0x0
#define TX_16_REQUEST 0x1
#define AT_COMMAND_REQUEST 0x08
#define AT_COMMAND_QUEUE_REQUEST 0x09
#define REMOTE_AT_REQUEST 0x17
#define ZB_TX_REQUEST 0x10
#define ZB_EXPLICIT_TX_REQUEST 0x11
#define RX_64_RESPONSE 0x80
#define RX_16_RESPONSE 0x81
#define RX_64_IO_RESPONSE 0x82
#define RX_16_IO_RESPONSE 0x83
#define AT_RESPONSE 0x88
#define TX_STATUS_RESPONSE 0x89
#define MODEM_STATUS_RESPONSE 0x8a
#define ZB_RX_RESPONSE 0x90
#define ZB_EXPLICIT_RX_RESPONSE 0x91
#define ZB_TX_STATUS_RESPONSE 0x8b
#define ZB_IO_SAMPLE_RESPONSE 0x92
#define ZB_IO_NODE_IDENTIFIER_RESPONSE 0x95
#define AT_COMMAND_RESPONSE 0x88
#define REMOTE_AT_COMMAND_RESPONSE 0x97


/**
 * TX STATUS constants
 */
#define	SUCCESS 0x0
#define CCA_FAILURE 0x2
#define INVALID_DESTINATION_WUOBJECT_SUCCESS 0x15
#define	NETWORK_ACK_FAILURE 0x21
#define NOT_JOINED_TO_NETWORK 0x22
#define	SELF_ADDRESSED 0x23
#define ADDRESS_NOT_FOUND 0x24
#define ROUTE_NOT_FOUND 0x25
#define PAYLOAD_TOO_LARGE 0x74

// modem status
#define HARDWARE_RESET 0
#define WATCHDOG_TIMER_RESET 1
#define ASSOCIATED 2
#define DISASSOCIATED 3
#define SYNCHRONIZATION_LOST 4
#define COORDINATOR_REALIGNMENT 5
#define COORDINATOR_STARTED 6

#define ZB_BROADCAST_RADIUS_MAX_HOPS 0

#define ZB_TX_UNICAST 0
#define ZB_TX_BROADCAST 8

#define AT_OK 0
#define AT_ERROR  1
#define AT_INVALID_COMMAND 2
#define AT_INVALID_PARAMETER 3
#define AT_NO_RESPONSE 4

#define NO_ERROR 0
#define CHECKSUM_FAILURE 1
#define PACKET_EXCEEDS_BYTE_ARRAY_LENGTH 2
#define UNEXPECTED_START_BYTE 3

/**
 * The super class of all XBee responses (RX packets)
 * Users should never attempt to create an instance of this class; instead
 * create an instance of a subclass
 * It is recommend to reuse subclasses to conserve memory
 */
typedef struct {
	// static const int MODEM_STATUS = 0x8a;

	// pointer to frameData
	uint8_t* _frameDataPtr;

    // private:
    uint8_t _apiId;
	uint8_t _msbLength;
	uint8_t _lsbLength;
	uint8_t _checksum;
	uint8_t _frameLength;
	uint8_t _errorCode;
	bool _complete;
} XBeeResponse;

/**
 * Represents a 64-bit XBee Address
 */
typedef struct{
    uint32_t _msb;
	uint32_t _lsb;
} XBeeAddress64;

/**
 * This class is extended by all Responses that include a frame id
 */
typedef struct {
    XBeeResponse super;
	uint8_t _frameId;
} FrameIdResponse;

/**
 * Common functionality for both Series 1 and 2 data RX data packets
 */
typedef struct{
    XBeeResponse super;
} RxDataResponse;

#ifdef SERIES_2
/**
 * Represents a Series 2 TX status packet
 */
typedef struct{
    FrameIdResponse super;
} ZBTxStatusResponse;

/**
 * Represents a Series 2 RX packet
 */
typedef struct{
    RxDataResponse super;
	XBeeAddress64 _remoteAddress64;
} ZBRxResponse;

/**
 * Represents a Series 2 RX I/O Sample packet
 */
typedef struct {
    ZBRxResponse super;
} ZBRxIoSampleResponse;
#endif

#ifdef SERIES_1
/**
 * Represents a Series 1 TX Status packet
 */
typedef struct{
    FrameIdResponse super;
} TxStatusResponse;


/**
 * Represents a Series 1 RX packet
 */

typedef struct { 
    RxDataResponse super;
} RxResponse;

/**
 * Represents a Series 1 16-bit address RX packet
 */
typedef struct { 
	RxResponse super;
	uint16_t _remoteAddress;
} Rx16Response;

/**
 * Represents a Series 1 64-bit address RX packet
 */
typedef struct{
	RxResponse super;
	XBeeAddress64 _remoteAddress;
} Rx64Response;

/**
 * Represents a Series 1 RX I/O Sample packet
 */
typedef struct {
    RxResponse super;
} RxIoSampleBaseResponse;

typedef struct {
    RxIoSampleBaseResponse super;
} Rx16IoSampleResponse;

typedef struct {
    RxIoSampleBaseResponse super;
	XBeeAddress64 _remoteAddress;
} Rx64IoSampleResponse;
#endif

/**
 * Represents a Modem Status RX packet
 */
typedef struct{
    XBeeResponse super;
} ModemStatusResponse;

/**
 * Represents an AT Command RX packet
 */
typedef struct{
    FrameIdResponse super;
} AtCommandResponse;

/**
 * Represents a Remote AT Command RX packet
 */
typedef struct {
    AtCommandResponse super;
	XBeeAddress64 _remoteAddress64;
} RemoteAtCommandResponse;


/**
 * Super class of all XBee requests (TX packets)
 * Users should never create an instance of this class; instead use an subclass of this class
 * It is recommended to reuse Subclasses of the class to conserve memory
 * <p/>
 * This class allocates a buffer to
 */
typedef struct{
	uint8_t _apiId;
	uint8_t _frameId;
} XBeeRequest;

// TODO add reset/clear method since responses are often reused
/**
 * Primary interface for communicating with an XBee Radio.
 * This class provides methods for sending and receiving packets with an XBee radio via the serial port.
 * The XBee radio must be configured in API (packet) mode (AP=2)
 * in order to use this software.
 * <p/>
 * Since this code is designed to run on a microcontroller, with only one thread, you are responsible for reading the
 * data off the serial buffer in a timely manner.  This involves a call to a variant of readPacket(...).
 * If your serial port is receiving data faster than you are reading, you can expect to lose packets.
 * Arduino only has a 128 byte serial buffer so it can easily overflow if two or more packets arrive
 * without a call to readPacket(...)
 * <p/>
 * In order to conserve resources, this class only supports storing one response packet in memory at a time.
 * This means that you must fully consume the packet prior to calling readPacket(...), because calling
 * readPacket(...) overwrites the previous response.
 * <p/>
 * This class creates an array of size MAX_FRAME_DATA_SIZE for storing the response packet.  You may want
 * to adjust this value to conserve memory.
 *
 * \author Andrew Rapp
 */
typedef struct {
	XBeeResponse _response;
	// current packet position for response.  just a state variable for packet parsing and has no relevance for the response otherwise
	uint8_t _pos;
	// last byte read
	uint8_t b;
	uint8_t _checksumTotal;
	uint8_t _nextFrameId;
	// buffer for incoming RX packets.  holds only the api specific frame data, starting after the api id byte and prior to checksum
	uint8_t _responseFrameData[MAX_FRAME_DATA_SIZE];
	bool _escape;
} XBee;

/**
 * All TX packets that support payloads extend this class
 */
typedef struct {
    XBeeRequest super;
    //private:
	uint8_t* _payloadPtr;
	uint8_t _payloadLength;
} PayloadRequest;

#ifdef SERIES_1

/**
 * Represents a Series 1 TX packet that corresponds to Api Id: TX_16_REQUEST
 * <p/>
 * Be careful not to send a data array larger than the max packet size of your radio.
 * This class does not perform any validation of packet size and there will be no indication
 * if the packet is too large, other than you will not get a TX Status response.
 * The datasheet says 100 bytes is the maximum, although that could change in future firmware.
 */
typedef struct {
    PayloadRequest super;
    //private:
	uint16_t _addr16;
	uint8_t _option;
} Tx16Request;

/**
 * Represents a Series 1 TX packet that corresponds to Api Id: TX_64_REQUEST
 *
 * Be careful not to send a data array larger than the max packet size of your radio.
 * This class does not perform any validation of packet size and there will be no indication
 * if the packet is too large, other than you will not get a TX Status response.
 * The datasheet says 100 bytes is the maximum, although that could change in future firmware.
 */
typedef struct {
    PayloadRequest super;
    //private:
	XBeeAddress64 _addr64;
	uint8_t _option;
} Tx64Request;

#endif


#ifdef SERIES_2

/**
 * Represents a Series 2 TX packet that corresponds to Api Id: ZB_TX_REQUEST
 *
 * Be careful not to send a data array larger than the max packet size of your radio.
 * This class does not perform any validation of packet size and there will be no indication
 * if the packet is too large, other than you will not get a TX Status response.
 * The datasheet says 72 bytes is the maximum for ZNet firmware and ZB Pro firmware provides
 * the ATNP command to get the max supported payload size.  This command is useful since the
 * maximum payload size varies according to certain settings, such as encryption.
 * ZB Pro firmware provides a PAYLOAD_TOO_LARGE that is returned if payload size
 * exceeds the maximum.
 */
typedef struct {
    PayloadRequest super;
	XBeeAddress64 _addr64;
	uint16_t _addr16;
	uint8_t _broadcastRadius;
	uint8_t _option;
} ZBTxRequest;

#endif

/**
 * Represents an AT Command TX packet
 * The command is used to configure the serially connected XBee radio
 */
typedef struct {
    XBeeRequest super;
    //private:
	uint8_t *_command;
	uint8_t *_commandValue;
	uint8_t _commandValueLength;
} AtCommandRequest;

/**
 * Represents an Remote AT Command TX packet
 * The command is used to configure a remote XBee radio
 */
typedef struct {
    AtCommandRequest super;
    XBeeAddress64 _remoteAddress64;
	uint16_t _remoteAddress16;
	bool _applyChanges;
} RemoteAtCommandRequest;

extern void nvmcomm_xbee_init(void);
extern void nvmcomm_xbee_setcallback(void (*func)(address_t, u08_t, u08_t *, u08_t));
extern void nvmcomm_xbee_poll(void);
extern int nvmcomm_xbee_send(address_t dest, u08_t nvc3_command, u08_t *d, u08_t l, u08_t option);

#endif //NVMCOMM_XBEE_H
