#ifndef APPLAYER_H
#define APPLAYER_H

#include "types.h"

struct appicationLayer appLayer;

int create_file( char* filename);
int open_file(char* filename);
void close_file(int fd);

int teceiver();
int transmitter();

int read();
int write();

#endif
