#ifndef APPLAYER_H
#define APPLAYER_H

#include "appTypes.h"


int fileDescriptor; /*Descritor correspondente à porta série*/
int status; /*Mode da AppLayer*/
struct stat st; /*Struct do ficheiro*/

int main(int argc, char* argv[]);

int create_file( char* filename);
int open_file(char* filename);
void close_file(int fd);


int Receiver();
int Transmissor();


int llopen (int port, char *dev);
int llclose (int port);

#endif
