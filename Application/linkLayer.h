#ifndef LINKLAYER_H
#define LINKLAYER_H

#include <stdlib.h>
#include <stdio.h>
#include "types.h"


int createInformationFrame(char* data, size_t dataSize, char* frame);

int createSupervisionFrame(char* frame, char A, char C);

int bccChecker(char* frame, size_t size);

unsigned short checksum(unsigned short *buffer, int count);

int byteStuffing(char* data, int dataSize, char* stuff);

int byteDestuffing(char* stuff, int stuffSize, char* data);

#endif