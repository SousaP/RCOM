#include "stuffing.h"


int stuffing(char* data, int size, char* stuffed){
    int i = 0;
    int j = 0;
    unsigned char byte;

    while(i < size){
        byte = data[i];

        if(i == 0) {
            stuffed[j] = byte;
            i++;
            j++;
            continue;
        }
        else if(i == size-1) {
            stuffed[j] = byte;
            j++;
            break;
        }

        if(byte == FLAG){
            stuffed[j] = ESCAPE;
            j++;
            stuffed[j] = FLAG_AUX;
            j++;
        }
        else if(byte == ESCAPE){
            stuffed[j] = ESCAPE;
            j++;
            stuffed[j] = ESCAPE_AUX;
            j++;
        }
        else{
            stuffed[j] = byte;
            j++;
        }
        i++;
    }

    return j;
}

int unstuffing(char* stuffed, int size, char* data){
    int i = 0;
    int j = 0;
    unsigned char byte;

    while(i < size){

        byte = stuffed[i];

        if(byte == ESCAPE){
            i++;
            byte = stuffed[i];
            if(byte == FLAG_AUX){
                data[j] = FLAG;
                j++;
            }
            else if(byte == ESCAPE_AUX){
                data[j] = ESCAPE;
                j++;
            }
            else{
                printf("Error unstuffing\n");
                return 0;
            }
        }
        else{
            data[j] = byte;
            j++;
        }
        i++;
    }
    return j;
}
