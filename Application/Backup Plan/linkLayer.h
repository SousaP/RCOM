#ifndef LINKLAYER
#define LINKLAYER

#include "types.h"

struct termios oldtio, newtio;

struct linkLayer lLayer;

void resendFrameAlrm(int signo);

int llopen(int type); //type = RECEIVER | TRANSMITTER

int llclose();

int llwrite(unsigned char * buffer, int length);

int llread(unsigned char * buffer, int length);

int lldisc();

void validator(unsigned char* frame, int frameSize);

int waitForSignal();

void sendREJsignal(int sig);

void createSupervisionFrame(char* frame, unsigned char A, unsigned char C);

#endif
