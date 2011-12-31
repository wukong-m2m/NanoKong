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

#include "nvmcomm_xbee.h"
#include "config.h"
#include "uart.h"
#include "debug.h"
#include "delay.h"
#include "nvmcomm3.h"

#ifdef NVM_USE_COMMXBEE

#define XBEE_UART 2
#define XBEE_UART_BAUDRATE 9600
#define NUM_ADDR 2
void (*f)(address_t src, u08_t nvc3_command, u08_t *payload, u08_t length); // The callback function registered by callback

/**
 * XBee init code
 * Begin
 */

void XBeeResponse_init(XBeeResponse* _response)
{
    _response->_complete = false;
    _response->_errorCode = NO_ERROR;
    _response->_checksum = 0;
}

void XBee_init(XBee* xbee)
{
    xbee->_pos = 0;
    xbee->_escape = false;
    xbee->_checksumTotal = 0;
    xbee->_nextFrameId = 0;
    XBeeResponse_init(&(xbee->_response));
    xbee->_response._frameDataPtr = xbee->_responseFrameData;
    if(XBEE_UART) uart_init(XBEE_UART, XBEE_UART_BAUDRATE);
}

void XBeeAddress64_init(XBeeAddress64* addr, uint32_t msb, uint32_t lsb)
{
    addr->_msb = msb;
    addr->_lsb = lsb;
}

void XBeeRequest_init(XBeeRequest* xbeerq, uint8_t apiId, uint8_t frameId)
{
    xbeerq->_apiId = apiId;
    xbeerq->_frameId = frameId;
}

void AtCommandRequest_init(AtCommandRequest* at, uint8_t *command, uint8_t *commandValue, uint8_t commandValueLength)
{
    XBeeRequest_init(&(at->super), AT_COMMAND_REQUEST, DEFAULT_FRAME_ID);
    at->_command = command;
	at->_commandValue = commandValue;
	at->_commandValueLength = commandValueLength;
}

void AtCommandRequest_init0(AtCommandRequest* at)
{
    AtCommandRequest_init(at, NULL, NULL, 0);
}

void AtCommandRequest_init1(AtCommandRequest* at, uint8_t *command)
{
    AtCommandRequest_init(at, command, NULL, 0);
}

void RemoteAtCommandRequest_init(RemoteAtCommandRequest* at, XBeeAddress64* remoteAddress64, uint16_t remoteAddress16, uint8_t *command, uint8_t *commandValue, uint8_t commandValueLength)
{
    AtCommandRequest_init(&(at->super), command, commandValue, commandValueLength);
	at->super.super._apiId = REMOTE_AT_REQUEST;
	at->_applyChanges = (commandValue!=NULL ? true:false);
    if(remoteAddress64!=NULL){
        at->_remoteAddress64._msb = remoteAddress64->_msb;
        at->_remoteAddress64._lsb = remoteAddress64->_lsb;
	    // don't worry.. works for series 1 too!
	    at->_remoteAddress16 = ZB_BROADCAST_ADDRESS;
    }else{
        at->_remoteAddress64._msb = 0x0;
        at->_remoteAddress64._lsb = BROADCAST_ADDRESS;
	    at->_remoteAddress16 = remoteAddress16;
    }
}

void PayloadRequest_init(PayloadRequest* payldrq, uint8_t apiId, uint8_t frameId, uint8_t *payload, uint8_t payloadLength)
{
    XBeeRequest_init(&(payldrq->super), apiId, frameId);
    payldrq->_payloadPtr = payload;
    payldrq->_payloadLength = payloadLength;
}
#ifdef SERIES_1
void Tx16Request_init(Tx16Request* tx16, uint16_t addr16, uint8_t option, uint8_t *data, uint8_t dataLength, uint8_t frameId)
{
    PayloadRequest_init(&(tx16->super), TX_16_REQUEST, frameId, data, dataLength);
	tx16->_addr16 = addr16;
	tx16->_option = option;
}

void Tx16Request_init3(Tx16Request* tx16, uint16_t addr16, uint8_t *data, uint8_t dataLength)
{
    Tx16Request_init(tx16, addr16, ACK_OPTION, data, dataLength, DEFAULT_FRAME_ID);
}

void Tx16Request_init0(Tx16Request* tx16)
{
    PayloadRequest_init(&(tx16->super), TX_16_REQUEST, DEFAULT_FRAME_ID, NULL, 0);
}

void Tx64Request_init(Tx64Request* tx64, XBeeAddress64* addr64, uint8_t option, uint8_t *data, uint8_t dataLength, uint8_t frameId)
{
    PayloadRequest_init(&(tx64->super), TX_64_REQUEST, frameId, data, dataLength);
	tx64->_addr64._msb = addr64->_msb;
	tx64->_addr64._lsb = addr64->_lsb;
	tx64->_option = option;
}

void Tx64Request_init3(Tx64Request* tx64, XBeeAddress64* addr64, uint8_t *data, uint8_t dataLength)
{
    Tx64Request_init(tx64, addr64, ACK_OPTION, data, dataLength, DEFAULT_FRAME_ID);
}

void Tx64Request_init0(Tx64Request* tx64)
{
    PayloadRequest_init(&(tx64->super), TX_64_REQUEST, DEFAULT_FRAME_ID, NULL, 0);
}
#endif
#ifdef SERIES_2
void ZBTxRequest_init(ZBTxRequest* zbtxrq, XBeeAddress64* addr64, uint16_t addr16, uint8_t broadcastRadius, uint8_t option, uint8_t *data, uint8_t dataLength, uint8_t frameId)
{
    PayloadRequest_init(&(zbtxrq->super), ZB_TX_REQUEST, frameId, data, dataLength);
    if(addr64) XBeeAddress64_init(&(zbtxrq->_addr64), addr64->_msb, addr64->_lsb);
    zbtxrq->_addr16 = addr16;
    zbtxrq->_broadcastRadius = broadcastRadius;
    zbtxrq->_option = option;
}

void ZBTxRequest_init0(ZBTxRequest* zbtxrq)
{
    PayloadRequest_init(&(zbtxrq->super), ZB_TX_REQUEST, DEFAULT_FRAME_ID, NULL, 0);
}

void ZBTxRequest_init3(ZBTxRequest* zbtxrq, XBeeAddress64* addr64, uint8_t *data, uint8_t dataLength)
{
    ZBTxRequest_init(zbtxrq, addr64, ZB_BROADCAST_ADDRESS, ZB_BROADCAST_RADIUS_MAX_HOPS, ZB_TX_UNICAST, data, dataLength, DEFAULT_FRAME_ID);
}
#endif


/* XBee init code End */

/**
 * XBee send code
 * Begin
 */
void XBee_sendByte(uint8_t b, bool escape)
{
    if (escape && (b == START_BYTE || b == ESCAPE || b == XON || b == XOFF)) {
        uart_write_byte(XBEE_UART, ESCAPE);
        uart_write_byte(XBEE_UART, b ^ 0x20);
    } else {
        uart_write_byte(XBEE_UART, b);
    }
}

void XBee_send(void* request, uint8_t _apiId, uint8_t _frameId, uint8_t frameDataLength, uint8_t (*getFrameData)(void*, uint8_t) )
{
    // read out pending requests
    //nvmcomm_xbee_poll();

    // the new new deal
    XBee_sendByte(START_BYTE, false);

    // send length
    uint8_t msbLen = ((frameDataLength + 2) >> 8) & 0xff;
    uint8_t lsbLen = (frameDataLength + 2) & 0xff;

    XBee_sendByte(msbLen, true);
    XBee_sendByte(lsbLen, true);

    // api id
    XBee_sendByte(_apiId, true);
    XBee_sendByte(_frameId, true);

    uint8_t checksum = 0;

    // compute checksum, start at api id
    checksum+= _apiId;
    checksum+= _frameId;

    for (int i = 0; i < frameDataLength; i++) {
        XBee_sendByte((*getFrameData)(request, i), true);
        checksum+= (*getFrameData)(request, i);
    }

    // perform 2s complement
    checksum = 0xff - checksum;

    // send checksum
    XBee_sendByte(checksum, true);

    // send packet
    uart_flush(XBEE_UART);
}
#ifdef SERIES_1
uint8_t getFrameData64(void* request, uint8_t pos)
{
    Tx64Request* req = (Tx64Request*) request;
    if (pos == 0) {
        return (req->_addr64._msb >> 24) & 0xff;
    } else if (pos == 1) {
        return (req->_addr64._msb >> 16) & 0xff;
    } else if (pos == 2) {
        return (req->_addr64._msb >> 8) & 0xff;
    } else if (pos == 3) {
        return req->_addr64._msb & 0xff;
    } else if (pos == 4) {
        return (req->_addr64._lsb >> 24) & 0xff;
    } else if (pos == 5) {
        return (req->_addr64._lsb >> 16) & 0xff;
    } else if (pos == 6) {
        return(req->_addr64._lsb >> 8) & 0xff;
    } else if (pos == 7) {
        return req->_addr64._lsb & 0xff;
    } else if (pos == 8) {
        return req->_option;
    } else {
        return (req->super._payloadPtr)[pos - TX_64_API_LENGTH];
    }
}

uint8_t getFrameData16(void* request, uint8_t pos)
{
    Tx16Request* req = (Tx16Request*) request;
    if (pos == 0) {
        return (req->_addr16 >> 8) & 0xff;
    } else if (pos == 1) {
        return req->_addr16 & 0xff;
    } else if (pos == 2) {
        return req->_option;
    } else {
        return (req->super._payloadPtr)[pos - TX_16_API_LENGTH];
    }
}
#endif
uint8_t getFrameDataAtCmd(void* request, uint8_t pos)
{
    AtCommandRequest* req = (AtCommandRequest*) request;
    if (pos == 0) {
        return (req->_command)[0];
    } else if (pos == 1) {
        return (req->_command)[1];
    } else {
        return (req->_commandValue)[pos - AT_COMMAND_API_LENGTH];
    }
}

uint8_t getFrameDataRemoteAtCmd(void* request, uint8_t pos)
{
    RemoteAtCommandRequest* req = (RemoteAtCommandRequest*) request;
    if (pos == 0) {
        return (req->_remoteAddress64._msb >> 24) & 0xff;
    } else if (pos == 1) {
        return (req->_remoteAddress64._msb >> 16) & 0xff;
    } else if (pos == 2) {
        return (req->_remoteAddress64._msb >> 8) & 0xff;
    } else if (pos == 3) {
        return req->_remoteAddress64._msb & 0xff;
    } else if (pos == 4) {
        return (req->_remoteAddress64._lsb >> 24) & 0xff;
    } else if (pos == 5) {
        return (req->_remoteAddress64._lsb >> 16) & 0xff;
    } else if (pos == 6) {
        return(req->_remoteAddress64._lsb >> 8) & 0xff;
    } else if (pos == 7) {
        return req->_remoteAddress64._lsb & 0xff;
    } else if (pos == 8) {
        return (req->_remoteAddress16 >> 8) & 0xff;
    } else if (pos == 9) {
        return req->_remoteAddress16 & 0xff;
    } else if (pos == 10) {
        return req->_applyChanges ? 2: 0;
    } else if (pos == 11) {
        return (req->super._command)[0];
    } else if (pos == 12) {
        return (req->super._command)[1];
    } else {
        return (req->super._command)[pos - REMOTE_AT_COMMAND_API_LENGTH];
    }
}
#ifdef SERIES_2
uint8_t getFrameDataZB(void* request, uint8_t pos)
{
    ZBTxRequest* req = (ZBTxRequest*) request;
    if (pos == 0) {
        return (req->_addr64._msb >> 24) & 0xff;
    } else if (pos == 1) {
        return (req->_addr64._msb >> 16) & 0xff;
    } else if (pos == 2) {
        return (req->_addr64._msb >> 8) & 0xff;
    } else if (pos == 3) {
        return req->_addr64._msb & 0xff;
    } else if (pos == 4) {
        return (req->_addr64._lsb >> 24) & 0xff;
    } else if (pos == 5) {
        return (req->_addr64._lsb >> 16) & 0xff;
    } else if (pos == 6) {
        return(req->_addr64._lsb >> 8) & 0xff;
    } else if (pos == 7) {
        return req->_addr64._lsb & 0xff;
    } else if (pos == 8) {
        return (req->_addr16 >> 8) & 0xff;
    } else if (pos == 9) {
        return req->_addr16 & 0xff;
    } else if (pos == 10) {
        return req->_broadcastRadius;
    } else if (pos == 11) {
        return req->_option;
    } else {
        return (req->super._payloadPtr)[pos - ZB_TX_API_LENGTH];
    }
}
#endif
#ifdef SERIES_1
void send64(uint8_t *buf, uint8_t len, uint32_t msb, uint32_t lsb)
{
    XBeeAddress64 addr;
    XBeeAddress64_init(&addr, msb, lsb);
    Tx64Request request;
    Tx64Request_init3(&request, &addr, buf, len);
    XBeeRequest* root = &(request.super.super);
    XBee_send(&request, root->_apiId, root->_frameId, TX_64_API_LENGTH+(request.super._payloadLength), &getFrameData64);
}

void send16(uint8_t *buf, uint8_t len, uint16_t addr)
{
    Tx16Request request;
    Tx16Request_init3(&request, addr, buf, len);
    XBeeRequest* root = &(request.super.super);
    XBee_send(&request, root->_apiId, root->_frameId, TX_16_API_LENGTH+(request.super._payloadLength), &getFrameData16);
}
#endif
void sendAtCmd(uint8_t *command, uint8_t *commandValue)
{
    AtCommandRequest request;
    if(commandValue!=NULL) AtCommandRequest_init(&(request), command, commandValue, sizeof(commandValue));
    else AtCommandRequest_init1(&(request), command);
    XBeeRequest* root = &(request.super);
    XBee_send(&request, root->_apiId, root->_frameId, AT_COMMAND_API_LENGTH+(request._commandValueLength), &getFrameDataAtCmd);
}

void sendRemoteAtCmd64(uint8_t *command, uint8_t *commandValue, uint32_t msb, uint32_t lsb)
{
    XBeeAddress64 addr;
    XBeeAddress64_init(&addr, msb, lsb);
    RemoteAtCommandRequest request;
    uint8_t size = (commandValue!=NULL) ? sizeof(commandValue):0;
    RemoteAtCommandRequest_init(&request, &addr, 0, command, commandValue, size);
    XBeeRequest* root = &(request.super.super);
    XBee_send(&request, root->_apiId, root->_frameId, REMOTE_AT_COMMAND_API_LENGTH+(request.super._commandValueLength), &getFrameDataRemoteAtCmd);
}

void sendRemoteAtCmd16(uint8_t *command, uint8_t *commandValue, uint16_t addr)
{
    RemoteAtCommandRequest request;
    uint8_t size = (commandValue!=NULL) ? sizeof(commandValue):0;
    RemoteAtCommandRequest_init(&request, NULL, addr, command, commandValue, size);
    XBeeRequest* root = &(request.super.super);
    XBee_send(&request, root->_apiId, root->_frameId, REMOTE_AT_COMMAND_API_LENGTH+(request.super._commandValueLength), &getFrameDataRemoteAtCmd);
}
#ifdef SERIES_2
void sendZB(uint8_t *buf, uint8_t len, uint32_t msb, uint32_t lsb)
{
    XBeeAddress64 addr;
    ZBTxRequest request;
    XBeeAddress64_init(&addr, msb, lsb);
    ZBTxRequest_init3(&request, &addr, buf, len);

    XBeeRequest* root = &(request.super.super);
    XBee_send(&request, root->_apiId, root->_frameId, ZB_TX_API_LENGTH+(request.super._payloadLength), &getFrameDataZB);
}
#endif
/* XBee send code End */


/**
 * XBee read code
 * Begin
 */

void XBeeResponse_reset(XBeeResponse* _response)
{
    XBeeResponse_init(_response);
    _response->_apiId = 0;
    _response->_msbLength = 0;
    _response->_lsbLength = 0;
    _response->_frameLength = 0;
    for (int i = 0; i < MAX_FRAME_DATA_SIZE; i++) {
        (_response->_frameDataPtr)[i] = 0;
    }
}

bool XBeeResponse_isAvailable(XBeeResponse* _response)
{
    return _response->_complete;
}

bool XBee_isAvailable(XBee* xbee)
{
    return XBeeResponse_isAvailable(&(xbee->_response));
}

bool XBeeResponse_isError(XBeeResponse* _response)
{
    return _response->_errorCode > 0;
}

bool XBee_isError(XBee* xbee)
{
    return XBeeResponse_isError(&(xbee->_response));
}

uint16_t XBeeResponse_getPacketLength(XBeeResponse* _response)
{
    return ((_response->_msbLength << 8) & 0xff) + (_response->_lsbLength & 0xff);
}

void XBee_resetResponse(XBee* xbee)
{
    xbee->_pos = 0;
    xbee->_escape = false;
    XBeeResponse_reset(&(xbee->_response));
}

void XBee_readPacket(XBee* xbee){
    XBeeResponse* _response = &(xbee->_response);
    uint8_t *b = &(xbee->b), *_pos = &(xbee->_pos);
    bool *_escape = &(xbee->_escape);
    // reset previous response
    if (XBeeResponse_isAvailable(_response) || XBeeResponse_isError(_response)) {
        // discard previous packet and start over
        XBee_resetResponse(xbee);
    }
    while (uart_available(XBEE_UART)) {
        *b = uart_read_byte(XBEE_UART);
        if (*_pos > 0 && *b == START_BYTE && ATAP == 2) {
            // new packet start before previous packeted completed -- discard previous packet and start over
            _response->_errorCode = UNEXPECTED_START_BYTE;
            return;
        }
        if (*_pos > 0 && *b == ESCAPE) {
            if (uart_available(XBEE_UART)) {
                *b = uart_read_byte(XBEE_UART);
                *b = 0x20 ^ *b;
            } else {
                // escape byte.  next byte will be
                *_escape = true;
                continue;
            }
        }
        if (*_escape == true) {
            *b = 0x20 ^ *b;
            *_escape = false;
        }
        // checksum includes all bytes starting with api id
        if (*_pos >= API_ID_INDEX) {
            (xbee->_checksumTotal) += *b;
        }
        switch(*_pos) {
            case 0:
                if (*b == START_BYTE) {
                    (*_pos)++;
                }
                break;
            case 1:
                // length msb
                _response->_msbLength = *b;
                (*_pos)++;
                break;
            case 2:
                // length lsb
                _response->_lsbLength = *b;
                (*_pos)++;
                break;
            case 3:
                _response->_apiId = *b;
                (*_pos)++;
                break;
            default:
                // starts at fifth byte
                if (*_pos > MAX_FRAME_DATA_SIZE) {
                    // exceed max size.  should never occur
                    _response->_errorCode = PACKET_EXCEEDS_BYTE_ARRAY_LENGTH;
                    return;
                }
                // check if we're at the end of the packet
                // packet length does not include start, length, or checksum bytes, so add 3
                if (*_pos == (XBeeResponse_getPacketLength(_response) + 3)) {
                    // verify checksum
                    if ((xbee->_checksumTotal & 0xff) == 0xff) {
                        _response->_checksum = *b;
                        _response->_complete = true;
                        _response->_errorCode = NO_ERROR;
                    } else {
                        // checksum failed
                        _response->_errorCode = CHECKSUM_FAILURE;
                    }
                    // minus 4 because we start after start,msb,lsb,api and up to but not including checksum
                    // e.g. if frame was one byte, _pos=4 would be the byte, pos=5 is the checksum, where end stop reading
                    _response->_frameLength = *_pos - 4;

                    // reset state vars
                    *_pos = 0;

                    xbee->_checksumTotal = 0;
                    return;
                } else {
                    // add to packet array, starting with the fourth byte of the apiFrame
                    (_response->_frameDataPtr)[*_pos - 4] = *b;
                    (*_pos)++;
                }
        }
    }
}

bool XBee_readPacket_timeout(XBee* xbee, uint32_t timeout)
{
    if (timeout < 1) {
        return false;
    }

    uint32_t start = 0;
    XBeeResponse* _response = &(xbee->_response);
    while (start < timeout) {
        XBee_readPacket(xbee);
        if (XBeeResponse_isAvailable(_response)) {
            return true;
        } else if (XBeeResponse_isError(_response)) {
            return false;
        }
        delay(MILLISEC(1));
        start++;
    }
    // timed out
    return false;
}

void XBeeResponse_setCommon(XBeeResponse* source, XBeeResponse* target)
{
    target->_apiId = source->_apiId;
    target->_checksum = source->_checksum;
    target->_complete = source->_complete;
    target->_errorCode = source->_errorCode;
    target->_frameLength = source->_frameLength;
    target->_msbLength = source->_msbLength;
    target->_lsbLength = source->_lsbLength;
}

uint8_t XBee_getResponseApiId(XBee* xbee)
{
    return xbee->_response._apiId;
}

void XBee_getResponse(XBee* xbee, void* response, uint8_t apiId)
{
    XBeeResponse* source = &(xbee->_response), *target = NULL;
    switch(apiId){
#ifdef SERIES_2
        case ZB_TX_STATUS_RESPONSE:
            {
                ZBTxStatusResponse* zb = (ZBTxStatusResponse*)response;
                // pass pointer array to subclass
                target = &(zb->super.super);
                target->_frameDataPtr = source->_frameDataPtr;
                XBeeResponse_setCommon(source, target);
                break;
            }
        case ZB_RX_RESPONSE:
            {
                ZBRxResponse* zb = (ZBRxResponse*) response;
                target = &(zb->super.super);
                target->_frameDataPtr = source->_frameDataPtr;
                XBeeResponse_setCommon(source, target);

                uint8_t *frameDataPtr = source->_frameDataPtr;
                zb->_remoteAddress64._msb = ((uint32_t)(frameDataPtr[0]) << 24) + ((uint32_t)(frameDataPtr[1]) << 16) + ((uint16_t)(frameDataPtr[2]) << 8) + frameDataPtr[3];
                zb->_remoteAddress64._lsb = ((uint32_t)(frameDataPtr[4]) << 24) + ((uint32_t)(frameDataPtr[5]) << 16) + ((uint16_t)(frameDataPtr[6]) << 8) + frameDataPtr[7];
                break;
            }
        case ZB_IO_SAMPLE_RESPONSE:
            {
                ZBRxIoSampleResponse* zb = (ZBRxIoSampleResponse*)response;
                // pass pointer array to subclass
                target = &(zb->super.super.super);
                target->_frameDataPtr = source->_frameDataPtr;
                XBeeResponse_setCommon(source, target);

                uint8_t *frameDataPtr = source->_frameDataPtr;
                zb->super._remoteAddress64._msb = (((uint32_t)(frameDataPtr[0]) << 24) + ((uint32_t)(frameDataPtr[1]) << 16) + ((uint16_t)(frameDataPtr[2]) << 8) + frameDataPtr[3]);
                zb->super._remoteAddress64._lsb = (((uint32_t)(frameDataPtr[4]) << 24) + ((uint32_t)(frameDataPtr[5]) << 16) + ((uint16_t)(frameDataPtr[6]) << 8) + (frameDataPtr[7]));
                break;
            }
#endif
#ifdef SERIES_1
        case TX_STATUS_RESPONSE:
            {
                TxStatusResponse* txStatus = (TxStatusResponse*)response;
                // pass pointer array to subclass
                target = &(txStatus->super.super);
                target->_frameDataPtr = source->_frameDataPtr;
                XBeeResponse_setCommon(source, target);
                break;
            }
        case RX_16_RESPONSE:
            {
                Rx16Response* rx16 = (Rx16Response*)response;
                // pass pointer array to subclass
                target = &(rx16->super.super.super);
                target->_frameDataPtr = source->_frameDataPtr;
                XBeeResponse_setCommon(source, target);
                break;
            }
        case RX_64_RESPONSE:
            {
                Rx64Response* rx64 = (Rx64Response*)response;
                // pass pointer array to subclass
                target = &(rx64->super.super.super);
                target->_frameDataPtr = source->_frameDataPtr;
                XBeeResponse_setCommon(source, target);

                uint8_t *frameDataPtr = source->_frameDataPtr;
                rx64->_remoteAddress._msb = (((uint32_t)(frameDataPtr[0]) << 24) + ((uint32_t)(frameDataPtr[1]) << 16) + ((uint16_t)(frameDataPtr[2]) << 8) + frameDataPtr[3]);
                rx64->_remoteAddress._lsb = (((uint32_t)(frameDataPtr[4]) << 24) + ((uint32_t)(frameDataPtr[5]) << 16) + ((uint16_t)(frameDataPtr[6]) << 8) + frameDataPtr[7]);
                break;
            }
        case RX_16_IO_RESPONSE:
            {
                Rx16IoSampleResponse* rx = (Rx16IoSampleResponse*)response;
                target = &(rx->super.super.super.super);
                target->_frameDataPtr = source->_frameDataPtr;
                XBeeResponse_setCommon(source, target);
                break;
            }
        case RX_64_IO_RESPONSE:
            {
                Rx64IoSampleResponse* rx = (Rx64IoSampleResponse*)response;
                target = &(rx->super.super.super.super);
                target->_frameDataPtr = source->_frameDataPtr;
                XBeeResponse_setCommon(source, target);

                uint8_t *frameDataPtr = source->_frameDataPtr;
                rx->_remoteAddress._msb = (((uint32_t)(frameDataPtr[0]) << 24) + ((uint32_t)(frameDataPtr[1]) << 16) + ((uint16_t)(frameDataPtr[2]) << 8) + frameDataPtr[3]);
                rx->_remoteAddress._lsb = (((uint32_t)(frameDataPtr[4]) << 24) + ((uint32_t)(frameDataPtr[5]) << 16) + ((uint16_t)(frameDataPtr[6]) << 8) + frameDataPtr[7]);
                break;
            }
#endif
        case AT_COMMAND_RESPONSE:
            {
                AtCommandResponse* at = (AtCommandResponse*)response;
                // pass pointer array to subclass
                target = &(at->super.super);
                target->_frameDataPtr = source->_frameDataPtr;
                XBeeResponse_setCommon(source, target);
                break;
            }
        case REMOTE_AT_COMMAND_RESPONSE:
            {
                // TODO no real need to cast.  change arg to match expected class
                RemoteAtCommandResponse* at = (RemoteAtCommandResponse*)response;

                // pass pointer array to subclass
                target = &(at->super.super.super);
                target->_frameDataPtr = source->_frameDataPtr;
                XBeeResponse_setCommon(source, target);

                uint8_t *frameDataPtr = source->_frameDataPtr;
                at->_remoteAddress64._msb = (((uint32_t)(frameDataPtr[1]) << 24) + ((uint32_t)(frameDataPtr[2]) << 16) + ((uint16_t)(frameDataPtr[3]) << 8) + frameDataPtr[4]);
                at->_remoteAddress64._lsb = (((uint32_t)(frameDataPtr[5]) << 24) + ((uint32_t)(frameDataPtr[6]) << 16) + ((uint16_t)(frameDataPtr[7]) << 8) + (frameDataPtr[8]));
                break;
            }
        case MODEM_STATUS_RESPONSE:
            {
                ModemStatusResponse* modem = (ModemStatusResponse*)response;

                // pass pointer array to subclass
                target = &(modem->super);
                target->_frameDataPtr = source->_frameDataPtr;
                XBeeResponse_setCommon(source, target);
                break;
            }
        default:
            break;
    }
}

uint8_t XBeeResponse_getDataLength(XBeeResponse* source)
{
    switch(source->_apiId){
        // ZBRxResponse (sub)classes
        case ZB_RX_RESPONSE:
        case ZB_IO_SAMPLE_RESPONSE:
            // markers to read data from packet array.  this is the index, so the 12th item in the array
            // offset = 11
            return XBeeResponse_getPacketLength(source) - (11) - 1;
        // RxResponse (sub)classes
        case RX_16_RESPONSE:
        case RX_64_RESPONSE:
        case RX_16_IO_RESPONSE:
        case RX_64_IO_RESPONSE:
            return XBeeResponse_getPacketLength(source) - (RX_16_RSSI_OFFSET + 2) - 1;
            // Instead, get value length for command classes
        case AT_COMMAND_RESPONSE:
            return source->_frameLength - 14;
        case REMOTE_AT_COMMAND_RESPONSE:
            return source->_frameLength - 4;
        case MODEM_STATUS_RESPONSE:
        case TX_STATUS_RESPONSE:
        case ZB_TX_STATUS_RESPONSE:
        default:
            break;
    }
    return -1;
}

uint8_t XBee_getDataLength(XBee* xbee)
{
    XBeeResponse* source = &(xbee->_response);
    return XBeeResponse_getDataLength(source);
}

uint8_t* XBee_getData(XBee* xbee)
{
    XBeeResponse* source = &(xbee->_response);
    switch(source->_apiId){
        // RxDataResponse (sub)classes
        case ZB_RX_RESPONSE:
        case ZB_IO_SAMPLE_RESPONSE:
            {
                uint8_t dataOffset = 11;
                return source->_frameDataPtr + dataOffset;
            }
        case RX_16_RESPONSE:
        case RX_64_RESPONSE:
        case RX_16_IO_RESPONSE:
        case RX_64_IO_RESPONSE:
            {
                uint8_t dataOffset = RX_16_RSSI_OFFSET + 2;
                return source->_frameDataPtr + dataOffset;
            }
            // Instead, get value for command classes
        case AT_COMMAND_RESPONSE:
            if (XBeeResponse_getDataLength(source) > 0) {
                // value is only included for query commands.  set commands does not return a value
                return source->_frameDataPtr + 4;
            }
            break;
        case REMOTE_AT_COMMAND_RESPONSE:
            if (XBeeResponse_getDataLength(source) > 0) {
                // value is only included for query commands.  set commands does not return a value
                return source->_frameDataPtr + 14;
            }
            break;
            // No data but status (sub)classes
        case MODEM_STATUS_RESPONSE:
        case TX_STATUS_RESPONSE:
        case ZB_TX_STATUS_RESPONSE:
        default:
            break;
    }
    return NULL;
}

uint8_t XBee_getDataByIndex(XBee* xbee, int index)
{
    uint8_t* _frameDataPtr = XBee_getData(xbee);
    if(_frameDataPtr) return _frameDataPtr[index];
    return -1;
}

uint8_t XBee_getResponseFrameDataLength(XBee* xbee)
{
    return xbee->_response._frameLength;
}

uint8_t XBee_getResponseErrorCode(XBee* xbee)
{
    return xbee->_response._errorCode;
}

XBeeResponse* getXBeeResponse(void* response, uint8_t apiId)
{
    switch(apiId){
#ifdef SERIES_2
        case ZB_TX_STATUS_RESPONSE:
            {
                ZBTxStatusResponse* resp = (ZBTxStatusResponse*) response;
                return &(resp->super.super);
            }
        case ZB_RX_RESPONSE:
            {
                ZBRxResponse* resp = (ZBRxResponse*) response;
                return &(resp->super.super);
            }
        case ZB_IO_SAMPLE_RESPONSE:
            {
                ZBRxIoSampleResponse* resp = (ZBRxIoSampleResponse*) response;
                return &(resp->super.super.super);
            }
#endif
#ifdef SERIES_1
        case TX_STATUS_RESPONSE:
            {
                TxStatusResponse* resp = (TxStatusResponse*) response;
                return &(resp->super.super);
            }
        case RX_16_RESPONSE:
            {
                Rx16Response* resp = (Rx16Response*) response;
                return &(resp->super.super.super);
            }
        case RX_64_RESPONSE:
            {
                Rx64Response* resp = (Rx64Response*) response;
                return &(resp->super.super.super);
            }
        case RX_16_IO_RESPONSE:
            {
                Rx16IoSampleResponse* resp = (Rx16IoSampleResponse*) response;
                return &(resp->super.super.super.super);
            }
        case RX_64_IO_RESPONSE:
            {
                Rx64IoSampleResponse* resp = (Rx64IoSampleResponse*) response;
                return &(resp->super.super.super.super);
            }
#endif
        case AT_COMMAND_RESPONSE:
            {
                AtCommandResponse* resp = (AtCommandResponse*) response;
                return &(resp->super.super);
            }
        case REMOTE_AT_COMMAND_RESPONSE:
            {
                RemoteAtCommandResponse* resp = (RemoteAtCommandResponse*) response;
                return &(resp->super.super.super);
            }
        case MODEM_STATUS_RESPONSE:
            {
                ModemStatusResponse* resp = (ModemStatusResponse*) response;
                return &(resp->super);
            }
        default:
            return NULL;
    }
}

/* Response Classes:
   case ZB_TX_STATUS_RESPONSE:
   {
   break;
   }
   case ZB_RX_RESPONSE:
   {
   break;
   }
   case ZB_IO_SAMPLE_RESPONSE:
   {
   break;
   }
   case TX_STATUS_RESPONSE:
   {
   break;
   }
   case RX_16_RESPONSE:
   {
   break;
   }
   case RX_64_RESPONSE:
   {
   break;
   }
   case RX_16_IO_RESPONSE:
   {
   break;
   }
   case RX_64_IO_RESPONSE:
   {
   break;
   }
   case AT_COMMAND_RESPONSE:
   {
   break;
   }
   case REMOTE_AT_COMMAND_RESPONSE:
   {
   break;
   }
   case MODEM_STATUS_RESPONSE:
   {
   break;
   }
   default:
   break;

*/

/* XBee read code End */

/**
 * XBee interface code
 * Begin
 */

XBee xbeeObj;
uint32_t addrTable[NUM_ADDR][2] = {{0x0013A200, 0x407B18F3},{0x0013A200, 0x407B18B5}};
uint8_t payload[NVC3_MESSAGE_SIZE+1];

bool addr_nvmcomm_to_xbee(address_t addr, uint32_t *msb, uint32_t *lsb)
{
    // Temporary: addresses <128 are ZWave, addresses >=128 are XBee
    if (addr<128)
      return false;
    addr -= 128;

    if (addr >= NUM_ADDR) return false;
    *msb = addrTable[addr][0];
    *lsb = addrTable[addr][1];
    return true;
}

bool addr_xbee_to_nvmcomm(address_t *addr, uint32_t msb, uint32_t lsb)
{
    int i;
    for (i = 0; i < NUM_ADDR; ++i) {
        if (addrTable[i][0] == msb && addrTable[i][0] == lsb){
            *addr = i + 128; // Temporary: addresses <128 are ZWave, addresses >=128 are XBee
            return true;
        }
    }
    return false;
}

void nvmcomm_xbee_receive(void)
{
    XBee_readPacket(&xbeeObj);
    DEBUGF_USART("RECV: XBee receive sth.\n");
    if(XBee_isAvailable(&xbeeObj)){
        DEBUGF_USART("RECV: XBee isAvailable\n");
        uint8_t id = XBee_getResponseApiId(&xbeeObj);
        XBeeResponse* response;
#ifdef SERIES_1
        if (id == RX_16_RESPONSE || id == RX_64_RESPONSE) {
            Rx16Response rx16;
            Rx64Response rx64;
            address_t src = 0;
            uint8_t option;
            // got a rx packet
            if (id == RX_16_RESPONSE) {
                XBee_getResponse(&xbeeObj, &rx16, RX_16_RESPONSE);
                response = getXBeeResponse(&rx16, RX_16_RESPONSE);
                option = response->_frameDataPtr[RX_16_RSSI_OFFSET+1];
                src = rx16._remoteAddress;
                DEBUGF_USART("RECV: 16 src "DBG8"\n", src);
            } else {
                XBee_getResponse(&xbeeObj, &rx64, RX_64_RESPONSE);
                response = getXBeeResponse(&rx64, RX_64_RESPONSE);
                option = response->_frameDataPtr[RX_64_RSSI_OFFSET+1];
                XBeeAddress64* addr64 = &(rx64._remoteAddress);
                addr_xbee_to_nvmcomm(&src, addr64->_msb, addr64->_lsb);
                DEBUGF_USART("RECV: 64 src "DBG8"\n", src);
            }

            // TODO check option, rssi bytes    
            int len = XBeeResponse_getDataLength(response), i;
            for (i = 0; i < len; ++i){
                uint8_t c = XBee_getDataByIndex(&xbeeObj, i);
                if (c != -1) payload[i] = c;
                else DEBUGF_USART("RECV: XBee get data failed\n");
            }
            if(f) f(src, payload[0], payload+1, len-1);
            return;
        }
#endif
#ifdef SERIES_2
        if(id == ZB_RX_RESPONSE){
            ZBRxResponse zbrxObj;
            XBee_getResponse(&xbeeObj, &zbrxObj, ZB_RX_RESPONSE);
            response = getXBeeResponse(&zbrxObj, ZB_RX_RESPONSE);
            uint8_t option = response->_frameDataPtr[4];
            if(option == ZB_PACKET_ACKNOWLEDGED){
                // packet acknowledged
                DEBUGF_USART("RECV: XBee ACKNOWLEDGED\n");
            }else{
                // packet not acknowledged
                DEBUGF_USART("RECV: XBee NOT ACKNOWLEDGED\n");
            }
            int len = XBeeResponse_getDataLength(response), i;
            for (i = 0; i < len; ++i){
                uint8_t c = XBee_getDataByIndex(&xbeeObj, i);
                if (c != -1) payload[i] = c;
                else DEBUGF_USART("RECV: XBee get data failed\n");
            }
            address_t dest;
            XBeeAddress64* addr64 = &(zbrxObj._remoteAddress64);
            if(addr_xbee_to_nvmcomm(&dest, addr64->_msb, addr64->_lsb) && !f) f(dest, payload[0], payload+1, len-1);
            return;
        } else if(id == MODEM_STATUS_RESPONSE) {
            //ModemStatusResponse msrObj;
            DEBUGF_USART("RECV: XBee get MODEM STATUS RESPONSE\n");
            return;
        }
#endif
    } else if (XBee_isError(&xbeeObj)) {
        DEBUGF_USART("RECV: error code: "DBG8"\n", XBee_getResponseErrorCode(&xbeeObj));
        return;
    } else {
        // not something we were expecting
        DEBUGF_USART("RECV: Not sth. expecting\n");
    }
}

void nvmcomm_xbee_init(void)
{
    XBee_init(&xbeeObj);
    f = NULL;
}

void nvmcomm_xbee_setcallback(void (*func)(address_t, u08_t, u08_t *, u08_t))
{
    f = func;
}

int nvmcomm_xbee_send(address_t dest, u08_t nvc3_command, u08_t *data, u08_t len, u08_t txoptions)
{
    uint8_t buf[NVC3_MESSAGE_SIZE+1];
    uint32_t h, l;
    int i;
    if (addr_nvmcomm_to_xbee(dest, &h, &l)) {
        DEBUGF_USART("SEND: address to "DBG32""DBG32"\n", h, l);
    } else {
        DEBUGF_USART("SEND: address init failed\n");
        return -1;
    }
    for (i = 0; i < len; ++i) buf[i+1] = data[i];
    buf[0] = nvc3_command;

#ifdef SERIES_1
    // send data thru XBee 802.15.4
    send64(buf,len+1,h,l);
    DEBUGF_USART("SEND: waiting for response after sending XBdata\n");

    // read response
    if(XBee_readPacket_timeout(&xbeeObj, 5000)){
        uint8_t id = XBee_getResponseApiId(&xbeeObj);
        if(id == TX_STATUS_RESPONSE){
            TxStatusResponse txStatus;
            XBee_getResponse(&xbeeObj, &txStatus, TX_STATUS_RESPONSE);
            XBeeResponse* response = getXBeeResponse(&txStatus, TX_STATUS_RESPONSE);

            uint8_t deliveryStatus = response->_frameDataPtr[1];
            if (deliveryStatus == SUCCESS){
                DEBUGF_USART("SEND: XBee delivery TXstatus is success\n");
                return 0;
            }
        } else {
            DEBUGF_USART("SEND: XBee getResponseTx failed\n");
            return -1;
        }
    }
#endif
#ifdef SERIES_2
    // send data thru ZigBee request
    sendZB(buf,len+1,h,l);
    DEBUGF_USART("SEND: waiting for response after sending ZBdata\n");

    // read response
    if(XBee_readPacket_timeout(&xbeeObj, 500)){
        uint8_t id = XBee_getResponseApiId(&xbeeObj);
        if(id == ZB_TX_STATUS_RESPONSE){
            ZBTxStatusResponse txStatus;
            XBee_getResponse(&xbeeObj, &txStatus, ZB_TX_STATUS_RESPONSE);
            XBeeResponse* response = getXBeeResponse(&txStatus, ZB_TX_STATUS_RESPONSE);

            uint8_t deliveryStatus = response->_frameDataPtr[4];
            if (deliveryStatus == SUCCESS){
                DEBUGF_USART("SEND: XBee delivery ZBTXstatus is success\n");
                return 0;
            }
        } else {
            DEBUGF_USART("SEND: XBee getResponseZB failed\n");
            return -1;
        }
    }
#endif
    DEBUGF_USART("SEND: XBee delivery status failed\n");
    return -1;
}

void nvmcomm_xbee_poll(void)
{
    if (uart_available(XBEE_UART))
        nvmcomm_xbee_receive();
}

/* XBee interface code End */

#endif
