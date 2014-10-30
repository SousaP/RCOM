#ifndef PACKETS_H
#define PACKETS_H

#include <stdlib.h>
#include <stdio.h>
#include "types.h"


int createControlStartPacket(char* packet, unsigned char* filename, int size);

int createControlEndPacket(char* packet, unsigned char* hash);

void createSupervisionFrame(char* frame, unsigned char A, unsigned char C);

#endif
