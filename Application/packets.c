#include "packets.h"

/*
*@param Packet to be created
*@param Sequence number
*@param Length of packet data
*@param Packet data
*@ret int -> Total packet size
*/
int createDataPacket(char* packet, int seqNumber, int length, char* data) {
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

int createControlStartPacket(char* packet, char* filename, char size) {

	sprintf(&packet[3], "%d", size);
	packet[0] = P_CONTROL_START;
	packet[1] = P_T_SIZE;
	packet[2] = strlen(packet[3]);
	packet[3 + (int) packet[2]] = P_T_NAME;
	packet[4 + (int) packet[2]] = strlen(filename);
	sprintf(&packet[5 + (int)packet[2]], "%d", filename);

	return 4 + (int)packet[2] + strlen(filename);
}

int createControlEndPacket(char* packet, char* hash) {
	packet[0] = P_CONTROL_END;
	packet[1] = P_T_SHA1;
	packet[2] = strlen(hash);

	int i;
	for(i = 0; i < (int) packet[2]; i++) {
		packet[i+3] = hash[i];
	}

	return 3 + (int) packet[2];
}

/*
*@param Packet to be processed
*@param Sequence number
*@param Length of packet data
*@param Where data is going to be stored
*@ret int -> Total data size
*/
int processDataPacket(char* packet, int seqNumber, int length, char* data) {
	if(packet[0] != 0x01) {
		printf("Error. Not a data packet.");
		return -1;
	}

	*seqNumber = (char) packet[1];
	unsigned int numOct = 256* packet[2] + packet[3];

	if(numOct != length - 4) {
		printf("Invalid size");
		return -2;
	}

	int i;
	int dataSize = 0;

	for(i = 4; i < length && dataSize < numOct; i++, dataSize++) {
		data[dataSize] = packet[i];
	}

	return dataSize;
}

/*
*@param Packet to be processed
*@param Packet size
*@param Control param
*@param Type of cont 1 param
*@param Length of cont 1 param
*@param Value of cont 1 param
*@param Type of cont 2 param
*@param Length of cont 2 param
*@param Value of cont 2 param
*@ret int -> 0 success
			-1 error
*/
int processControlPacket(char* packet, int size, char control, char T1, char L1, char* V1, char T2, char L2, char* V2) {
	*control = packet[0];
	*T1 = packet[1];
	*T2 = packet[2];

	int i;
	int pSize = 0;
	for(i = 3; i < L1; i++, pSize++) {
		V1[pSize] = packet[i];
	}

	*T2 = packet[pSize];
	*L2 = packet[pSize + 1];

	pSize = pSize + 2;
	for(i = 0; i < L2; i++, pSize++) {
		V2[pSize] = packet[i];
	}

	if(pSize != size)
		return -1;
	return 0;
}