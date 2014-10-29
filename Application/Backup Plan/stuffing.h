#ifndef STUFFING
#define STUFFING

#include "types.h"
int stuffedSize(char* data, int size);

int stuffing(char* data, int size, char* stuffed);

int unstuffing(char* stuffed, int size, char* data);

#endif

