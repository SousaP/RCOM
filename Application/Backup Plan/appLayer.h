#ifndef APPLAYER
#define APPLAYER

#include "types.h"

struct applicationLayer appData;

void transmitter();

void receiver();

void receiveFile();

void sendFile();

int createDataFrame(char* aux, char* data, int framesize); //retorna o novo tamanho

int createStart(char * buffer);

void representloadingbar(int inicio, int size);

#endif

