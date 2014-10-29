#ifndef TYPES
#define TYPES

#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <math.h>

// GENERIC

#define _POSIX_SOURCE 1 /* POSIX compliant source */
#define FALSE 0
#define TRUE 1

// APPLICATION

#define RECEIVER 0
#define TRANSMITTER 1


// APPLICATION FRAME CONTROL (AFC)


#define P_CONTROL_DATA  1
#define P_CONTROL_START 2
#define P_CONTROL_END   3
#define P_N       255
#define P_T_SIZE  0
#define P_T_NAME  1
#define P_T_SHA1   2

// LINK

#define MAX_FRAME_SIZE 512 // Frame max size
#define STUFF_MAX_SIZE MAX_FRAME_SIZE*2 // Stuffed frame max size

// STUFFING

#define ESCAPE      0x7d
#define FLAG_AUX    0x5e
#define ESCAPE_AUX  0x5d

// LINK FRAME CONTROL (FRAME)
// flag | A | C | A^C | [ AF | XOR(AF)] | flag
// C = Tipo
// AFC = Application Frame = Dados

#define FLAG      0x7e

#define FRAME_A_T       0x03 // Comandos enviados pelo Emissor; Respostas enviadas pelo Receptor
#define FRAME_A_R       0x01 // Comandos enviados pelo Receptor; Respostas enviadas pelo Emissor

#define FRAME_C_I0      0x00
#define FRAME_C_I1      0x10
#define FRAME_C_SET     0x03
#define FRAME_C_DISC    0x0B
#define FRAME_C_UA      0x07
#define FRAME_C_RR0     0x05
#define FRAME_C_RR1     0x85
#define FRAME_C_REJ0    0x01
#define FRAME_C_REJ1    0x81

struct applicationLayer {
    int fileDescriptor; /*Descritor correspondente à porta série*/
    int status; /*TRANSMITTER | RECEIVER*/
    int dataSize;
    char filename[257];
    unsigned int sequenceNumber;
};

struct linkLayer {
    char port[20]; /*Dispositivo /dev/ttySx, x = 0, 1*/
  int fileDescriptor; /*Descritor correspondente à porta série*/
  int baudRate; /*Velocidade de transmissão*/

  unsigned int sequenceNumber; /*Número de sequência da trama: 0, 1*/
  unsigned int timeout; /*Valor do temporizador: 1 s*/
  unsigned int numTransmissions; /*Número de tentativas em caso de falha*/
    unsigned int numFailedTransmissions; /*Número de tentativas falhadas*/
  char frame[STUFF_MAX_SIZE]; /*Trama*/
    int frameSize;
};

#endif