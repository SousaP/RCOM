#ifndef STUFFING
#define STUFFING

#include "types.h"
int stuffedSize(unsigned char* data, int size);

int stuffing(unsigned char* data, int size, unsigned char* stuffed);

int unstuffing(unsigned char* stuffed, int size, unsigned char* data);

#endif

