#include "appLayer.h"
#include "packets.h"
#include <openssl/sha.h>

int size;
int numSeq;

void transmitter() {
    numSeq = 0;
    appLayer.fileDescriptor = llopen(TRANSMITTER);

    appWrite();

    lldisc();
}

void receiver() {
    int sizeReceived = 0;
    int fileW = open(appLayer.filename, O_WRONLY | O_CREAT, 0666);
    appLayer.fileDescriptor = llopen(RECEIVER);
    appLayer.sequenceNumber = -1;

    int receivedFrames = 0;
    int failedFrames = 0;
    int n = 0;
    unsigned char buffer[MAX_FRAME_SIZE-6];
    unsigned char bufferW[MAX_FRAME_SIZE-6];

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
            printf("Transmission started............\n");

            int sizelength=(int)buffer[2];

            unsigned char sizechar[300];
            memcpy(&sizechar[0], &buffer[3], sizelength);
            size = atoi(&sizechar[0]);

        }
        else if(buffer[0]==P_CONTROL_END)
        {
            printf("Transmission ended!\n");
            if(buffer[1] == P_T_SHA1) {

                char * dataC = (char *)malloc(sizeReceived + 5);
                int filereader = open(appLayer.filename,O_RDONLY);
                if(read(filereader,dataC,sizeReceived) != -1) {

                    unsigned char hash[SHA_DIGEST_LENGTH];
                    SHA1(dataC, sizeReceived, hash);
                    int i;
                    for(i = 0; i <= buffer[2]; i++) {
                        if(i == buffer[2]) {
                            printf("SHA1 Checksum -- Success.\n");
                            break;
                        }

                        if(hash[i] != buffer[i+3]) {
                            printf("SHA1 Checksum -- Error.\n");
                            break;
                        }
                    }
                    continue;
                }
            }
        }
        else
        {
            if(bufferS > 4) {

                if((appLayer.sequenceNumber+1)%128 != buffer[1]) {
                    printf("Wrong sequence number!");
                    continue;
                }
                appLayer.sequenceNumber++;

                int i;
                for(i = 4; i < bufferS; i++) {
                    bufferW[i-4] = buffer[i];
                }

                write(fileW, bufferW, bufferS - 4);
                receivedFrames++;
                sizeReceived += bufferS - 4;
                n++;
                loadingBar(sizeReceived, size);
            }
        }
    }

    close(fileW);

    printf("%d received in total. %d successful frames received!\n", receivedFrames, receivedFrames - failedFrames);
}

void appWrite() {

    struct stat st;
    stat(appLayer.filename, &st);
    size = st.st_size;

    unsigned char bufferStartPacket[MAX_FRAME_SIZE-6];
    int result = createControlStartPacket(bufferStartPacket, appLayer.filename, size);
    llwrite(bufferStartPacket,result);

    unsigned char * dataC = (char *)malloc(size + 5);
    int fileW = open(appLayer.filename, O_RDONLY);
    if(read(fileW,dataC,size) == -1) {
        printf("Error opening file: %s\n", appLayer.filename);
        llclose();
        exit(-1);
    }

    unsigned char hash[SHA_DIGEST_LENGTH];
    SHA1(dataC, size, hash);

    unsigned char data[MAX_FRAME_SIZE-6];
    unsigned char aux[MAX_FRAME_SIZE-6];
    int sentFrames = 0;
    int i;
    for(i = 0; i < (int)size/appLayer.dataSize; i++) {
        memcpy(&data[0], &dataC[i*appLayer.dataSize], appLayer.dataSize);
        int framesize = createDataPacket(aux,numSeq,appLayer.dataSize, data);
        numSeq++;
        llwrite(aux,framesize);
        sentFrames++;
    }

    if(i * appLayer.dataSize < size) {
        memcpy(&data[0], &dataC[i*appLayer.dataSize], size - i * appLayer.dataSize);
        int framesize = createDataPacket(aux, numSeq, size - i * appLayer.dataSize, data);
        numSeq++;
        llwrite(aux,framesize);
        sentFrames++;

    }

    printf("A total of %d packets were sent!\n", sentFrames);

    unsigned char bufferend[MAX_FRAME_SIZE-6];
    result = createControlEndPacket(bufferend, hash);
    llwrite(bufferend,result);

}

void loadingBar(int filled, int size) {
    system("clear");

    int bar = (float)(1.0*filled/size)*40.0;
    int totalSize = 40 - bar;
    int i = 0;
    for(i = 0; i < bar; i++) {
        printf("%c", '=');
    }
    for(i = 0; i < totalSize; i++) {
        printf(" ");
    }

    printf("\n\n%d / %d bytes sent\n",filled,size);
}