#include "types.h"
#include "appLayer.h"
#include "linkLayer.h"

int main(int argc, char *argv[])
{
    // argv: tipo / port / tentativas / timeou/ [ficheiro / framesize]

    if(argc < 6) {
        printf("ARGUMENTS ERROR\n./rcom <type> <port> <tries> <timeout> <framesize>\n");
        return -1;
    }

    strcpy(linkLayer.port, argv[2]);
    linkLayer.numTransmissions = atoi(argv[3]);
    linkLayer.timeout = atoi(argv[4]);
    linkLayer.baudRate = 38400;

    if (strcmp(argv[1], "transmitter") == 0) {

        printf("*Transmitter*\n");


        if (argc == 8) {
            strcpy(appData.filename,argv[6]);
            appData.dataSize = atoi(argv[7]);
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
        printf("*Receiver*\n");
        strcpy(appData.filename,argv[6]);
        if(argc > 7)
            appData.dataSize = atoi(argv[7]);
        receiver();

    } else {
        printf("ARGUMENTS ERROR\ntype should be 'receiver' or 'transmitter'\n");
        return -1;
    }

    llclose();

	return 0;
}
