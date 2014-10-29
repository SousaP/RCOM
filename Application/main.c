#include "types.h"
#include "appLayer.h"
#include "linkLayer.h"

int main(int argc, char *argv[])
{
    // ./rcom <type> <port> <tries> <timeout> <framesize> <filename>

    if(argc < 7) {
        printf("ARGUMENTS ERROR\n./rcom <type> <port> <tries> <timeout> <framesize> <filename>\n");
        return -1;
    }

    strcpy(linkLayer.port, argv[2]);
    linkLayer.numTransmissions = atoi(argv[3]);
    linkLayer.timeout = atoi(argv[4]);
    linkLayer.baudRate = 38400;

    if (strcmp(argv[1], "transmitter") == 0) {

        printf("<Transmitter>\n");


        if (argc == 7) {
            strcpy(appLayer.filename,argv[6]);
            appLayer.dataSize = atoi(argv[5]);
        } else if(argc != 7) {
            printf("Wrong Arguments\n");
            return -1;
        }
        appMode = TRANSMITTER;

      transmitter();

    } else if(strcmp(argv[1], "receiver") == 0) {

        if(argc != 7) {
            printf("Wrong Arguments\n");
            return -1;
        }
        printf("<Receiver>\n");
        appMode = RECEIVER;
        strcpy(appLayer.filename,argv[6]);
        appLayer.dataSize = atoi(argv[5]);
        receiver();

    } else {
        printf("Type should be 'receiver' or 'transmitter'\n");
        return -1;
    }

    llclose();

	return 0;
}
