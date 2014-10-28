#include "appLayer.h"
#include <openssl/sha.h>

int numSeq;
int size;

int create_file(char* filename){
  //retorna o fd do ficheiro que cria
  printf("\nCreating file %s", filename);
  return open(filename, O_WRONLY | O_CREAT | O_APPEND | O_TRUNC, 0744);
}

void close_file(int fd){
  close(fd);
}

int open_file(char* filename){
//retorna o Fd do ficheiro que tenta abrir
  int file_fd = open(filename, O_RDONLY);

  stat(filename, &st);
  if(st.st_size == 0)
      return -1;

  return file_fd;
}

int receiver(){
  int sizeR = 0;
  int fileW = create_file(appLayer.filename);
  appLayer.fileDescriptor = llopen(RECEIVER);
  appLayer.sequenceNumber = -1;

  int receivedFrames = 0;
  int badFrames = 0;
  int n = 0;
  char buffer[MAX_FRAME_SIZE - 6];
  char writeBUF[MAX_FRAME_SIZE - 6];

  while(1) {

    int bufferSize = llread(buffer, MAX_FRAME_SIZE - 6);

    if(bufferSize < -1) {
      badFrames++;
    }
    else if(bufferSize == -1) {
      break;
    }
    else {
      if(buffer[0] == 0) {
        receivedFrames++;
      }
    }

    if(buffer[0] == P_CONTROL_START) {
      printf("Transmition started.......\n");

      int sizeTemp = (int) buffer[2];

      char sizeC[100];
      memcpy(&sizeC[0], &buffer[3], sizeTemp);
      size = atoi(&sizechar[0]);
    }
    else if(buffer[0] == P_CONTROL_END) {
      printf("Transmition ended!\n");
      if(buffer[1] == P_T_SHA1) {
        int fileR = openFile(appLayer.filename);
        char *data = (char*) malloc(sizeR + 5);

       if(read(fileR, data, sizeR) != -1) {
          unsigned char hash[256];
          SHA1(data, sizeR, hash);

          int i;
          for(i = 0; i <= buffer[2]; i++) {
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

        write(fileR, writeBUF, bufferSize - 4);

        sizeR += bufferSize - 4;
        n++;
      }
    }
  }
}
int transmitter(){
  numSeq = 0;
  appLayer.fileDescriptor = llopen(TRANSMITTER);

  write();

  lldisc();
}

int read() {

}

int write() {

}
