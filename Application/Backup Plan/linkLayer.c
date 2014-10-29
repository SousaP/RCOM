#include "link.h"
#include "types.h"


void resendFrame_alarm(int signo) {
    if(linkData.numFailedTransmissions >= linkData.numTransmissions) {
        printf("ERROR: Timeout\n");
        llclose(linkData.fileDescriptor);
        exit(0);
    }
    linkData.numFailedTransmissions++;

    write(linkData.fileDescriptor, linkData.frame, linkData.frameSize);
    alarm(linkData.timeout);
}

int llopen(int type) {

    signal(SIGALRM,resendFrame_alarm);



    linkData.fileDescriptor = open(linkData.port, O_RDWR | O_NOCTTY );
    if (linkData.fileDescriptor < 0) {
        perror(linkData.port); 
        exit(-1); 
    }


    if ( tcgetattr(linkData.fileDescriptor,&oldtio) == -1) {
        perror("tcgetattr");
        exit(-1);
    }



    linkData.sequenceNumber = 0;

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = linkData.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;  
    newtio.c_cc[VMIN]     = 1;  

    tcflush(linkData.fileDescriptor, TCIFLUSH);
    if ( tcsetattr(linkData.fileDescriptor,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    unsigned char set[5]; 
    set[0] = LFC_FLAG;
    set[1] = LFC_A_T;
    set[2] = LFC_C_SET;
    set[3] = LFC_A_T^LFC_C_SET;
    set[4] = LFC_FLAG;

    unsigned char ua[5];
    ua[0] = LFC_FLAG;
    ua[1] = LFC_A_T;
    ua[2] = LFC_C_UA;
    ua[3] = LFC_A_T^LFC_C_UA;
    ua[4] = LFC_FLAG;

    if (type == RECEIVER) {  
        dfaReceive(set, 5);

        write(linkData.fileDescriptor,ua,5);


    } else if (type == TRANSMITTER) {
        memcpy(&linkData.frame[0], &set[0], 5);
        linkData.frameSize = 5;
        linkData.numFailedTransmissions = 0;

        resendFrame_alarm(0);
        dfaReceive(ua, 5);


        alarm(0);
    }

    return linkData.fileDescriptor;
}

int llclose() {

    printf("Close\n");
    tcflush(linkData.fileDescriptor, TCOFLUSH);

    if ( tcsetattr(linkData.fileDescriptor, TCSANOW, &oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
  }

  close(linkData.fileDescriptor);
}

void dfaReceive(unsigned char* frame, int frameSize) {

    if (frameSize <= 0)
        return;

    int STOP = FALSE;
    int framePos = -1;
    int tries = 0;


    while(STOP == FALSE) {
        char tmp[2];
        read(linkData.fileDescriptor, tmp, 1);



        if (frame[framePos+1] == tmp[0]) {
            framePos++;
        } else if (frame[0] == tmp[0]) {
            framePos = 0;
            tries++;
        } else {
            framePos = -1;
            tries++;
        }

        if (framePos == frameSize-1) {
            STOP = TRUE;            
        }
    }

}

int llwrite(unsigned char * buffer, int length) {
    
    char uFrame[MAX_SIZE*2];

    int xor = 0;
    int i;
    for(i = 0; i < length; i++) {
        xor ^= buffer[i];
    }

    uFrame[0] = LFC_FLAG;
    uFrame[1] = LFC_A_T;

    if(linkData.sequenceNumber == 0)
        uFrame[2] = LFC_C_I0;
    else
        uFrame[2] = LFC_C_I1;
    
    uFrame[3] = uFrame[1]^uFrame[2];

    memcpy(&uFrame[4], &buffer[0], length);

    uFrame[4+length] = xor;
    uFrame[4+length+1] = LFC_FLAG;

    linkData.frameSize = stuffing(uFrame, 4+length+2, linkData.frame);

    linkData.numFailedTransmissions = 0;
    resendFrame_alarm(0);

    while(TRUE) {
        int action = waitResponse();

        if(action == 0) {
            alarm(0);
            break;

        } else {
            alarm(0);
            resendFrame_alarm(0);
        }
    }

    if(linkData.sequenceNumber == 0)
        linkData.sequenceNumber = 1;
    else
        linkData.sequenceNumber = 0;

    return length + 6;
}

int llread(unsigned char * buffer, int length) {



    if(length > MAX_SIZE-6) {
        printf("ERROR: Invalid Length\n");
        return -2;
    }

    char sFrame[MAX_SIZE*2];
    char uFrame[MAX_SIZE];
    int thisSequenceNumber;

    int pos = 0;
    int flag = 0;

    while (flag < 2) {

        if(pos >= MAX_SIZE*2) {
            pos = 0;
            flag = 0;          
        }

        int res = read(linkData.fileDescriptor, &sFrame[pos], 1); 
        if(sFrame[pos] == LFC_FLAG && flag == 0) {
            pos++;
            flag++;


        } else if(sFrame[pos] == LFC_FLAG && flag == 1 && pos == 1) {
            continue;


        } else if(sFrame[pos] == LFC_FLAG && flag == 1 && pos < 4) {
            pos = 1;
            flag = 1;


        } else if(sFrame[pos] != LFC_FLAG && flag == 0) {
            continue;


        } else if(flag == 1) {

            if(pos == 1){
                if(sFrame[pos] != LFC_A_T) {
                    sendREJ(linkData.sequenceNumber);
                    return -4;
                } else {
                    pos++;
                }
            } else if(pos == 2) {
                if(sFrame[pos] == LFC_C_DISC) {
                    pos++;
                } else if(sFrame[pos] == LFC_C_I0) {
                    thisSequenceNumber = 0;
                    pos++;
                } else if(sFrame[pos] == LFC_C_I1) {
                    thisSequenceNumber = 1;
                    pos++;
                } else {              
                    sendREJ(linkData.sequenceNumber);
                    return -4;
                }
            } else if(pos == 3) {
                if(sFrame[pos] != (sFrame[pos-2]^sFrame[pos-1]) ){
                    sendREJ(thisSequenceNumber);
                    return -4;
                } else {
                    pos++;
                }

            } else if(sFrame[pos] == LFC_FLAG) {
                pos++;
                flag++;


            } else {
                pos++;
            }
        }
    }
    int uFrameSize = unstuffing(sFrame, pos, uFrame);

    if(uFrameSize == 5) {

        if(uFrame[2] == LFC_C_DISC) {


            linkData.frame[0] = LFC_FLAG;
            linkData.frame[1] = LFC_A_R;
            linkData.frame[2] = LFC_C_DISC;    
            linkData.frame[3] = linkData.frame[1]^linkData.frame[2];
            linkData.frame[4] = LFC_FLAG;

            linkData.frameSize = 5;
            linkData.numFailedTransmissions = 0;

            resendFrame_alarm(0);


            unsigned char uaDisc[5];
            uaDisc[0] = LFC_FLAG;
            uaDisc[1] = LFC_A_R;
            uaDisc[2] = LFC_C_UA;
            uaDisc[3] = uaDisc[1]^uaDisc[2];
            uaDisc[4] = LFC_FLAG;

            dfaReceive(uaDisc, 5);

            alarm(0);

            return -1;


        } else {
            return -3;
        }


    } else if(uFrameSize > 5) {

        int i;
        int xor = 0;
        for(i = 4; i < uFrameSize-2; i++)
            xor ^= uFrame[i];


        if(xor != uFrame[uFrameSize-2]) {
            printf("**BCC2 - XOR FAILED**\n");
            sendREJ(thisSequenceNumber);
            return -4;
        }
    } else {
        printf("Unexpected Error on llread()\n");
        exit(-1);
    }

    

    if(length < linkData.frameSize-7)
        return -2;


    unsigned char temp[5];
    temp[0] = LFC_FLAG;
    temp[1] = LFC_A_T;

    if(thisSequenceNumber == 0) {
        temp[2] = LFC_C_RR1;        
    } else {
        temp[2] = LFC_C_RR0;
    }

    temp[3] = temp[1]^temp[2];
    temp[4] = LFC_FLAG;

    write(linkData.fileDescriptor, temp, 5);
    
    if(linkData.sequenceNumber != thisSequenceNumber)
        return llread(buffer, length);

    if(thisSequenceNumber == 0)
        linkData.sequenceNumber = 1;
    else
        linkData.sequenceNumber = 0;



    memcpy(&buffer[0], &uFrame[4], uFrameSize-6);
    return uFrameSize-6;
}

int lldisc() {



    linkData.frame[0] = LFC_FLAG;
    linkData.frame[1] = LFC_A_T;
    linkData.frame[2] = LFC_C_DISC;    
    linkData.frame[3] = linkData.frame[1]^linkData.frame[2];
    linkData.frame[4] = LFC_FLAG;

    linkData.frameSize = 5;

    linkData.numFailedTransmissions = 0;

    resendFrame_alarm(0);



    char discReceived[5];

    discReceived[0] = LFC_FLAG;
    discReceived[1] = LFC_A_R;
    discReceived[2] = LFC_C_DISC;    
    discReceived[3] = discReceived[1]^discReceived[2];
    discReceived[4] = LFC_FLAG;

    dfaReceive(discReceived, 5);

    alarm(0);

    
    linkData.frame[0] = LFC_FLAG;
    linkData.frame[1] = LFC_A_R;
    linkData.frame[2] = LFC_C_UA;
    linkData.frame[3] = linkData.frame[1]^linkData.frame[2];
    linkData.frame[4] = LFC_FLAG;

    linkData.frameSize = 5;

    write(linkData.fileDescriptor, linkData.frame, linkData.frameSize);

    sleep(2);
}

int waitResponse() {
    int pos = 0;
    int action = 0;
    char rf[2];
    while(TRUE) {
        char tmp[2];
        read(linkData.fileDescriptor, tmp, 1);

    

        if(pos == 0 && tmp[0] == LFC_FLAG) {
            pos++;
        } else if(pos == 1 && tmp[0] == LFC_A_T) {
            rf[0] = tmp[0];
            pos++;            
        } else if(pos == 2 && linkData.sequenceNumber == 0 && tmp[0] == LFC_C_RR1) {
            rf[1] = tmp[0];
            action = 0;
            pos++;
        } else if(pos == 2 && linkData.sequenceNumber == 1 && tmp[0] == LFC_C_RR0) {
            rf[1] = tmp[0];
            action = 0;
            pos++;
        } else if(pos == 2 && linkData.sequenceNumber == 0 && tmp[0] == LFC_C_REJ1) {
            rf[1] = tmp[0];
            action = 1;
            pos++;
        } else if(pos == 2 && linkData.sequenceNumber == 1 && tmp[0] == LFC_C_REJ0) {
            rf[1] = tmp[0];
            action = 1;
            pos++;
        } else if(pos == 3 && tmp[0] == rf[0]^rf[1]) {
            pos++;
        } else if(pos == 4 && tmp[0] == LFC_FLAG) {
            break;
        } else if(tmp[0] == LFC_FLAG) {
            pos = 1;
        } else {
            pos = 0;
        }
    }
    return action;
}

void sendREJ(int sn) {
    unsigned char temp[5];
    temp[0] = LFC_FLAG;
    temp[1] = LFC_A_T;

    if(sn == 0) {
        temp[2] = LFC_C_REJ1;        
    } else {
        temp[2] = LFC_C_REJ0;
    }

    temp[3] = temp[1]^temp[2];
    temp[4] = LFC_FLAG;

    write(linkData.fileDescriptor, temp, 5);

    printf("Sent REJ\n");
}

