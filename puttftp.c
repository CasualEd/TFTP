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

void printString(char * string){
	int lgthString=strlen(string); //we retrieve the length of the string to write it on the standard output
	write(STDOUT_FILENO, string, lgthString);
}

int main(int argc, char **argv){
	struct addrinfo hints;
	struct addrinfo *result;
	int s;
	char * retrievedHost = malloc(ADDRSIZE*sizeof(char)); //buffer used to store the address
	char * retrievedService = malloc(ADDRSIZE*sizeof(char)); //buffer used to store the service port 
	memset(&hints, 0, sizeof(struct addrinfo));
	int sock;
	int lentot;
	int len;
	char * recvbuf = malloc(BLOCKLENGTH*sizeof(char));
	int sendBlockSize = BLOCKLENGTH;
	int recvBlockSize = BLOCKLENGTH;
	void * ACKpacket = malloc(ACKSIZE*sizeof(char));
	void * DATApacket = malloc(BLOCKLENGTH*sizeof(char));
	char * SbytesR = malloc(BLOCKLENGTH*sizeof(char));
	
	
	if(argc != 3){//If there is no the two expected parameters: we stop the program
		fprintf(stderr, "Usage : %s host file\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
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
	lentot = strnlen(argv[2], BUFSIZE)+9; //size of the RRQ
	char * Tpacket = malloc(lentot*sizeof(char));
	//Opcode
	*((short*) Tpacket) = htons(2); //2 for write request
	//Filename
	Tpacket = (char*) Tpacket;
	strncpy(Tpacket+2, argv[2], BUFSIZE);
	//mode
	len = strnlen(argv[2], BUFSIZE)+3; //2(line64)+1=3 to let one byte equal to 0x0
	strncpy(Tpacket+len, "octet", 6);
	
	//test if socket is disponible
	sendto(sock, Tpacket, lentot, 0, result->ai_addr, result->ai_addrlen); //send the file putting request
	
	//Building the basic ACK packet
	*((short*) ACKpacket) = htons(4);
	
	
	int blockIndex=0;
	//Building the basic DATA packet
	*((short*) DATApacket) = htons(3);
	*((short*) DATApacket+1) = htons(0);
	
	
	while((sendBlockSize==BLOCKLENGTH) && (blockIndex < MAXBLOCKINDEX)){
		sendBlockSize=strnlen(argv[2], BUFSIZE)+2; //arbitraire pour commencer
		*((short*) ACKpacket+1) = htons(blockIndex); //
		recvBlockSize=recvfrom(sock, recvbuf, BLOCKLENGTH, 0,
			result->ai_addr,&(result->ai_addrlen));
		if(recvBlockSize==4){
			if(*((short*) recvbuf)==htons(4)){
				printString("It is an ACK packet sending next packet!");
			}else{
				printString("This is not an ACK packet, exit!");
			exit(EXIT_FAILURE);
			}
		}else{
			printString("This is not an ACK packet, exit!");
			exit(EXIT_FAILURE);
		}
		strncpy(DATApacket+2, argv[2], BUFSIZE);
		sendto(sock, DATApacket, sendBlockSize, 0,
			result->ai_addr, result->ai_addrlen); //send the ACK
		//write(STDOUT_FILENO, fileData, BLOCKLENGTH);
		blockIndex++;
	}
	
	exit(EXIT_SUCCESS);
}
//will be used for puttftp bind(sock, result->ai_addr, result->ai_addrlen);
