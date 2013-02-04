#ifndef TESTRTT_H
#define TESTRTT_H

extern char PyZwave_messagebuffer[1024];
extern int PyZwave_src;
extern int PyZwave_print_debug_info;
int PyWave_send(unsigned id,unsigned char *in,int len);
int PyZwave_init_usb(char *dev_name);
int PyZwave_init(char *host);
int PyZwave_receive(int);
extern char *PyZwave_status();
extern void PyZwave_clearstatus();
extern int PyZwave_zwavefd();
extern void PyZwave_routing(unsigned node_id);

#endif
