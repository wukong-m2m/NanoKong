
#include "nvmcomm_zwave.h"
#include "config.h"
#include "uart.h"
#include "debug.h"

#ifdef NVM_USE_COMMZWAVE

#define ZWAVE_UART             1

#define ZWAVE_STATUS_SOF       1
#define ZWAVE_STATUS_LEN       2
#define ZWAVE_STATUS_TYPE      3
#define ZWAVE_STATUS_CMD       4
#define ZWAVE_STATUS_ID        5  
#define ZWAVE_STATUS_DATA      6
#define ZWAVE_STATUS_CRC       7
#define ZWAVE_STATUS_DONE      8

// u16_t g_seq = 0;
u08_t state;        // Current state
u08_t len;          // Length of the returned payload
u08_t type;         // 0: request 1: resposne 2: timeout
u08_t cmd;          // the serial api command number of the current payload
u08_t payload[64];  // The data of the current packet
u08_t payload_length;  // Length of the payload while reading a packet
// TODO: used?
u08_t last_node = 0;
u08_t seq;          // Sequence number which is used to match the callback function
// u32_t expire;  // The expire time of the last command
void (*f)(address_t src, u08_t *payload, u08_t length); // The callback function registered by callback
void (*f_nodeinfo)(u08_t *payload, u08_t length);

// Private
void display_nif(u08_t *payload, u08_t len) {
    DEBUGF_COMM("Status=%d Node=%d Device=%d:%d:%d\n", payload[0],payload[1],payload[3],payload[4],payload[5]);
    last_node = payload[1];
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
  state = ZWAVE_STATUS_SOF;
  f=NULL;
  f_nodeinfo=display_nif;
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
  //   state = ZWAVE_STATUS_SOF;
  //   if (f!=NULL) f(payload,i);
  //   Serial.write("timeout...\n");
  //   return true;
  // }
  while (uart_available(ZWAVE_UART)) {
// TODO    expire = now + 1000;
    u08_t c = uart_read_byte(ZWAVE_UART);
//    DEBUGF_COMM("c="DBG8" state="DBG8"\n\r", c, state);
    if (state == ZWAVE_STATUS_SOF) {
      // TODO: temporary until we figure out where the extra 'packets' from the Z-Wave module are coming from
      payload[4] = 0;
      payload[5] = 0;
      if (c == 1) {
        state = ZWAVE_STATUS_LEN;
        len = 0;
      } else if (c == 0x15) {
        // TODO: send: packet is incorrect
      } else if (c == 0x18) {
        // TODO: send: chip busy
      } else if (c == 0x06) {
        // TODO: send: correct
      }
    } else if (state == ZWAVE_STATUS_LEN) {
      len = c-3;
      state = ZWAVE_STATUS_TYPE;
    } else if (state == ZWAVE_STATUS_TYPE) {
      type = c;
      state = ZWAVE_STATUS_CMD;
    } else if (state == ZWAVE_STATUS_CMD) {
      cmd = c;
      state = ZWAVE_STATUS_DATA;
      payload_length = 0;
    } else if (state == ZWAVE_STATUS_DATA) {
      payload[payload_length++] = c;
      len--;
      if (len == 0) {
        state = ZWAVE_STATUS_CRC;
      }
    } else if (state == ZWAVE_STATUS_CRC) {
      uart_write_byte(ZWAVE_UART, 6);
      state = ZWAVE_STATUS_SOF;
      if (f!=NULL)
        f(payload[1], payload+4, payload_length-4); // Trim off first 4 bytes to get to the data. Byte 1 is the sending node.
      if (cmd == 0x49 && f_nodeinfo)
          f_nodeinfo(payload, payload_length);
    }
  }
}

// Send ZWave command to another node. This command can be used as wireless repeater between 
// two nodes. It has no assumption of the payload sent between them.
void nvmcomm_zwave_send(address_t dest, u08_t *b, u08_t l, u08_t option) {
  int k;
  u08_t crc;
// TODO: buffer size was 24, but aren't Z-Wave packets larger?
  u08_t buf[64];

#ifdef DEBUG
  DEBUGF_COMM("Sending message to "DBG8", length "DBG8": ", dest, l);
  for (size8_t i=0; i<l; ++i) {
    DEBUGF_COMM(" "DBG8"", b[i]);
  }
  DEBUGF_COMM("\n");
#endif

  buf[0] = 1;
  buf[1] = l+7;
  buf[2] = 0;
  buf[3] = 0x13;
  buf[4] = dest;
  buf[5] = l;
  for(k=0;k<l;k++)
    buf[k+6] = b[k];
  buf[l+6] = option;
  buf[l+7] = seq;

  crc = 0xff;
  uart_write_byte(ZWAVE_UART, buf[0]);
  for(k=0;k<l+7;k++) {
    uart_write_byte(ZWAVE_UART, buf[k+1]);
    crc = crc ^ buf[k+1];
  }
  seq++;
  uart_write_byte(ZWAVE_UART, crc);
// TODO  expire = millis()+1000;
}


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
