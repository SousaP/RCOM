#include "linkLayer.h"

void resendFrame_alarm(int signo) {
    if(linkLayer.numFailedTransmissions >= linkLayer.numTransmissions) {
        printf("ERROR: Timeout\n");
        llclose(linkLayer.fileDescriptor);
        exit(0);
    }
    linkLayer.numFailedTransmissions++;

    write(linkLayer.fileDescriptor, linkLayer.frame, linkLayer.frameSize);
    alarm(linkLayer.timeout);
}

/*
*@param Data to be sent
*@param Data size
*@param Frame to be created
*@ret int -> Total frame size
*/
int createInformationFrame(char* data, size_t dataSize, char* frame) {

	char* stuffInfo = (char*)malloc(sizeof(char)*MAX_SIZE);

	frame[0] = FLAG; // estava F
	frame[1] = FRAME_A_T ;  // estava A
	frame[2] = FRAME_C_SET; // estava SETUP
	frame[3] = FRAME_A_T  ^ FRAME_C_SET;  // estava BCC1

	char BCC2 = data[0];

	int i;
	for(i = 0; i < dataSize; i++) {
		BCC2 ^= data[i];
	}

	int size = byteStuffing(data, dataSize, stuffInfo);

	memcpy(&frame[4], stuffInfo, size);

	frame[size + 4] = BCC2;
	frame[size + 5] = FLAG;

	return (size+6);
}

/*
*@param Frame to be created
*@param A frame param
*@param C frame param
*@ret int 0 -> Frame configured
*/
int createSupervisionFrame(char* frame, char A, char C) {
	frame[0] = FLAG;
	frame[1] = A;
	frame[2] = C;
	frame[3] = A^C;
	frame[4] = FLAG;

	return 0;
}

/*
*@param Frame for BCC comparation
*@param Frame size
*@ret int -2->Error in BCC2
*     -1->Error in BCC1
*     0->Correct BCC
*/
int bccChecker(char* frame, size_t size) {
	if(*(frame + 3) != (*(frame + 2) ^ *(frame + 1))) {
		printf("BCC1 != BCC2");
		return -1;
	}

	char xorData = *(frame + 4);

	int i;
	for(i = 1; i < size; i++) {
		xorData ^= *(frame + 4 + i);
	}

	if(xorData != *(frame + 4 + size)) {
		return -2;
	}
	else return 0;

}

/*
*@param buffer
*@param count
*@ret int -2->Error in BCC2
*     -1->Error in BCC1
*     0->Correct BCC
*/
unsigned short checksum(unsigned short *buffer, int count) {
	register unsigned short sum = 0;
	while(count--) {
		sum += *buffer++;
		if(sum & 0xFFFF0000) {
			sum &= 0xFFFF;
			sum++;
		}
	}
	return ~(sum & 0xFFFF);
}


/*
*@param data
*@param dataSize
*@param stuff
*@ret iterador do stuffing
*/
int byteStuffing(char* data, int dataSize, char* stuff) {
	int i = 0;
	int stuffITR = 0;
	char byte;
	while(i < dataSize) {
		byte = data[i];
		if(byte == FLAG) {
			stuff[stuffITR] = ESCAPE;
			stuffITR++;
			stuff[stuffITR] = FLAG_AUX;
			stuffITR++;
		}
		else if(byte == ESCAPE) {
			stuff[stuffITR] = ESCAPE;
			stuffITR++;
			stuff[stuffITR] = ESCAPE_AUX;
			stuffITR++;
		}
		else{
			stuff[stuffITR] = byte;
			stuffITR++;
		}
		++i;
	}
	return stuffITR;
}


/*
*@param stuff
*@param stuffSize
*@param data
*@ret iterador do stuffing
*/
int byteDestuffing(char* stuff, int stuffSize, char* data) {
	int i = 0;
	int stuffITR = 0;
	char byte;
	while(i < stuffSize){
		byte = stuff[i];
		if(byte == ESCAPE){
			byte = stuff[++i];
			if(byte == FLAG_AUX){
				data[stuffITR] = FLAG;
				stuffITR++;
			}
			else if(byte == ESCAPE_AUX){
				data[stuffITR] = ESCAPE;
				stuffITR++;
			}
			else{
				return -1;
			}
		}
		else{
			data[stuffITR] = byte;
			stuffITR++;
		}
		++i;
	}
	return stuffITR;
}

int llopen(){

    signal(SIGALRM,resendFrame_alarm);

	linkLayer.fileDescriptor = open(linkLayer.port, O_RDWR | O_NOCTTY );
    if (linkLayer.fileDescriptor < 0) {
        perror(linkLayer.port);
        exit(-1);
    }


    if ( tcgetattr(linkLayer.fileDescriptor,&oldtio) == -1) {
        perror("tcgetattr");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = linkLayer.baudRate | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = OPOST;

    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;
    newtio.c_cc[VMIN]     = 1;

    char UA[5];
    createSupervisionFrame(UA,FRAME_A_T,FRAME_C_UA);

    char SET[5];
    createSupervisionFrame(SET,FRAME_A_T,FRAME_C_SET);

    if(appMode == TRANSMITTER){
    	memcpy(&linkLayer.frame[0], &SET[0], 5);
    	linkLayer.frameSize = 5;
        linkLayer.numFailedTransmissions = 0;

        resendFrame_alarm(0);
        validator(UA, 5);

        alarm(0);

    }

    else if(appMode == RECEIVER){
        write(linkLayer.fileDescriptor, UA, sizeof(UA));

    }
    return linkLayer.fileDescriptor;
}


/*
*@param frame
*@param frameSize
*/
void validator(unsigned char* frame, int frameSize) {

    if (frameSize <= 0)
        return;

    int STOP = FALSE;
    int framePos = -1;
    int tries = 0;


    while(STOP == FALSE) {
        char tmp[2];
        read(linkLayer.fileDescriptor, tmp, 1);

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


/*
Close port
*/
int llclose() {

	if(appMode == TRANSMITTER){
		char discS[5];

		createSupervisionFrame(discS,FRAME_A_T,FRAME_C_DISC);
		memcpy(&linkLayer.frame[0], &discS[0], 5);
    	linkLayer.frameSize = 5;
    	linkLayer.numFailedTransmissions = 0;

   	 	resendFrame_alarm(0);

   	 	char discR[5];
    	createSupervisionFrame(discR,FRAME_A_T,FRAME_C_DISC);

    	validator(discR, 5);

    	alarm(0);

    	char UA[5];
    	createSupervisionFrame(UA,FRAME_A_T,FRAME_C_UA);
		memcpy(&linkLayer.frame[0], &UA[0], 5);
    	linkLayer.frameSize = 5;
    	linkLayer.numFailedTransmissions = 0;

   		resendFrame_alarm(0);

        alarm(0);

   		sleep(2);
	}
	if(appMode == RECEIVER){
		char discR[5];
    	createSupervisionFrame(discR,FRAME_A_T,FRAME_C_DISC);

    	validator(discR, 5);

    	alarm(0);

    	char discS[5];

		createSupervisionFrame(discS,FRAME_A_T,FRAME_C_DISC);
		memcpy(&linkLayer.frame[0], &discS[0], 5);
    	linkLayer.frameSize = 5;
    	linkLayer.numFailedTransmissions = 0;

   	 	resendFrame_alarm(0);

        char UA[5];
        createSupervisionFrame(UA,FRAME_A_T,FRAME_C_UA);

        validator(UA, 5);
	}

    printf("Close\n");
    tcflush(linkLayer.fileDescriptor, TCOFLUSH);

    if ( tcsetattr(linkLayer.fileDescriptor, TCSANOW, &oldtio) == -1) {
      perror("tcsetattr");
      exit(-1);
  	}

  	close(linkLayer.fileDescriptor);
}

/*
Wait for response
*/
int waitResponse() {
    int pos = 0;
    int action = 0;
    char rf[2];
    while(TRUE) {
        char tmp[2];
        read(linkLayer.fileDescriptor, tmp, 1);



        if(pos == 0 && tmp[0] == FLAG) {
            pos++;
        } else if(pos == 1 && tmp[0] == FRAME_A_T) {
            rf[0] = tmp[0];
            pos++;
        } else if(pos == 2 && linkLayer.sequenceNumber == 0 && tmp[0] == FRAME_C_RR1) {
            rf[1] = tmp[0];
            action = 0;
            pos++;
        } else if(pos == 2 && linkLayer.sequenceNumber == 1 && tmp[0] == FRAME_C_RR0) {
            rf[1] = tmp[0];
            action = 0;
            pos++;
        } else if(pos == 2 && linkLayer.sequenceNumber == 0 && tmp[0] == FRAME_C_REJ1) {
            rf[1] = tmp[0];
            action = 1;
            pos++;
        } else if(pos == 2 && linkLayer.sequenceNumber == 1 && tmp[0] == FRAME_C_REJ0) {
            rf[1] = tmp[0];
            action = 1;
            pos++;
        } else if(pos == 3 && tmp[0] == rf[0]^rf[1]) {
            pos++;
        } else if(pos == 4 && tmp[0] == FLAG) {
            break;
        } else if(tmp[0] == FLAG) {
            pos = 1;
        } else {
            pos = 0;
        }
    }
    return action;
}

void sendREJ(int mode) {
	char C;
    if(mode == RECEIVER) {
        C = FRAME_C_REJ1;
    } else {
        C = FRAME_C_REJ0;
    }

    char rej[5];
	createSupervisionFrame(rej,FRAME_A_T,C);
    write(linkLayer.fileDescriptor, rej, 5);

    printf("Sent REJ\n");
}

int llwrite(unsigned char * buffer, int length) {

    char uFrame[STUFF_MAX_SIZE];
    int xor = 0;
    int i;
    for(i = 0; i < length; i++) {
        xor ^= buffer[i];
    }

    uFrame[0] = FLAG;
    uFrame[1] = FRAME_A_T ;

    if(linkLayer.sequenceNumber == 0)
        uFrame[2] = FRAME_C_I0;
    else
        uFrame[2] = FRAME_C_I1;

    uFrame[3] = uFrame[1]^uFrame[2];

    memcpy(&uFrame[4], &buffer[0], length);

    uFrame[4+length] = xor;
    uFrame[4+length+1] = FLAG;

    linkLayer.frameSize = byteStuffing(uFrame, 4+length+2, linkLayer.frame);

    linkLayer.numFailedTransmissions = 0;
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

    if(linkLayer.sequenceNumber == 0)
        linkLayer.sequenceNumber = 1;
    else
        linkLayer.sequenceNumber = 0;

    return length + 6;
}

int llread(unsigned char * buffer, int length) {



    if(length > MAX_FRAME_SIZE-6) {
        printf("ERROR: Invalid Length\n");
        return -2;
    }

    char sFrame[MAX_FRAME_SIZE*2];
    char uFrame[STUFF_MAX_SIZE];
    int thisSequenceNumber;

    int pos = 0;
    int flag = 0;

    while (flag < 2) {

        if(pos >= MAX_FRAME_SIZE*2) {
            pos = 0;
            flag = 0;
        }

        int res = read(linkLayer.fileDescriptor, &sFrame[pos], 1);
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
                    sendREJ(linkLayer.sequenceNumber);
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
                    sendREJ(linkLayer.sequenceNumber);
                    return -4;
                }
            } else if(pos == 3) {
                if(sFrame[pos] != (sFrame[pos-2]^sFrame[pos-1]) ){
                    sendREJ(thisSequenceNumber);
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
    int uFrameSize = byteDestuffing(sFrame, pos, uFrame);

    if(uFrameSize == 5) {

        if(uFrame[2] == FRAME_C_DISC) {


            linkLayer.frame[0] = FLAG;
            linkLayer.frame[1] = FRAME_A_T;
            linkLayer.frame[2] = FRAME_C_DISC;
            linkLayer.frame[3] = linkLayer.frame[1]^linkLayer.frame[2];
            linkLayer.frame[4] = FLAG;

            linkLayer.frameSize = 5;
            linkLayer.numFailedTransmissions = 0;

            resendFrame_alarm(0);


            unsigned char uaDisc[5];
            uaDisc[0] = FLAG;
            uaDisc[1] = FRAME_A_T;
            uaDisc[2] = FRAME_C_UA;
            uaDisc[3] = uaDisc[1]^uaDisc[2];
            uaDisc[4] = FLAG;

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
            printf("**BCC2 - XOR FAILED**\n");
            sendREJ(thisSequenceNumber);
            return -4;
        }
    } else {
        printf("Unexpected Error on llread()\n");
        exit(-1);
    }



    if(length < linkLayer.frameSize-7)
        return -2;


    unsigned char temp[5];
    temp[0] = FLAG;
    temp[1] = FRAME_A_T;

    if(thisSequenceNumber == 0) {
        temp[2] =  FRAME_C_RR1; //  estava LFC_C_RR1
    } else {
        temp[2] = FRAME_C_RR0; //  estava LFC_C_RR0
    }

    temp[3] = temp[1]^temp[2];
    temp[4] = FLAG;

    write(linkLayer.fileDescriptor, temp, 5);

    if(linkLayer.sequenceNumber != thisSequenceNumber)
        return llread(buffer, length);

    if(thisSequenceNumber == 0)
        linkLayer.sequenceNumber = 1;
    else
        linkLayer.sequenceNumber = 0;



    memcpy(&buffer[0], &uFrame[4], uFrameSize-6);
    return uFrameSize-6;
}
