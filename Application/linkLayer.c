#include "linkLayer.h"

/*
*@param Data to be sent
*@param Data size
*@param Frame to be created
*@ret int -> Total frame size
*/
int createInformationFrame(char* data, size_t dataSize, char* frame) {

	char* stuffInfo = (char*)malloc(sizeof(char)*MAX_SIZE);

	frame[0] = F;
	frame[1] = A;
	frame[2] = C_SETUP;
	frame[3] = BCC1;

	char BCC2 = data[0];

	int i;
	for(i = 0; i < dataSize; i++) {
		BCC2 ^= data[i];
	}

	int size = byteStuffing(data, dataSize, stuffInfo);

	memcpy(&frame[4], stuffInfo, size);

	frame[size + 4] = BCC2;
	frame[size + 5] = F;

	return (size+6);
}

/*
*@param Frame to be created
*@param A frame param
*@param C frame param
*@ret int 0 -> Frame configured
*/
int createSupervisionFrame(char* frame, char A, char C) {
	frame[0] = F;
	frame[1] = A;
	frame[2] = C;
	frame[3] = BCC1;
	frame[4] = F;

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

	if(xorData != *(frame 4 + size)) {
		return -2;
	}
	else return 0;

}

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