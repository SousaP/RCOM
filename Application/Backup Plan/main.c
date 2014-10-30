#include "linkLayer.h"
#include "appLayer.h"
#include "types.h"

int main(int argc, char *argv[]) 
{
    // argv: tipo / port / tentativas / timeout / baundrate / [ficheiro / framesize]

    if(argc < 6) {
        printf("ARGUMENTS ERROR\n./rcom type port tries timeout baundrate [file framesize]\n");
        return -1;        
    }

    strcpy(lLayer.port, argv[2]);
    lLayer.numTransmissions = atoi(argv[3]);
    lLayer.timeout = atoi(argv[4]);
    lLayer.baudRate = atoi(argv[5]);

    int found=1;
    while(found==1)
    {
        switch(lLayer.baudRate)
        {
            case 2400:found=0;break;
            case 4800:found=0;break;
            case 9600:found=0;break;
            case 19200:found=0;break;
            case 38400:found=0;break;
            case 57600:found=0;break;
            case 115200:found=0;break;
            case 31250:found=0;break;
            default:
            {
                printf("Error baundrate must be (2400,4800,9600,19200,38400,57600,115200 or 31250)\n");
                printf("New baundrate: ");
                scanf("%d",&lLayer.baudRate);
                break;
            }

        }
    }

    if (strcmp(argv[1], "send") == 0) {
        system("clear");
        printf("Sender machine!\n");
        
   
        if (argc == 8) {
            strcpy(appLayer.filename,argv[6]);
            appLayer.dataSize = atoi(argv[7]);
            if(appLayer.dataSize > MAX_FRAME_SIZE){
            printf("Frame size too big!\n");
            return -1;
        }
        } else if(argc != 6) {
            printf("ARGUMENTS ERROR\n./rcom type port tries timeout baundrate [file framesize]\n");
            return -1; 
        }

        transmitter();

    } else if(strcmp(argv[1], "receive") == 0) {

        if(argc < 7) {
            printf("ARGUMENTS ERROR\nfilename is required\n");
            return -1; 
        }
        system("clear");
        printf("Receive machine!\n");
        strcpy(appLayer.filename,argv[6]);
        if(argc > 7)
            appLayer.dataSize = atoi(argv[7]);
        if(appLayer.dataSize > MAX_FRAME_SIZE){
            printf("Frame size too big!\n");
            return -1;
        }
        receiver();

    } else {
        printf("ARGUMENTS ERROR\ntype should be 'receive' or 'send\n");
        return -1; 
    }

    llclose();

    return 0;
}
