#include "types.h"
#include "app.h"
#include "link.h"

void testStuffing();

int main(int argc, char *argv[]) 
{
    // argv: tipo / port / tentativas / timeout / baundrate / [ficheiro / framesize]

    if(argc < 6) {
        printf("ARGUMENTS ERROR\n./rcom type port tries timeout baundrate [file framesize]\n");
        return -1;        
    }

    strcpy(linkData.port, argv[2]);
    linkData.numTransmissions = atoi(argv[3]);
    linkData.timeout = atoi(argv[4]);
    linkData.baudRate = atoi(argv[5]);

    int found=1;
    while(found==1)
    {
        switch(linkData.baudRate)
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
                scanf("%d",&linkData.baudRate);
                break;
            }

        }
    }
   


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

void testStuffing() {
    char test1[6];
    int i;
    test1[0] = 'A';
    test1[1] = 'B';
    test1[2] = LFC_FLAG;
    test1[3] = 0x7D;
    test1[4] = 'D';
    test1[5] = '\0';

    printf("Before stuffing:\n");
    for(i = 0; i < strlen(test1); i++)
    {
            printf("%x\n", test1[i]);
    }

    char stuffed[8];
    if(stuffing(test1, 6, stuffed))
        printf("Stuffing completed!\n");

    printf("After stuffing:\n");
    for(i = 0; i < 8; i++)
    {
            printf("%x\n", stuffed[i]);
    }

    if(unstuffing(stuffed, 8, test1))
    {
        printf("Unstuffing success!\n");
    }

    for(i = 0; i < 6; i++)
    {
            printf("%x\n", test1[i]);
    }

}