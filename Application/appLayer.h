#ifndef APPLAYER_H
#define APPLAYER_H

#include "types.h"

struct stat st;
struct applicationLayer appLayer;

int createFile( char* filename);
int openFile(char* filename);

int receiver();
int transmitter();

int appWrite();

#endif
