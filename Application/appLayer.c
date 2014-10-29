#include "appLayer.h"
#include "packets.h"
#include <openssl/sha.h>

int numSeq;
int size;

int createFile(char* filename){
  //retorna o fd do ficheiro que cria
  printf("\nCreating file %s", filename);
  return open(filename, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0744);
}

int openFile(char* filename){
//retorna o Fd do ficheiro que tenta abrir
  int file_fd = open(filename, O_RDONLY);

  stat(filename, &st);
  if(st.st_size == 0)
      return -1;

  return file_fd;
}

int receiver(){

  int sizeR = 0;
  int fileW = createFile(appLayer.filename);
  appLayer.fileDescriptor = llopen(RECEIVER);
  printf("\nllopen Done");
  appLayer.sequenceNumber = -1;

  int receivedFrames = 0;
  int badFrames = 0;
  int n = 0;
  char buffer[MAX_FRAME_SIZE - 6];
  char writeBUF[MAX_FRAME_SIZE - 6];
int count = 0;
  while(1) {

    int bufferSize = llread(buffer, MAX_FRAME_SIZE - 6);
    int j;
    for(j = 0; j < bufferSize; j++) {
      printf("%d ", buffer[j]);
    }
    if(bufferSize == -1) {
      break;
    }

    else if(bufferSize < -1) {
      badFrames++;
    }
    else {
      if(buffer[0] == 0) {
        receivedFrames++;

      }
      printf("\n%d received frames",receivedFrames);
    }

    if(buffer[0] == P_CONTROL_START) {
      printf("Transmition started.......\n");

      int sizeT = (int) buffer[2];

      char sizeC[300];
      memcpy(&sizeC[0], &buffer[3], sizeT);
      size = atoi(&sizeC[0]); // estava (&sizechar[0]); pos oque esta agora
    }
    else if(buffer[0] == P_CONTROL_END) {
      printf("Transmition ended!\n");
      if(buffer[1] == P_T_SHA1) {
        int fileR = openFile(appLayer.filename);
        char *data = (char*) malloc(sizeR + 5);

       if(read(fileR, data, sizeR) != -1) {

          unsigned char hash[SHA_DIGEST_LENGTH];

          SHA1(data, sizeR, hash);

          int i;
          for(i = 0; i <= (int) buffer[2]; i++) {
            if(i == buffer[2]) {
              printf("SHA1 Checksum OK!\n");
              break;
            }
            if(hash[i] != buffer[i+3]) {
              printf("SHA1 Checksum Error!\n");
              break;
            }
          }
          continue;
        }
      }
    }
    else {
      printf("blablabla");
      if(bufferSize > 4) {
        if((appLayer.sequenceNumber + 1)%128 != buffer[1]) {
          printf("Error in sequence number\n");
          continue;
        }
        appLayer.sequenceNumber++;

        int i;
        for(i = 4; i < bufferSize; i++) {
          writeBUF[i-4] = buffer[i];
        }

        write(fileW, writeBUF, bufferSize - 4); // estava fileR que nao estava definido

        sizeR += bufferSize - 4;
        n++;
      }
    }
  }
  printf("%d frames received. %d failed frames\n", receivedFrames, badFrames);
  close(fileW);
}

int transmitter() {
  numSeq = 0;
  appLayer.fileDescriptor = llopen(TRANSMITTER);
  printf("\nllopen Done");

  appWrite();

  printf("\nappWrite Done");

disconnect();

  return 0;
}

int appWrite() {
  printf("\nStarting appWrite");
  stat(appLayer.filename, &st);
  size = st.st_size;

  char bufferS[MAX_FRAME_SIZE-6];
  int sizeP = createControlStartPacket(bufferS, appLayer.filename, size);
  llwrite(bufferS, sizeP);

  char* dataP = (char*) malloc(size + 5);
  int fileW = openFile(appLayer.filename);

  if(read(fileW, dataP, size) == -1) {
    printf("Error opening file");
    llclose();
    return -1;
  }

  unsigned char hash[SHA_DIGEST_LENGTH];
  SHA1(dataP, size, hash); // estava so data

  char data[MAX_FRAME_SIZE-6];
  char packetAux[MAX_FRAME_SIZE-6];
  int framesSent = 0;
  int i;
  for(i = 0; i < (int) size/appLayer.dataSize; i++) {
    memcpy(&data[0], &dataP[i*appLayer.dataSize], appLayer.dataSize);
    int frameSize = createDataPacket(packetAux, numSeq, appLayer.dataSize, data); // estava sequenceNumber
    numSeq++;
    llwrite(packetAux, frameSize);
    framesSent++;
  }

  if(i * appLayer.dataSize < size) {
    memcpy(&data[0], &dataP[i*appLayer.dataSize], size - i *appLayer.dataSize);
    int frameSize = createDataPacket(packetAux, numSeq, size - i * appLayer.dataSize, data);// estava sequenceNumber
    numSeq++;
    llwrite(packetAux, frameSize);
    framesSent++;
  }

  printf("%d packets sent!\n", framesSent);

  char bufferE[MAX_FRAME_SIZE-6];
  sizeP = createControlEndPacket(bufferE, hash);
  llwrite(bufferE, sizeP);

  return 0;
}
