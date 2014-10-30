#ifndef APPLAYER
#define APPLAYER

#include "types.h"

struct applicationLayer appLayer;

void transmitter();

void receiver();

void appWrite();

int createDataFrame(unsigned char* aux, unsigned char* data, int framesize); //retorna o novo tamanho

int createStart(unsigned char * buffer);

int createEnd(unsigned char* aux, unsigned char* hash);

void representloadingbar(int inicio, int size);

#endif

