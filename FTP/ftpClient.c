#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <netdb.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>

#define FTP_PORT 21
#define MAXLENGTH 1024

char user[MAXLENGTH];
char password[MAXLENGTH];
char host[MAXLENGTH];
char path[MAXLENGTH];
char* ipAddress;

int sockfd;
int datasockfd;
int portConnection;

//Checking Adress format
int checkAddress(char* address)
{
	char checkFTP[7];
	bzero(checkFTP, 7);
	strncpy(checkFTP, address, 6);
	
	//Checks if ftp:// is at the beginning of the address
	if(strcmp(checkFTP, "ftp://") != 0)
	{
		printf("Missing ftp:// at the beginning of the address!\n");
		exit(1);
	}
	
	//Checks if adress is complete
	if (strlen(address) == 6)
	{
		printf("Address too short!\n");
		exit(1);
	}
	
	//Pointing to position after username
	char * separatorUser = strchr(address + 6, ':');
	char * separatorAt;
	char * separatorPath;
	

	if(separatorUser != NULL)
	{
		//Checks for @ separator after pass
		separatorAt = strchr(separatorUser, '@');
		if(separatorAt == NULL)
		{
			printf("Missing separator @ after user:password!\n");
			exit(1);
		}
		
		//Checks for / before url-path
		separatorPath = strchr(separatorAt, '/');
		if(separatorPath == NULL)
		{
			printf("Missing separator between host and path!\n");
			exit(1);
		}
		
		//Calculate adress elements length
		char userLength = separatorUser - address - 6;
		char passwordLength = separatorAt - separatorUser - 1;
		char hostLength = separatorPath - separatorAt - 1;
		char pathLength = address + strlen(address) - separatorPath - 1;
		
		//Check if element is null(inexistent)
		if(userLength == 0)
		{
			printf("User not found\n");
			exit(1);
		}
		if(passwordLength == 0)
		{
			printf("Password not found\n");
			exit(1);
		}
		if(hostLength == 0)
		{
			printf("Host not found\n");
			exit(1);
		}
		if(pathLength == 0)
		{
			printf("Path not found\n");
			exit(1);
		}
		
		//Save global variables
		strncpy(user, address+6, userLength);
		strncpy(password, separatorUser + 1, passwordLength);
		strncpy(host, separatorAt + 1, hostLength);
		strcpy(path, separatorPath + 1);
	}
	
	
	else if(separatorUser == NULL)
	{
		separatorAt = strchr(address, '@');
		if(separatorAt != NULL)
		{
			printf("There is no user and password!\n");
			exit(1);
			
		}
		
		separatorPath = strchr(address + 6, '/');
		if(separatorPath == NULL)
		{
			printf("Missing separator between host and path!\n");
			exit(1);
		}
		
		char hostLength = separatorPath - address - 6;
		char pathLength = address + strlen(address) - separatorPath - 1;
		if(hostLength == 0)
		{
			printf("Host not found\n");
			exit(1);
		}
		
		if(pathLength == 0)
		{
			printf("Path not found\n");
			exit(1);
		}
		
		//Save global variables
		strncpy(host, address + 6, hostLength);
		strcpy(path, separatorPath + 1);
	}
		
	return 0;
}

int getMessage(char * message)
{
	bzero(message, MAXLENGTH);
	
	char buffer[MAXLENGTH];
	char trash[MAXLENGTH];
	bzero(buffer, MAXLENGTH);
	bzero(trash, MAXLENGTH);
	
    //sleep(1);
    
	int totalRead = recv(sockfd, buffer, MAXLENGTH-1, 0);
	//printf("buffer %s\n", buffer);
	int i = 0;
    
    sleep(1);
	
	//Check if exists trash in socket connection
	while(totalRead == MAXLENGTH - 1)
	{
		totalRead = recv(sockfd, trash, MAXLENGTH-1, 0);
        sleep(1);
	}
	
	if(totalRead <= 0)
    {
        shutdown(sockfd,SHUT_RDWR);
        printf("\nEmpty Message Received!!!\n");
        close(sockfd);
        exit(1);
    }
	
	for(i = 0; i < MAXLENGTH; i++)
	{
		if(buffer[i] < '0' || buffer[i] > '9')
			break;
	}
	
	char tempNum[5];
	bzero(tempNum,5);
	strncpy(tempNum, buffer, i); 
	
	int status = atoi(tempNum);
	
	buffer[totalRead] = '\0';
	
	//Copies to adress passed in parameters
	strcpy(message, buffer + i);
	
	return status;
}

void startConnection()
{
	//getip.c founded this
	struct hostent *h;
	struct sockaddr_in server_addr;
	
	if ((h=gethostbyname(host)) == NULL) {  
		printf("Error while getting ip address!\n");
		exit(1);
	}

	printf("Host name  : %s\n", h->h_name);
	
	ipAddress = inet_ntoa(*((struct in_addr *)h->h_addr));
	printf("IP Address : %s\n", ipAddress);
	
	//clientTCP.c founded this
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ipAddress);
	server_addr.sin_port = htons(FTP_PORT);
	
	/*open an TCP socket*/
	if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		exit(1);
    }

	/*connect to the server*/
	if(connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("connect()");
		exit(1);
	}
	
	char message[MAXLENGTH];
	
	int state = getMessage(message);
	
	//Service ready for new user -> 220 Code
	if(state != 220)
	{
		printf("Error while getting first message!\n");
        shutdown(sockfd,SHUT_RDWR);
        close(sockfd);
		exit(1);
	}
}

void sendStartData()
{
	char message[MAXLENGTH];
	char buffer[MAXLENGTH];
	int written;
	int state;
	int j = 0;
	
	//Sending User
	bzero(message, MAXLENGTH);
		
	//In case of not existing username uses default anonymous
	if(strlen(user) == 0)
	{
		strcpy(user, "anonymous");
	}
    
    printf("user: %s\n", user);
    
	strcpy(message, "USER ");
    strcat(message, user);
    strcat(message, "\r\n");
		
    printf("message user %s\n", message);
		
	written = send(sockfd, message, strlen(message), 0);
	
	if(written <= 0)
	{
		printf("Error when writing user to socket\n");
        shutdown(sockfd,SHUT_RDWR);
        close(sockfd);
		exit(1);
	}
	
	bzero(buffer, MAXLENGTH);

	state = getMessage(buffer);
	
	if(state >= 400)
	{
		printf("\nError Occured, Error Code: %d\n", state);
        shutdown(sockfd,SHUT_RDWR);
        close(sockfd);
		exit(1);
	}

	bzero(message, MAXLENGTH);
	
	if(strlen(password) == 0)
	{
		strcpy(password, "123");
	}
	
	strcpy(message, "PASS ");
	strcat(message, password);
	strcat(message, "\r\n");

		written = send(sockfd, message, strlen(message), 0);
		
		if(written <= 0)
		{
			printf("Error when writing pass to socket\n");
			shutdown(sockfd,SHUT_RDWR);
			close(sockfd);
			exit(1);
		}
		
		bzero(buffer, MAXLENGTH);
	
		state = getMessage(buffer);
		
		if(state >= 400)
		{
			printf("\nError Occured, Error Code: %d\n", state);
			shutdown(sockfd,SHUT_RDWR);
			close(sockfd);
			exit(1);
		}
	
	bzero(buffer, MAXLENGTH);
	bzero(message, MAXLENGTH);
	
	//Start passive mode
	strcpy(message, "PASV\r\n");
	send(sockfd, message, strlen(message), 0);
	
	state = getMessage(buffer);
	
	if(state >= 400)
	{
		printf("Error while getting password authentication message: Error Code: %d\n", state);
        shutdown(sockfd,SHUT_RDWR);
        close(sockfd);
		exit(1);
	}
	
	//Get two last numbers from return message
	int port5, port6;
	
	char * commaPort6 = strrchr(buffer, ',');
	
	if(commaPort6 == NULL)
	{
		printf("ERROR when getting the comma on right\n");
		exit(1);
	}
	
	commaPort6[0] = '\0';
	char * commaPort5 = strrchr(buffer, ',');
	
	if(commaPort5 == NULL)
	{
		printf("ERROR when getting the comma on left\n");
		exit(1);
	}
	
	commaPort5[0] = '\0';
	commaPort6++;
	
	port5 = atoi(commaPort5 + 1);
	port6 = atoi(commaPort6);
	
	portConnection = port5*256 + port6;

	struct	sockaddr_in server_addr;
	
	//clientTCP.c founded this
	bzero((char*)&server_addr,sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(ipAddress);
	server_addr.sin_port = htons(portConnection);
	
	if ((datasockfd = socket(AF_INET,SOCK_STREAM,0)) < 0) {
		perror("socket()");
		exit(1);
    }

	/*connect to the server*/
	if(connect(datasockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
		perror("connect()");
		exit(1);
	}
}

int receiveData(){
   
	   int state;
	   char message[MAXLENGTH];
	   char buffer[MAXLENGTH];
	   
	   bzero(message, MAXLENGTH);
	   bzero(buffer, MAXLENGTH);
	   
	   strcpy(message, "RETR /");
	   strcat(message, path);
	   strcat(message, "\r\n");
	   
	   send(sockfd, message, strlen(message), 0);
	   
	   state = getMessage(buffer);
   
	if(state >= 400)
	{
		printf("Error while getting retrieve message: Error Code: %d\n", state);
		shutdown(sockfd,SHUT_RDWR);
		close(sockfd);
		shutdown(datasockfd,SHUT_RDWR);
		close(datasockfd);
		exit(1);
	}
	
	// Abrir ficheiro aqui
   
	int fd;
    int readchars = 1;
    
    bzero(buffer, MAXLENGTH);
    
    char * filename = strrchr(path, '/') + 1;
    
    printf("filename: %s\n", filename);
    
    //If exists, gets deleted
    unlink(filename);
    
    fd = open(filename, O_WRONLY | O_CREAT, 0777);

    if (fd < 0){
	printf("\nCannot open file!\n");
        shutdown(sockfd,SHUT_RDWR);
        close(sockfd);
        shutdown(datasockfd,SHUT_RDWR);
        close(datasockfd);
        exit(1);
    }

    printf("Progress:\n");
    while(readchars>0)
	{
        bzero(buffer, MAXLENGTH);
        
        readchars=recv(datasockfd, buffer, MAXLENGTH, 0);
       	printf(":");
		fflush(stdout);
		if(readchars > 0)
			write(fd, buffer, readchars);
	}
    
    close(fd);
    printf("\n");
    bzero(buffer, MAXLENGTH);

    sleep(3);
    
    recv(sockfd, buffer, MAXLENGTH, MSG_DONTWAIT);
    
    return 0;
}

int main(int argc, char *argv[])
{
	bzero(user, MAXLENGTH);
	
	if(argc != 2)
	{
		printf("usage: ftpClient ftp://[<user>:<password>@]<host>/<url-path>\n");
		exit(1);
	}
	
	checkAddress(argv[1]);
	
	if(strlen(user) != 0){
        printf("%c[%d;%dmUSER: %c[%dm",27,1,35,27,0);
        printf("%s\n", user);

    }
			
	if(strlen(host) != 0){
        printf("%c[%d;%dmHOST: %c[%dm",27,1,35,27,0);
        printf("%s\n", host);
    }
		
	if(strlen(path) != 0){
        printf("%c[%d;%dmPATH: %c[%dm",27,1,35,27,0);
		printf("%s\n", path);
    }
	
	startConnection();
	printf("\nConection Started!!!\n");

	sendStartData();
	printf("\nStart Data Sent!!!\n");

	receiveData();
	printf("\nData Received!!!\n");

	printf("%c[%d;%dmALL OK!!%c[%dm\n",27,1,32,27,0);

	shutdown(sockfd,SHUT_RDWR);
	close(sockfd);
	shutdown(datasockfd,SHUT_RDWR);
	close(datasockfd);
	
	return 0;
}
