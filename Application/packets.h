#ifndef PACKETS_H
#define PACKETS_H

#include <stdlib.h>
#include <stdio.h>
#include "types.h"


int createDataPacket(char* packet, int seqNumber, int length, char* data);

int createControlPacket(char* packet, char control, char T1, char L1, char* V1, char T2, char L2, char* V2);

int processControlPacket(char* packet, int size, char control, char T1, char L1, char* V1, char T2, char L2, char* V2);

int processDataPacket(char* packet, int seqNumber, int length, char* data);

#endif
