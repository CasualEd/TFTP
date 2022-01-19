#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#define BUFSIZE 1024
#define ADDRSIZE 100
#define BLOCKLENGTH 512
#define ACKSIZE 4
#define MAXBLOCKINDEX 65535 //2^(16)-1

int getaddrinfo(const char *node, const char *service,
	const struct addrinfo *hints, struct addrinfo **res);

void testArgs(int argc, char ** argv){
		if(argc != 3){//If there is no the two expected parameters: we stop the program
		fprintf(stderr, "Usage : %s host file\n", argv[0]);
		exit(EXIT_FAILURE);
	}
}

void sendRRQ(char ** argv, int sock, struct addrinfo * result){
	int lentot = strnlen(argv[2], BUFSIZE)+9; //size of the RRQ
	int len;
	char * Tpacket = malloc(lentot*sizeof(char));
	//Opcode
	*((short*) Tpacket) = htons(1);
	//Filename
	Tpacket = (char*) Tpacket;
	strncpy(Tpacket+2, argv[2], BUFSIZE);
	//mode
	len = strnlen(argv[2], BUFSIZE)+3; //2(line64)+1=3 to let one byte equal to 0x0
	strncpy(Tpacket+len, "octet", 6);
	sendto(sock, Tpacket, lentot, 0, result->ai_addr, result->ai_addrlen); //send the file retrieving request
	free(Tpacket);
}

void recvData(int sock, struct addrinfo * result, char * ACKpacket, char * fileData, char * SbytesR){
	int len;
	int blockIndex=0;
	int recvBlockSize = BLOCKLENGTH;
	
	while((recvBlockSize==BLOCKLENGTH) && (blockIndex < MAXBLOCKINDEX)){
		recvBlockSize=recvfrom(sock, fileData, BLOCKLENGTH, 0,
			result->ai_addr,&(result->ai_addrlen));
		blockIndex++;
		*((short*) ACKpacket+1) = htons(blockIndex);
		sendto(sock, ACKpacket, ACKSIZE, 0,
			result->ai_addr, result->ai_addrlen); //send the ACK
		write(STDOUT_FILENO, fileData, BLOCKLENGTH);
	}
		
	recvBlockSize=(blockIndex-1)*BLOCKLENGTH+recvBlockSize;
	sprintf(SbytesR, "%d", recvBlockSize);
	len = strnlen("\nBytes received : ", BUFSIZE);
	write(STDOUT_FILENO, "\nBytes received : ", len);
	len = strnlen(SbytesR, BUFSIZE);
	write(STDOUT_FILENO, SbytesR, len);
	len = strnlen("\n", BUFSIZE);
	write(STDOUT_FILENO, "\n", len);
	
	free(fileData);
	free(SbytesR);
}

int main(int argc, char **argv){
	struct addrinfo hints;
	struct addrinfo *result;
	int s;
	char * retrievedHost = malloc(ADDRSIZE*sizeof(char)); //buffer used to store the address
	char * retrievedService = malloc(ADDRSIZE*sizeof(char)); //buffer used to store the service port 
	memset(&hints, 0, sizeof(struct addrinfo));
	int sock;
	void * ACKpacket = malloc(ACKSIZE*sizeof(char));
	char * fileData = malloc(BLOCKLENGTH*sizeof(char));
	char * SbytesR = malloc(BLOCKLENGTH*sizeof(char));
	
	
	testArgs(argc, argv);
	
	hints.ai_protocol=IPPROTO_UDP;//UDP
	s = getaddrinfo(argv[1], "1069", &hints, &result); //retrieve connection information about the entered url and port n°.
	if(s != 0){//check if an error occured when getting the address
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}
	
	s = getnameinfo(result->ai_addr, result->ai_addrlen, 
		retrievedHost, ADDRSIZE,
		retrievedService, ADDRSIZE,
		NI_NUMERICHOST | NI_NUMERICSERV);
	
	//Q3 creation of a connection socket to the server
	if((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ){
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	
	//Q4
	sendRRQ(argv, sock, result);
	//Building the basic ACK packet
	*((short*) ACKpacket) = htons(4);
	
	recvData(sock, result, ACKpacket, fileData, SbytesR);
	
	exit(EXIT_SUCCESS);
}
//will be used for puttftp bind(sock, result->ai_addr, result->ai_addrlen);
