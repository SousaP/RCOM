#include "appLayer.h"

#include <openssl/md5.h>

int size;
int sequencenumber;

void transmitter() {
    sequencenumber = 0;
    appLayer.fileDescriptor = llopen(TRANSMITTER);

    appWrite();

    lldisc();
}

void receiver() {
    int sizeReceived = 0;
    int filewriter = open(appLayer.filename,O_CREAT | O_WRONLY);
    appLayer.fileDescriptor = llopen(RECEIVER);
    appLayer.sequenceNumber = -1;

    int receivedFrames = 0;
    int failedFrames = 0;
    int n = 0;
    unsigned char buffer[MAX_FRAME_SIZE-6];
    unsigned char towrite[MAX_FRAME_SIZE-6];

    while(TRUE) {
        int bufferS = llread(buffer, MAX_FRAME_SIZE-6);

        if(bufferS == -1){
            break;
        } else if(bufferS < -1){
            failedFrames++;
        } else {
            if(buffer[0] == FRAME_C_I0){
                receivedFrames++;
            }
        }

        if(buffer[0]==P_CONTROL_START)
        {
            printf("Start Transmission\n");


            int sizelength=(int)buffer[2];

            char sizechar[300];
            memcpy(&sizechar[0], &buffer[3], sizelength);
            size = atoi(&sizechar[0]);

        }
        else if(buffer[0]==P_CONTROL_END)
        {
            printf("End Transmission\n");
            if(buffer[1] == P_T_SHA1) {

                char * datacontent = (char *)malloc(sizeReceived + 5);
                int filereader = open(appLayer.filename,O_RDONLY);
                if(read(filereader,datacontent,sizeReceived) != -1) {

                    char md5sum[16];
                    MD5(datacontent, sizeReceived, md5sum);

                    int i;
                    for(i = 0; i <= buffer[2]; i++) {
                        if(i == buffer[2]) {
                            printf("Checksum -- Success.\n");
                            break;
                        }

                        if(md5sum[i] != buffer[i+3]) {
                            printf("Checksum -- Error.\n");
                            break;
                        }
                    }
                    continue;
                }
            }

            printf("Could not verify Checksum.\n");
        }
        else
        {
            if(bufferS > 4) {


                if((appLayer.sequenceNumber+1)%128 != buffer[1]) {
                    printf("ERROR: App Sequence Number");
                    continue;
                }

                appLayer.sequenceNumber++;

                int lengthtowrite = deleteDataFlags(towrite,buffer,bufferS);

                write(filewriter,towrite,lengthtowrite);
                receivedFrames++;


                sizeReceived += lengthtowrite;
                n++;

                representloadingbar(sizeReceived,size);
            }

        }
    }

    close(filewriter);

    printf("%d received in total. %d successful frames received!\n", receivedFrames, receivedFrames - failedFrames);
}

void appWrite() {

    struct stat st;
    stat(appLayer.filename, &st);
    size = st.st_size;

    char * datacontent = (unsigned char *)malloc(size + 5);

    unsigned char bufferstart[MAX_FRAME_SIZE-6];
    int result = createStart(bufferstart);
    llwrite(bufferstart,result);


    unsigned char * datacontent = (char *)malloc(size + 5);

    int filewriter = open(appLayer.filename,O_RDONLY);
    if(read(filewriter,datacontent,size) == -1) {
        printf("ERROR: Open File\n");
        llclose();
        exit(-1);
    }

    unsigned char md5sum[16];
    MD5(datacontent, size, md5sum);


    int i;
    unsigned char data[MAX_FRAME_SIZE-6];
    unsigned char aux[MAX_FRAME_SIZE-6];
    int sentFrames = 0;
    for(i = 0; i < (int)size/appLayer.dataSize; i++) {

        memcpy(&data[0], &datacontent[i*appLayer.dataSize], appLayer.dataSize);
        int framesize = createDataFrame(aux,data,appLayer.dataSize);


        llwrite(aux,framesize);
        sentFrames++;
    }


    if(i * appLayer.dataSize < size) {
        memcpy(&data[0], &datacontent[i*appLayer.dataSize], size - i * appLayer.dataSize);
        int framesize = createDataFrame(aux, data, size - i * appLayer.dataSize);
        llwrite(aux,framesize);

        sentFrames++;
    }

    printf("%d packets sent!\n", sentFrames);


    unsigned char bufferend[MAX_FRAME_SIZE-6];
    result = createEnd(bufferend, md5sum);
    llwrite(bufferend,result);

}

int deleteDataFlags(unsigned char* aux,unsigned char* data, int dataSize) {

    int i;
    for(i=4;i<dataSize;i++)
    {
        aux[i-4]=data[i];
    }

    return dataSize-4;
}


int createDataFrame(unsigned char* aux, unsigned char* data, int dataSize) {
    aux[0]=P_CONTROL_DATA;
    aux[1]=sequencenumber%128;
    aux[2]=(dataSize/256);
    aux[3]=dataSize%256;


    int i;
    for(i=0;i<dataSize;i++) {
        aux[i+4] = data[i];

    }


    sequencenumber++;

    return 4+dataSize;
}

int createStart(unsigned char* aux) {

    aux[0] = P_CONTROL_START;

    aux[1] = P_T_SIZE;
    sprintf(&aux[3],"%d",size);
    aux[2] = strlen(&aux[3]);


    aux[3+(int)aux[2]] = P_T_NAME;
    aux[3+(int)aux[2]+1] = strlen(appLayer.filename);
    sprintf(&aux[3+(int)aux[2]+2], "%s", appLayer.filename);

    return 3 + ((int)aux[2]) + 1 + strlen(appLayer.filename);
}

int createEnd(unsigned char* aux, unsigned char* md5) {

    aux[0] = P_CONTROL_END;
    aux[1] = P_T_SHA1;
    aux[2] = 16;
    int i;
    for(i = 0; i < 16; i++)
        aux[i+3] = md5[i];

    return 3 + 16;
}

void representloadingbar(int inicio, int size) {

    system("clear");
    int i = 0;
    printf("[");

    int escrevetraco = (float)(1.0*inicio/size)*20.0;
    int escrevespaces = 20 - escrevetraco;

    for(i = 0; i < escrevetraco; i++) {
        printf("-");
    }

    for(i = 0; i < escrevespaces; i++) {
        printf("  ");
    }

    printf("] %d / %d bytes \n",inicio,size);
}
