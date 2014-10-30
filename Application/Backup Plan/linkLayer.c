#include "linkLayer.h"
#include "appLayer.h"

void resendFrameAlrm(int signo) {
    if(lLayer.numFailedTransmissions >= lLayer.numTransmissions) {
        printf("ERROR: Timeout\n");
        llclose(lLayer.fileDescriptor);
        exit(-1);
    }
    lLayer.numFailedTransmissions++;

    write(lLayer.fileDescriptor, lLayer.frame, lLayer.frameSize);
    alarm(lLayer.timeout);
}

int llopen(int type) {

    signal(SIGALRM,resendFrameAlrm);

    lLayer.fileDescriptor = open(lLayer.port, O_RDWR | O_NOCTTY );
    if (lLayer.fileDescriptor < 0) {
        perror(lLayer.port);
        exit(-1);
    }


    if ( tcgetattr(lLayer.fileDescriptor,&oldtio) == -1) {
        perror("tcgetattr");
        exit(-1);
    }

    lLayer.sequenceNumber = 0;

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = lLayer.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;
    newtio.c_cc[VMIN]     = 1;

    tcflush(lLayer.fileDescriptor, TCIFLUSH);
    if ( tcsetattr(lLayer.fileDescriptor,TCSANOW,&newtio) == -1) {
      perror("tcsetattr");
      exit(-1);
    }

    unsigned char set[5];
    createSupervisionFrame(set,FRAME_A_T,FRAME_C_SET);


    unsigned char ua[5];
    createSupervisionFrame(ua,FRAME_A_T,FRAME_C_UA);

    if (type == RECEIVER) {
        validator(set, 5);

        write(lLayer.fileDescriptor,ua,5);


    } else if (type == TRANSMITTER) {
        memcpy(&lLayer.frame[0], &set[0], 5);
        lLayer.frameSize = 5;
        lLayer.numFailedTransmissions = 0;

        resendFrameAlrm(0);
        validator(ua, 5);


        alarm(0);
    }

    return lLayer.fileDescriptor;
}

int llclose() {

    printf("Close\n");
    tcflush(lLayer.fileDescriptor, TCOFLUSH);

    if ( tcsetattr(lLayer.fileDescriptor, TCSANOW, &oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
  }

  close(lLayer.fileDescriptor);

}

int llwrite(unsigned char * buffer, int length) {

    unsigned char uFrame[STUFF_MAX_SIZE];

    int xor = 0;
    int i;
    for(i = 0; i < length; i++) {
        xor ^= buffer[i];
    }

    uFrame[0] = FLAG;
    uFrame[1] = FRAME_A_T;

    if(lLayer.sequenceNumber == 0)
        uFrame[2] = FRAME_C_I0;
    else
        uFrame[2] = FRAME_C_I1;

    uFrame[3] = uFrame[1]^uFrame[2];

    memcpy(&uFrame[4], &buffer[0], length);

    uFrame[4+length] = xor;
    uFrame[4+length+1] = FLAG;

    lLayer.frameSize = stuffing(uFrame, 4+length+2, lLayer.frame);

    lLayer.numFailedTransmissions = 0;
    resendFrameAlrm(0);

    while(TRUE) {
        int watDo = waitForSignal();

        if(watDo == 0) {
            alarm(0);
            break;

        } else {
            alarm(0);
            resendFrameAlrm(0);
        }
    }

    if(lLayer.sequenceNumber == 0)
        lLayer.sequenceNumber = 1;
    else
        lLayer.sequenceNumber = 0;

    return length + 6;
}

int llread(unsigned char * buffer, int length) {

    if(length > MAX_FRAME_SIZE-6) {
        printf("ERROR: Invalid Length\n");
        return -2;
    }

    unsigned char sFrame[STUFF_MAX_SIZE];
    unsigned char uFrame[MAX_FRAME_SIZE];
    int thisSequenceNumber;

    int pos = 0;
    int flag = 0;

    while (flag < 2) {

        if(pos >= STUFF_MAX_SIZE) {
            pos = 0;
            flag = 0;
        }

        int res = read(lLayer.fileDescriptor, &sFrame[pos], 1);
        if(sFrame[pos] == FLAG && flag == 0) {
            pos++;
            flag++;
        } else if(sFrame[pos] == FLAG && flag == 1 && pos == 1) {
            continue;
        } else if(sFrame[pos] == FLAG && flag == 1 && pos < 4) {
            pos = 1;
            flag = 1;
        } else if(sFrame[pos] != FLAG && flag == 0) {
            continue;
        } else if(flag == 1) {
            if(pos == 1){
                if(sFrame[pos] != FRAME_A_T) {
                    sendREJsignal(lLayer.sequenceNumber);
                    return -4;
                } else {
                    pos++;
                }
            } else if(pos == 2) {
                if(sFrame[pos] == FRAME_C_DISC) {
                    pos++;
                } else if(sFrame[pos] == FRAME_C_I0) {
                    thisSequenceNumber = 0;
                    pos++;
                } else if(sFrame[pos] == FRAME_C_I1) {
                    thisSequenceNumber = 1;
                    pos++;
                } else {
                    sendREJsignal(lLayer.sequenceNumber);
                    return -4;
                }
            } else if(pos == 3) {
                if(sFrame[pos] != (sFrame[pos-2]^sFrame[pos-1]) ){
                    sendREJsignal(thisSequenceNumber);
                    return -4;
                } else {
                    pos++;
                }
            } else if(sFrame[pos] == FLAG) {
                pos++;
                flag++;
            } else {
                pos++;
            }
        }
    }

    int uFrameSize = unstuffing(sFrame, pos, uFrame);

    if(uFrameSize == 5) {

        if(uFrame[2] == FRAME_C_DISC) {

          createSupervisionFrame(lLayer.frame,FRAME_A_R,FRAME_C_DISC);

            lLayer.frameSize = 5;
            lLayer.numFailedTransmissions = 0;

            resendFrameAlrm(0);

            unsigned char uaDisc[5];
            createSupervisionFrame(uaDisc,FRAME_A_R,FRAME_C_UA);

            validator(uaDisc, 5);

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
            printf("BCC comparasion failed\n");
            sendREJsignal(thisSequenceNumber);
            return -4;
        }
    } else {
        printf("llread failed!)\n");
        return -1;
    }

    if(length < lLayer.frameSize-7)
        return -2;

    unsigned char temp[5];
    temp[0] = FLAG;
    temp[1] = FRAME_A_T;

    if(thisSequenceNumber == 0) {
        temp[2] = FRAME_C_RR1;
    } else {
        temp[2] = FRAME_C_RR0;
    }

    temp[3] = temp[1]^temp[2];
    temp[4] = FLAG;

    write(lLayer.fileDescriptor, temp, 5);

    if(lLayer.sequenceNumber != thisSequenceNumber)
        return llread(buffer, length);

    if(thisSequenceNumber == 0)
        lLayer.sequenceNumber = 1;
    else
        lLayer.sequenceNumber = 0;

    memcpy(&buffer[0], &uFrame[4], uFrameSize-6);
    return uFrameSize-6;
}

int lldisc() {

    createSupervisionFrame(lLayer.frame,FRAME_A_T,FRAME_C_DISC);

    lLayer.frameSize = 5;
    lLayer.numFailedTransmissions = 0;

    resendFrameAlrm(0);

    unsigned char discReceived[5];
    createSupervisionFrame(discReceived,FRAME_A_R,FRAME_C_DISC);
    validator(discReceived, 5);

    alarm(0);

    createSupervisionFrame(lLayer.frame,FRAME_A_R,FRAME_C_UA);

    lLayer.frameSize = 5;

    write(lLayer.fileDescriptor, lLayer.frame, lLayer.frameSize);

    return 0;
}

void validator(unsigned char* frame, int frameSize) {

    if (frameSize <= 0)
        return;

    int STOP = FALSE;
    int framePos = -1;

    while(STOP == FALSE) {
        unsigned char ch[1];
        read(lLayer.fileDescriptor, ch, 1);

        if (frame[framePos+1] == ch[0]) {
            framePos++;
        } else if (frame[0] == ch[0]) {
            framePos = 0;
        } else {
            framePos = -1;
        }

        if (framePos == frameSize-1) {
            STOP = TRUE;
        }
    }
}

int waitForSignal() {
    int pos = 0;
    int watDo = 0;
    char xorF[2];
    while(TRUE) {
        unsigned char ch[1];
        read(lLayer.fileDescriptor, ch, 1);

        if(pos == 0 && ch[0] == FLAG) {
            pos++;
        } else if(pos == 1 && ch[0] == FRAME_A_T) {
            xorF[0] = ch[0];
            pos++;
        } else if(pos == 2 && lLayer.sequenceNumber == 0 && ch[0] == FRAME_C_RR1) {
            xorF[1] = ch[0];
            watDo = 0;
            pos++;
        } else if(pos == 2 && lLayer.sequenceNumber == 1 && ch[0] == FRAME_C_RR0) {
            xorF[1] = ch[0];
            watDo = 0;
            pos++;
        } else if(pos == 2 && lLayer.sequenceNumber == 0 && ch[0] == FRAME_C_REJ1) {
            xorF[1] = ch[0];
            watDo = 1;
            pos++;
        } else if(pos == 2 && lLayer.sequenceNumber == 1 && ch[0] == FRAME_C_REJ0) {
            xorF[1] = ch[0];
            watDo = 1;
            pos++;
        } else if(pos == 3 && ch[0] == xorF[0]^xorF[1]) {
            pos++;
        } else if(pos == 4 && ch[0] == FLAG) {
            break;
        } else if(ch[0] == FLAG) {
            pos = 1;
        } else {
            pos = 0;
        }
    }
    return watDo;
}

void sendREJsignal(int sig) {
    unsigned char temp[5];
    temp[0] = FLAG;
    temp[1] = FRAME_A_T;

    if(sig == 0) {
        temp[2] = FRAME_C_REJ1;
    } else {
        temp[2] = FRAME_C_REJ0;
    }

    temp[3] = temp[1]^temp[2];
    temp[4] = FLAG;

    write(lLayer.fileDescriptor, temp, 5);

    printf("Sent REJ signal\n");
}

void createSupervisionFrame(char* frame, unsigned char A, unsigned char C){
    frame[0] = FLAG;
    frame[1] = A;
    frame[2] = C;
    frame[3] = A^C;
    frame[4] = FLAG;
}