#include "app.h"  

#include <openssl/md5.h>  

int size;
int sequencenumber;

void transmitter() {
    sequencenumber = 0;
    appData.fileDescriptor = llopen(TRANSMITTER);
    
    sendFile();
    
    lldisc();
}

void receiver() {
    int sizeReceived = 0;
    int filewriter = open(appData.filename,O_CREAT | O_WRONLY);
    appData.fileDescriptor = llopen(RECEIVER);
    appData.sequenceNumber = -1;

    int receivedFrames = 0;
    int failedFrames = 0;
    int n = 0;
    char buffer[MAX_SIZE-6];
    char towrite[MAX_SIZE-6];

    while(TRUE) {
        int tamanhobuffer = llread(buffer, MAX_SIZE-6);

        if(tamanhobuffer == -1){
            break;
        } else if(tamanhobuffer < -1){
            failedFrames++;
        } else {            
            if(buffer[0] == 0){
                receivedFrames++;
            }
        }

        if(buffer[0]==AFC_C_START)
        {
            printf("Start Transmission\n");

            
            int sizelength=(int)buffer[2];
            
            char sizechar[300];
            memcpy(&sizechar[0], &buffer[3], sizelength);
            size = atoi(&sizechar[0]);
            
        }
        else if(buffer[0]==AFC_C_END)
        {
            printf("End Transmission\n");
            if(buffer[1] == AFC_T_MD5) {

                char * datacontent = (char *)malloc(sizeReceived + 5);
                int filereader = open(appData.filename,O_RDONLY);
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
            if(tamanhobuffer > 4) {

                
                if((appData.sequenceNumber+1)%128 != buffer[1]) {
                    printf("ERROR: App Sequence Number");
                    continue;
                }

                appData.sequenceNumber++;

                int lengthtowrite = deleteDataFlags(towrite,buffer,tamanhobuffer);
                
                write(filewriter,towrite,lengthtowrite);


                sizeReceived += lengthtowrite;
                n++;
                
                representloadingbar(sizeReceived,size);
            }
             
        }
    }

    close(filewriter);

    printf("%d received in total. %d successful frames received!\n", receivedFrames, receivedFrames - failedFrames);
}

void sendFile() {

    
    struct stat st;
    stat(appData.filename, &st);
    size = st.st_size;

    
    char bufferstart[MAX_SIZE-6];
    int result = createStart(bufferstart);
    llwrite(bufferstart,result);

    
    char * datacontent = (char *)malloc(size + 5);
    int filewriter = open(appData.filename,O_RDONLY);
    if(read(filewriter,datacontent,size) == -1) {
        printf("ERROR: Open File\n");
        llclose();
        exit(-1);
    }

    char md5sum[16];
    MD5(datacontent, size, md5sum);

    
    int i;
    char data[MAX_SIZE-6];
    char aux[MAX_SIZE-6];
    int sentFrames = 0;
    for(i = 0; i < (int)size/appData.dataSize; i++) {

        memcpy(&data[0], &datacontent[i*appData.dataSize], appData.dataSize);
        int framesize = createDataFrame(aux,data,appData.dataSize);


        llwrite(aux,framesize);      
        sentFrames++;
    }

    
    if(i * appData.dataSize < size) {
        memcpy(&data[0], &datacontent[i*appData.dataSize], size - i * appData.dataSize);
        int framesize = createDataFrame(aux, data, size - i * appData.dataSize);
        llwrite(aux,framesize);      
        sentFrames++;
    }

    printf("%d packets sent!\n", sentFrames);


    char bufferend[MAX_SIZE-6];
    result = createEnd(bufferend, md5sum);
    llwrite(bufferend,result);

    
}

int deleteDataFlags(char* aux,char* data, int dataSize) {

    int i;
    for(i=4;i<dataSize;i++)
    {
        aux[i-4]=data[i];
    }

    return dataSize-4;
}


int createDataFrame(char* aux, char* data, int dataSize) {
    aux[0]=AFC_C_DATA;
    aux[1]=sequencenumber%128;
    aux[2]=(dataSize/256);
    aux[3]=dataSize%256;

   
    int i;
    for(i=0;i<dataSize;i++)
    {
        aux[i+4] = data[i];
        
    }
    

    sequencenumber++;

    return 4+dataSize;
}

int createStart(char* aux) {
    
    aux[0] = AFC_C_START;
    
    

    aux[1] = AFC_T_SIZE;
    sprintf(&aux[3],"%d",size);
    aux[2] = strlen(&aux[3]);


    aux[3+(int)aux[2]] = AFC_T_NAME;
    aux[3+(int)aux[2]+1] = strlen(appData.filename);
    sprintf(&aux[3+(int)aux[2]+2], "%s", appData.filename);

    return 3 + ((int)aux[2]) + 1 + strlen(appData.filename);
}

int createEnd(char* aux, char* md5) {

    aux[0] = AFC_C_END;
    aux[1] = AFC_T_MD5;
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
