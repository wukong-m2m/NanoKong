
#include "nvmcomm_zwave.h"
#include "config.h"
#include "uart.h"
#include "debug.h"
#include "delay.h"
#include "nvmcomm3.h"

#ifdef NVM_USE_COMMZWAVE

#define ZWAVE_UART              1

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

#define ZWAVE_ACK              0x06

// u16_t g_seq = 0;
u08_t state;        // Current state
u08_t len;          // Length of the returned payload
u08_t type;         // 0: request 1: resposne 2: timeout
u08_t cmd;          // the serial api command number of the current payload
u08_t payload[NVC3_MESSAGE_SIZE+4];  // The data of the current packet. 4 bytes protocol overhead (see nvmcomm_zwave_receive).
u08_t payload_length;  // Length of the payload while reading a packet
// TODO: used?
u08_t last_node = 0;
u08_t seq;          // Sequence number which is used to match the callback function
u08_t ack_got = 0;
// u32_t expire;  // The expire time of the last command
void (*f)(address_t src, u08_t *payload, u08_t length); // The callback function registered by callback
void (*f_nodeinfo)(u08_t *payload, u08_t length);


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
    nvmcomm_zwave_poll();
		if (state != ZWAVE_STATUS_WAIT_SOF) {	// wait for WAIT_SOF statie (idle state)
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
		for(i=0;i<len;i++) 
			DEBUGF_COMM(""DBG8" ", buf[i]);
			DEBUGF_COMM("CRC="DBG8"\n", crc);
#endif
		state = ZWAVE_STATUS_WAIT_ACK;
		ack_got = 0;

		// get SerialAPI ack
		while(1) {
      i = 0;
      while(!uart_available(ZWAVE_UART) && i++<1000)
        delay(MILLISEC(1));
      if (uart_available(ZWAVE_UART)) {
        nvmcomm_zwave_poll();			
  			if (ack_got == 1) {
  				return 0;
  			} else {
  				DEBUGF_COMM("Ack error!!! zstate="DBG8" ack_got="DBG8"\n", state, ack_got);
  			}
		  } else {
				DEBUGF_COMM("No ack!!! zstate="DBG8" ack_got="DBG8"\n", state, ack_got);
		  }
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
		DEBUGF_COMM("SerialAPI_request retry......\n");
	}
	return -1; // Never happens
}

int ZW_sendData(uint8_t id, u08_t *in, u08_t len, u08_t txoptions)
{
	unsigned char buf[NVC3_MESSAGE_SIZE+7];
  int i;
  
	buf[0] = ZWAVE_TYPE_REQ;
	buf[1] = ZWAVE_REQ_SENDDATA;
	buf[2] = id;
	buf[3] = len+1;
  buf[4] = COMMAND_CLASS_PROPRIETARY;
  for(i=0; i<len; i++)
    buf[i+5] = in[i];
	buf[5+len] = txoptions;
  buf[6+len] = seq++;
	return SerialAPI_request(buf, len + 7);
}
//===================================================================================================================
// End: copied & modified from testrtt.c
//===================================================================================================================




// Blocking receive.
// Returns the Z-Wave cmd of the received message.
// Calls the callback for .... messages?
void nvmcomm_zwave_receive(void) {
  while (!uart_available(ZWAVE_UART)) { }
  while (uart_available(ZWAVE_UART)) {
// TODO    expire = now + 1000;
    u08_t c = uart_read_byte(ZWAVE_UART);
    DEBUGF_COMM("c="DBG8" state="DBG8"\n\r", c, state);
    if (state == ZWAVE_STATUS_WAIT_ACK) {
      if (c == ZWAVE_ACK) {
  			state = ZWAVE_STATUS_WAIT_SOF;
  			ack_got=1;
  		} else if (c == 0x15) {
          // TODO: send: packet is incorrect
      } else if (c == 0x18) {
          // TODO: send: chip busy
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
      type = c;
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
      if (type == ZWAVE_TYPE_REQ && cmd == ZWAVE_CMD_APPLICATIONCOMMANDHANDLER)
      if (f!=NULL)
        f(payload[1], payload+4, payload_length-4); // Trim off first 4 bytes to get to the data. Byte 1 is the sending node.
      if (cmd == 0x49 && f_nodeinfo)
          f_nodeinfo(payload, payload_length);
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
}

void nvmcomm_zwave_setcallback(void (*func)(address_t, u08_t *, u08_t)) {
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
    nvmcomm_zwave_receive();
}

// Send ZWave command to another node. This command can be used as wireless repeater between 
// two nodes. It has no assumption of the payload sent between them.
int nvmcomm_zwave_send(address_t dest, u08_t *data, u08_t len, u08_t txoptions) {
#ifdef DEBUG
  DEBUGF_COMM("Sending message to "DBG8", length "DBG8": ", dest, len);
  for (size8_t i=0; i<len; ++i) {
    DEBUGF_COMM(" "DBG8"", data[i]);
  }
  DEBUGF_COMM("\n");
#endif

  return ZW_sendData(dest, data, len, txoptions);
// TODO  expire = millis()+1000;
}









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
