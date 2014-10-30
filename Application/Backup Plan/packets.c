#include "packets.h"
#include <openssl/sha.h>
/*
*@param Packet to be created
*@param Sequence number
*@param Length of packet data
*@param Packet data
*@ret int -> Total packet size
*/
int createDataPacket(unsigned char* packet, int seqNumber, int length, unsigned char* data) {
	packet[0] = P_CONTROL_DATA;
	packet[1] = seqNumber % 128;
	packet[2] = length / 256;
	packet[3] = length % 256;

	int i;
	for (i = 0; i < length; i++) {
		packet[4+i] = data[i];
	}

	return (length + 4);
}

/*
*@param Packet to be created
*@param Control param
*@param Type of cont 1 param
*@param Length of cont 1 param
*@param Value of cont 1 param
*@param Type of cont 2 param
*@param Length of cont 2 param
*@param Value of cont 2 param
*@ret int -> Total packet size
*/

int createControlStartPacket(char* packet, unsigned char* filename, int size) {

	sprintf(&packet[3], "%d", size);
	packet[0] = P_CONTROL_START;
	packet[1] = P_T_SIZE;
	packet[2] = strlen(&packet[3]);
	packet[3 + (int) packet[2]] = P_T_NAME;
	packet[4 + (int) packet[2]] = strlen(filename);
	sprintf(&packet[5 + (int)packet[2]], "%s", filename);

	return 4 + (int)packet[2] + strlen(filename);
}

int createControlEndPacket(char* packet, unsigned char* hash) {
	packet[0] = P_CONTROL_END;
	packet[1] = P_T_SHA1;
	packet[2] = SHA_DIGEST_LENGTH;

	int i;
	for(i = 0; i < (int) SHA_DIGEST_LENGTH; i++)
        packet[i+3] = hash[i];

    return 3 + SHA_DIGEST_LENGTH;
}



void creatFrame(char* frame, unsigned char A, unsigned char C){
	frame[0] = FLAG;
	frame[1] = A;
	frame[2] = C;
	frame[3] = A^C;
	frame[4] = FLAG;
}
