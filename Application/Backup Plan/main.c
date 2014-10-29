#include "linkLayer.h"
#include "appLayer.h"
#include "types.h"

int main(int argc, char *argv[])
{
    // argv: tipo / port / tentativas / timeout / [ficheiro / framesize]

    if(argc < 5) {
        printf("ARGUMENTS ERROR\n./rcom type port tries timeout baundrate [file framesize]\n");
        return -1;
    }

    strcpy(linkData.port, argv[2]);
    linkData.numTransmissions = atoi(argv[3]);
    linkData.timeout = atoi(argv[4]);
    linkData.baudRate = 38400;


    if (strcmp(argv[1], "transmitter") == 0) {

        printf("[Transmitter]\n");


        if (argc == 7) {
            strcpy(appData.filename,argv[5]);
            appData.dataSize = atoi(argv[6]);
        } else if(argc != 6) {
            printf("ARGUMENTS ERROR\n./rcom type port tries timeout baundrate [file framesize]\n");
            return -1;
        }

        transmitter();

    } else if(strcmp(argv[1], "receiver") == 0) {

        if(argc < 7) {
            printf("ARGUMENTS ERROR\nfilename is required\n");
            return -1;
        }
        printf("[Receiver]\n");
        strcpy(appData.filename,argv[5]);
        if(argc > 7)
            appData.dataSize = atoi(argv[6]);
        receiver();

    } else {
        printf("ARGUMENTS ERROR\ntype should be 'receiver' or 'transmitter'\n");
        return -1;
    }

    llclose();

	return 0;
}
