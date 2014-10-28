llopen(){
    
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

}