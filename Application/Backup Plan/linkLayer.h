#ifndef LINKLAYER
#define LINKLAYER

#include "types.h"

struct termios oldtio, newtio;

struct linkLayer lLayer;

int llopen(int type); //type = RECEIVER | TRANSMITTER

int llwrite(unsigned char * buffer, int length);

int llread(unsigned char * buffer, int length);

int llclose();

void validator(unsigned char* frame, int frameSize);

void resendFrameAlrm(int signo);

void sendREJsignal(int sig);

int waitForSignal();


#endif
