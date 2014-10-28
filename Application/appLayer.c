#include "appLayer.h"

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
  create_file(appLayer.filename);
  appLayer.fileDescriptor = llopen(RECEIVER);
  appLayer.sequenceNumber = -1;

  int receivedFrames = 0;
  int badFrames = 0;
  int n = 0;
  char buffer[MAX_FRAME_SIZE - 6];
  char writeBUF[MAX_FRAME_SIZE - 6];

  while(1) {
    
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

int main(int argc, char* argv[])
{
  if (argc < 4)
    {
      printf("./program <r/w> <dev> <file>\n");
      return -1;
      }


  char *mode = argv[1];
  char *device = argv[2];
  char *filename=argv[3];

  if(mode[0]!='w' && mode[0] !='r' )
  {
    printf("./program <r/w> <dev> <file>\n");
    return -1;
  }
  else if(mode[0] == 'w')
  return Transmissor();
  else if(mode[0] == 'r')
  return Receiver();


  return -1;
}
