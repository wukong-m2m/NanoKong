#ifndef TESTRTT_H
#define TESTRTT_H

extern char PyZwave_messagebuffer[1024];
extern int PyZwave_src;
int PyWave_send(unsigned id,unsigned char *in,int len);
int PyZwave_init(char *host);
int PyZwave_receive(int);

#endif