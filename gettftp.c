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

int getaddrinfo(const char *node, const char *service,
	const struct addrinfo *hints, struct addrinfo **res);


int main(int argc, char **argv){
	struct addrinfo hints;
	struct addrinfo *result, *rp;
	char * service=NULL;
	int s;
	char * retrievedHost = malloc(ADDRSIZE*sizeof(char)); //buffer used to store the address
	char * retrievedService = malloc(ADDRSIZE*sizeof(char)); //buffer used to store the address
	memset(&hints, 0, sizeof(struct addrinfo));
	int sock;
	
	//If there is no the two expected parameters: we stop the program
	if(argc != 3){
		fprintf(stderr, "Usage : %s host file\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	
	//hints.ai_protocol=6; //we choose UDP for resolving the address
	s = getaddrinfo(argv[1], "1069", &hints, &result);
	
	//check if an error occured when getting the address
	if(s != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
		exit(EXIT_FAILURE);
	}
	
	//for (rp=result; rp->ai_next==NULL; rp=rp->ai_next){} //to take the last pointer if wanted
	//rp = result;result->ai_addrlen
	
	s = getnameinfo(result->ai_addr, result->ai_addrlen, 
		retrievedHost, ADDRSIZE,
		retrievedService, ADDRSIZE,
		NI_NUMERICHOST | NI_NUMERICSERV);
	
	//Q3 creation of a connection socket to the server
	if((sock = socket(AF_INET, SOCK_DGRAM, IPROTO_UDP)) < 0 ){
		perror("socket error");
		exit(EXIT_FAILURE);
	}
	
	
	
	
	exit(EXIT_SUCCESS);
}

//will be used for puttftp bind(sock, result->ai_addr, result->ai_addrlen);
