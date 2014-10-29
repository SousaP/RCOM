#ifndef LINKLAYER
#define LINKLAYER

#include "types.h"

struct termios oldtio, newtio;

struct linkLayer linkData;

int llopen(int type); //type = RECEIVER | TRANSMITTER

int llwrite(unsigned char * buffer, int length);

int llread(unsigned char * buffer, int length);

int llclose();

void dfaReceive(unsigned char* frame, int frameSize);

void resendFrame_alarm(int signo);

void sendREJ(int sn);

int waitResponse();


#endif

