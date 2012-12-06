#include "nvmcomm_zwave.h"
#include "config.h"
#include "uart.h"
#include "debug.h"
#include "delay.h"
#include "error.h"
#include "nvmcomm.h"

#ifdef NVM_USE_COMMZWAVE

//#define ZWAVE_UART              1//for arduino
#define ZWAVE_UART              2//for wukong board

#define ZWAVE_STATUS_WAIT_ACK        0
#define ZWAVE_STATUS_WAIT_SOF        1
#define ZWAVE_STATUS_WAIT_LEN        2
#define ZWAVE_STATUS_WAIT_TYPE       3
#define ZWAVE_STATUS_WAIT_CMD        4
#define ZWAVE_STATUS_WAIT_ID         5
#define ZWAVE_STATUS_WAIT_DATA       6
#define ZWAVE_STATUS_WAIT_CRC        7
#define ZWAVE_STATUS_WAIT_DONE       8

#define ZWAVE_TYPE_REQ          0x00
#define ZWAVE_TYPE_CMD          0x01

#define ZWAVE_CMD_APPLICATIONCOMMANDHANDLER 0x04

#define COMMAND_CLASS_PROPRIETARY   0x88

#define ZWAVE_REQ_SENDDATA     0x13
#define FUNC_ID_MEMORY_GET_ID  0x20

#define ZWAVE_ACK              0x06

// u16_t g_seq = 0;
u08_t state;        // Current state
u08_t len;          // Length of the returned payload
u08_t type;         // 0: request 1: response 2: timeout
u08_t cmd;          // the serial api command number of the current payload
u08_t payload[NVMCOMM_MESSAGE_SIZE+5];  // The data of the current packet. 
// 4 bytes protocol overhead (see nvmcomm_zwave_receive),
// 1 byte for the nvc3_command, which is the first byte in the buffer.
u08_t payload_length;  // Length of the payload while reading a packet
// TODO: used?
u08_t last_node = 0;
u08_t seq;          // Sequence number which is used to match the callback function
u08_t ack_got = 0;
int zwsend_ack_got = 0;
u08_t wait_CAN_NAK = 1;
u08_t zwave_learn_block = 0;
u32_t zwave_time_learn_start;
u08_t zwave_mode=0;
bool zwave_btn_is_push=FALSE, zwave_btn_is_release=FALSE, zwave_learn_on=FALSE;
u32_t zwave_time_btn_interrupt,zwave_time_btn_push,zwave_time_btn_release;
// u32_t expire;  // The expire time of the last command

bool nvmcomm_zwave_my_address_loaded = FALSE;
address_t nvmcomm_zwave_my_address;

void (*f)(address_t src, u08_t nvc3_command, u08_t *payload, u08_t length); // The callback function registered by callback
void (*f_nodeinfo)(u08_t *payload, u08_t length);

int ZW_sendData(uint8_t id, uint8_t nvc3_command, u08_t *in, u08_t len, u08_t txoptions);
int SerialAPI_request(unsigned char *buf, int len);

bool addr_nvmcomm_to_zwave(address_t nvmcomm_addr, uint8_t *zwave_addr) {
    // Temporary: addresses <128 are ZWave, addresses >=128 are XBee
    if (nvmcomm_addr>=128)
        return false;
    *zwave_addr = nvmcomm_addr;
    return true;
}

bool addr_zwave_to_nvmcomm(address_t *nvmcomm_addr, uint8_t zwave_addr) {
    if (zwave_addr>=128)
        return false;
    *nvmcomm_addr = zwave_addr;
    return true;
}


// Blocking receive.
// Returns the Z-Wave cmd of the received message.
// Calls the callback for .... messages?
void nvmcomm_zwave_receive(int processmessages) {
    //DEBUGF_ZWAVETRACE("zwave receive!!!!!!!!!!!");
    while (!uart_available(ZWAVE_UART)) { }
    while (uart_available(ZWAVE_UART)) {
        // TODO    expire = now + 1000;
        u08_t c = uart_read_byte(ZWAVE_UART);
        DEBUGF_ZWAVETRACE("c="DBG8" state="DBG8"\n\r", c, state);
        if (state == ZWAVE_STATUS_WAIT_ACK) {
            if (c == ZWAVE_ACK) {
                state = ZWAVE_STATUS_WAIT_SOF;
                wait_CAN_NAK = 1;
                ack_got=1;
            } else if (c == 0x15) {
                // send: no ACK from other side
                if (wait_CAN_NAK != 128)
                    wait_CAN_NAK *= 2;
                DEBUGF_COMM("[NAK] SerialAPI LRC checksum error!!! delay: %dms\n", wait_CAN_NAK);
                delay(MILLISEC(wait_CAN_NAK));
                state = ZWAVE_STATUS_WAIT_SOF;
                ack_got=0;
            } else if (c == 0x18) {
                // send: chip busy
                if (wait_CAN_NAK != 128)
                    wait_CAN_NAK *= 2;
                DEBUGF_COMM("[CAN] SerialAPI frame is dropped by ZW!!! delay: %dms\n", wait_CAN_NAK);
                delay(MILLISEC(wait_CAN_NAK));
                state = ZWAVE_STATUS_WAIT_SOF;
                ack_got=0;
            } else if (c == 1) {
                state = ZWAVE_STATUS_WAIT_LEN;
                len = 0;  
            } else {
                DEBUGF_COMM("Unexpected byte while waiting for ACK %x\n", c);
            }
        } else if (state == ZWAVE_STATUS_WAIT_SOF) {
            if (c == 0x01) {
                state = ZWAVE_STATUS_WAIT_LEN;
                len = 0;
            } else if (c == ZWAVE_ACK) {
                DEBUGF_COMM("ZWAVE_STATUS_WAIT_SOF: SerialAPI got unknown ACK ????????\n");
                ack_got = 1;
            }
        } else if (state == ZWAVE_STATUS_WAIT_LEN) {
            len = c-3; // 3 bytes for TYPE, CMD, and CRC
            state = ZWAVE_STATUS_WAIT_TYPE;
        } else if (state == ZWAVE_STATUS_WAIT_TYPE) {
            type = c; // 0: request 1: response 2: timeout
            state = ZWAVE_STATUS_WAIT_CMD;
        } else if (state == ZWAVE_STATUS_WAIT_CMD) {
            cmd = c;
            state = ZWAVE_STATUS_WAIT_DATA;
            payload_length = 0;
        } else if (state == ZWAVE_STATUS_WAIT_DATA) {
            payload[payload_length++] = c;
            len--;
            if (len == 0) {
                state = ZWAVE_STATUS_WAIT_CRC;
            }
        } else if (state == ZWAVE_STATUS_WAIT_CRC) {
            uart_write_byte(ZWAVE_UART, 6);
            state = ZWAVE_STATUS_WAIT_SOF;
            if (type == ZWAVE_TYPE_REQ && cmd == 0x13) {
                zwsend_ack_got = payload[1];
            }
            if (type == ZWAVE_TYPE_REQ && cmd == ZWAVE_CMD_APPLICATIONCOMMANDHANDLER)
                if (f!=NULL) {
                    address_t nvmcomm_addr;
                    if (addr_zwave_to_nvmcomm(&nvmcomm_addr, payload[1]) && processmessages==1)
                        f(nvmcomm_addr, payload[4], payload+5, payload_length-5); // Trim off first 5 bytes to get to the data. Byte 1 is the sending node, byte 4 is the command
                }
            if (cmd == FUNC_ID_MEMORY_GET_ID) {
                nvmcomm_zwave_my_address = payload[4];
                nvmcomm_zwave_my_address_loaded = TRUE;
            }
            if (cmd == 0x49 && f_nodeinfo)
                f_nodeinfo(payload, payload_length);
            if (cmd == 0x50) {
                if(payload[1]==0x01) {
                    zwave_learn_block = 1;
                    //	   DEBUGF_COMM("zwave payload block !!!!!!!!!!!!!!!!");
                }
                else if(payload[1]==6) {//network stop, learn off
                    unsigned char b[10];
                    unsigned char onoff=0;
                    int k;
                    b[0] = 1;
                    b[1] = 5;
                    b[2] = 0;
                    b[3] = 0x50;
                    b[4] = onoff;//off
                    b[5] = seq;
                    b[6] = 0xff^5^0^0x50^onoff^seq;
                    seq++;
                    //DEBUGF_COMM("zwave payload learnoff !!!!!!!!!!!!!!!!");
                    for(k=0;k<7;k++)
                    {
                        //Serial1.write(b[k]);
                        uart_write_byte(ZWAVE_UART, b[k]);
                    }
                    zwave_learn_on=FALSE;
                    zwave_learn_block=0;
                    zwave_mode=0;
                }
            }
        }
    }
}



// Public interface
void nvmcomm_zwave_init() {
    // TODO: why is this here?
    // for(i=0;i<100;i++)
    //   mainloop();
    // TODO: analog read
    // randomSeed(analogRead(0));
    // seq = random(255);
    seq = 42; // temporarily init to fixed value
    state = ZWAVE_STATUS_WAIT_SOF;
    f=NULL;
    f_nodeinfo=NULL;
    uart_init(ZWAVE_UART, ZWAVE_UART_BAUDRATE);
    // TODO
    // expire = 0;

    unsigned char buf[] = {ZWAVE_TYPE_REQ, FUNC_ID_MEMORY_GET_ID};
    nvmcomm_poll();
    uint8_t retries = 10;
    address_t previous_received_address = 0;

    //DEBUGF_ZWAVETRACE("test msg");
    while(!nvmcomm_zwave_my_address_loaded) {
        while(!nvmcomm_zwave_my_address_loaded && retries-->0) {
            SerialAPI_request(buf, 2);
            nvmcomm_poll();
        }
        if(!nvmcomm_zwave_my_address_loaded) // Can't read address -> panic
            error(ERROR_COMM_INIT_FAILED);
        if (nvmcomm_zwave_my_address != previous_received_address) { // Sometimes I get the wrong address. Only accept if we get the same address twice in a row. No idea if this helps though, since I don't know what's going on exactly.
            nvmcomm_zwave_my_address_loaded = false;
            previous_received_address = nvmcomm_zwave_my_address;
        }
    }
    DEBUGF_COMM("My Zwave node_id: %x\n", nvmcomm_zwave_my_address);
}

void nvmcomm_zwave_setcallback(void (*func)(address_t, u08_t, u08_t *, u08_t)) {
    f = func;
}

void nvmcomm_zwave_poll(void) {
    // TODO
    // unsigned long now = millis();
    // 
    // if (expire && (now > expire)) {
    //   expire = 0;
    //   type = 2;
    //   state = ZWAVE_STATUS_WAIT_SOF;
    //   if (f!=NULL) f(payload,i);
    //   Serial.write("timeout...\n");
    //   return true;
    // }
    if (uart_available(ZWAVE_UART))
    {    nvmcomm_zwave_receive(1);
        //DEBUGF_ZWAVETRACE("zwave poll!!!!!!!!!!!");
    }
}

// Send ZWave command to another node. This command can be used as wireless repeater between 
// two nodes. It has no assumption of the payload sent between them.
int nvmcomm_zwave_send(address_t dest, u08_t nvc3_command, u08_t *data, u08_t len, u08_t txoptions) {
#ifdef DEBUG
    DEBUGF_COMM("Sending command "DBG8" to "DBG8", length "DBG8": ", nvc3_command, dest, len);
    for (size8_t i=0; i<len; ++i) {
        DEBUGF_COMM(" "DBG8"", data[i]);
    }
    DEBUGF_COMM("\n");
#endif
    u08_t zwave_addr;
    if (addr_nvmcomm_to_zwave(dest, &zwave_addr))
        return ZW_sendData(zwave_addr, nvc3_command, data, len, txoptions);
    else
        return -1; // Not a ZWave address
    // TODO  expire = millis()+1000;
}

// Get the ID of this node
address_t nvmcomm_zwave_get_node_id() {
    return nvmcomm_zwave_my_address;
}

void nvmcomm_zwave_reset() {
    unsigned char b[10];
    int k;

    b[0] = 1;
    b[1] = 4;
    b[2] = 0;
    b[3] = 0x42;
    b[4] = seq;
    b[5] = 0xff^4^0^0x42^seq;
    seq++;
    for(k=0;k<7;k++)
    {
        uart_write_byte(ZWAVE_UART, b[k]);
    }
    zwave_mode=0;
    //DEBUGF_COMM("reset complete!!!!!!!!!!");

}

void nvmcomm_zwave_learn() {
    unsigned char b[10];
    unsigned char onoff=1;
    int k;    
    if(zwave_learn_on==FALSE)
    {
        zwave_time_learn_start=avr_currentTime;
        zwave_learn_on=TRUE;
        b[0] = 1;
        b[1] = 5;
        b[2] = 0;
        b[3] = 0x50;
        b[4] = onoff;
        b[5] = seq;
        b[6] = 0xff^5^0^0x50^onoff^seq;
        seq++;
        //DEBUGF_COMM("zwave learn !!!!!!!!!!!!!!!!");
        for(k=0;k<7;k++)
        {
            uart_write_byte(ZWAVE_UART, b[k]);
        }
    }
    //DEBUGF_COMM("current:"DBG32" start:"DBG32", zwave_learn_block:"DBG8": ", avr_currentTime, zwave_time_learn_start, zwave_learn_block);
    if(avr_currentTime-zwave_time_learn_start>5000 && !zwave_learn_block) { //time out learn off
        //DEBUGF_COMM("turn off!!!!!!!!!!!!!!!!");
        onoff=0;
        b[0] = 1;
        b[1] = 5;
        b[2] = 0;
        b[3] = 0x50;
        b[4] = onoff;//off
        b[5] = seq;
        b[6] = 0xff^5^0^0x50^onoff^seq;
        seq++;	   
        for(k=0;k<7;k++)
        {
            uart_write_byte(ZWAVE_UART, b[k]);
        }  
        zwave_learn_on=FALSE;
        zwave_learn_block=0;
        zwave_mode=0;
    }
}





//===================================================================================================================
// Copied & modified from testrtt.c
//===================================================================================================================
int SerialAPI_request(unsigned char *buf, int len)
{
    unsigned char c = 1;
    int i;
    unsigned char crc;
    int retry = 5;

    while (1) {
        // read out pending request from Z-Wave
        if (uart_available(ZWAVE_UART))
            nvmcomm_zwave_receive(0); // Don't process received messages
        if (state != ZWAVE_STATUS_WAIT_SOF) {	// wait for WAIT_SOF state (idle state)
            DEBUGF_COMM("SerialAPI is not in ready state!!!!!!!!!! zstate="DBG8"\n", state);
            DEBUGF_COMM("Try to send SerialAPI command in a wrong state......\n");
            delay(MILLISEC(100));
            //continue;
        }

        // send SerialAPI request
        c=1;
        uart_write_byte(ZWAVE_UART, c); // SOF (start of frame)
        c = len+1;
        uart_write_byte(ZWAVE_UART, c); // len (length of frame)
        crc = 0xff;
        crc = crc ^ (len+1);
        for(i=0;i<len;i++) {
            crc = crc ^ buf[i];
            uart_write_byte(ZWAVE_UART, buf[i]); // REQ, cmd, data
        }
        uart_write_byte(ZWAVE_UART, crc); // LRC checksum
#ifdef DEBUG
        DEBUGF_COMM("Send len="DBG8" ", len+1);
        for(i=0;i<len;i++) {
            DEBUGF_COMM(""DBG8" ", buf[i]);
        }
        DEBUGF_COMM("CRC="DBG8"\n", crc);
        ;
#endif
        state = ZWAVE_STATUS_WAIT_ACK;
        ack_got = 0;

        // get SerialAPI ack
        i = 0;
        while(!uart_available(ZWAVE_UART) && i++<100)
            delay(MILLISEC(1));
        if (uart_available(ZWAVE_UART)) {
            nvmcomm_zwave_poll();			
            if (ack_got == 1) {
                return 0;
            } else {
                DEBUGF_COMM("Ack error!!! zstate="DBG8" ack_got="DBG8"\n", state, ack_got);
            }
        } 
        if (state == ZWAVE_STATUS_WAIT_ACK) {
            state = ZWAVE_STATUS_WAIT_SOF; // Give up and don't get stuck in the WAIT_ACK state
            DEBUGF_COMM("Back to WAIT_SOF state.\n");
        }
        if (!retry--) {
            DEBUGF_COMM("SerialAPI request:\n");
            for (i=0; i<len; i++) {
                DEBUGF_COMM(""DBG8" ", buf[i]);
            }
            DEBUGF_COMM("\n");
            DEBUGF_COMM("error!!!\n", __FUNCTION__);
            return -1;
        }
        DEBUGF_COMM("SerialAPI_request retry (%d)......\n", retry);
    }
    return -1; // Never happens
}

int ZW_sendData(uint8_t id, uint8_t nvc3_command, u08_t *in, u08_t len, u08_t txoptions)
{
    unsigned char buf[NVMCOMM_MESSAGE_SIZE+8];
    int i;
    int timeout = 1000;
    zwsend_ack_got = -1;

    buf[0] = ZWAVE_TYPE_REQ;
    buf[1] = ZWAVE_REQ_SENDDATA;
    buf[2] = id;
    buf[3] = len+2;
    buf[4] = COMMAND_CLASS_PROPRIETARY;
    buf[5] = nvc3_command; // See nvmcomm.h
    for(i=0; i<len; i++)
        buf[i+6] = in[i];
    buf[6+len] = txoptions;
    buf[7+len] = seq++;
    if (SerialAPI_request(buf, len + 8) != 0)
        return -1;
    while (zwsend_ack_got == -1 && timeout-->0) {
        nvmcomm_poll();
        delay(MILLISEC(1));
    }
    if (zwsend_ack_got == 0) // ACK 0 indicates success
        return 0;
    else {
        DEBUGF_COMM("========================================ZW_sendDATA ack got: %x\n", zwsend_ack_got);
        return -1;    
    }
}
//===================================================================================================================
// End: copied & modified from testrtt.c
//===================================================================================================================


// 
// public:
//   int getType() {
//     return type;
//   }
//   void DisplayNodeInfo() {
//     char buf[128];
//     
//     snprintf(buf,64,"Status=%d Node=%d Device=%d:%d:%d\n", payload[0],payload[1],payload[3],payload[4],payload[5]);
//     Serial.write(buf);
//   }
//   
// 
// 
//   
//   // Include or exclude node from the network
//   void networkIncludeExclude(byte t,byte m) {
//     byte b[10];
//     int k;
//   
//     b[0] = 1;
//     b[1] = 5;
//     b[2] = 0;
//     b[3] = t;
//     b[4] = m;
//     b[5] = seq;
//     b[6] = 0xff^5^0^t^m^seq;
//     seq++;
//     for(k=0;k<7;k++)
//       Serial1.write(b[k]);
//   }
//   
//   // Reset the ZWave module to the factory default value. This must be called carefully since it will make
//   // the network unusable.
//   void reset() {
//     byte b[10];
//     int k;
//   
//     b[0] = 1;
//     b[1] = 4;
//     b[2] = 0;
//     b[3] = 0x42;
//     b[4] = seq;
//     b[5] = 0xff^4^0^0x42^seq;
//     seq++;
//     for(k=0;k<6;k++)
//       Serial1.write(b[k]);
//         
//   }
//   // Start inclusion procedure to add a new node
//   void includeAny() {networkIncludeExclude(0x4A,1);}
// 
//   // Stop inclusion/exclusion procedure
//   void LearnStop() {networkIncludeExclude(0x4A,5);}
// 
//   // Start exclusion procedure
//   void excludeAny() {networkIncludeExclude(0x4B,1);}
// 
//   // Set the value of a node
//   void set(byte id,byte v,byte option) {
//     byte b[3];
//     
//     b[0] = 0x20;
//     b[1] = 1;
//     b[2] = v;
//     send(id,b,3,option);
//   }
// };
// 
// ZWaveClass ZWave;
// 
// void offack(byte *b,int len)
// {
//   if (ZWave.getType() == 0) {
//     delay(500);
//     ZWave.callback(onack);
//     ZWave.set(last_node,255,5);
//   } else if (ZWave.getType() == 2) {
//       Serial.write("Timeout\n");
//       ZWave.callback(onack);
//       ZWave.set(last_node,255,5);
//   } 
// }
// 
// void onack(byte *b,int len)
// {
//   if (ZWave.getType() == 0) {
//     delay(500);
//     ZWave.callback(offack);
//     ZWave.set(last_node,0,5);
//   } else if (ZWave.getType() == 2) {
//       Serial.write("Timeout\n");
//       ZWave.callback(offack);
//       ZWave.set(last_node,0,5);
//   }
// }
// 
// void include_cb(byte *b,int len)
// {
//   char buf[64];
//   
//   snprintf(buf,64,"Status=%d Node=%d\n", b[0],b[1]);
// }
// 
// void exclude_cb(byte *b,int len)
// {
//   char buf[64];
//   
//   snprintf(buf,64,"Status=%d Node=%d\n", b[0],b[1]);
// }
// 
// void help()
// {
//   Serial.write("a: Include a new node\n");
//   Serial.write("d: Exclude a node\n");
//   Serial.write("s: stop inclusion/exclusion\n");
//   Serial.write("t: test the current node\n");
//   Serial.write("Press the program button of the node to change the current node\n");
// }
//   
// void loop()
// {
//   if (ZWave.mainloop()) {
//     
//   }
//   if (Serial.available()) {
//     byte c = Serial.read();
//     if (c == 'a') {
//       ZWave.callback(include_cb);
//       ZWave.includeAny();
//     } else if (c == 'd') {
//       ZWave.callback(exclude_cb);
//       ZWave.excludeAny();
//     } else if (c == 's') {
//       ZWave.callback(0);
//       ZWave.LearnStop();
//     } else if (c == 't') {
//       ZWave.callback(offack);
//       ZWave.set(last_node,0,5);
//     } else {
//       help();
//     }
//   }
// }
//   





#endif
