#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>


#define BUF_SIZE 1000


char *host_name = "127.0.0.1"; /* local host */
int port = 8000;


int main(int argc, char *argv[]) 
{
	struct sockaddr_in serv_addr;
 	struct hostent* server;	
	int num, answer;
	
	if (argc < 2) { 
		printf("Usage: %s integer\n", argv[0]);
		exit(-1);
	}
	num = atoi(argv[1]);
	
	if ( ( server = gethostbyname(host_name) ) == 0 ) 
	{
		perror("Error resolving local host\n");
		exit(1);
	}

	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = ((struct in_addr *)(server->h_addr))->s_addr;
	serv_addr.sin_port = htons(port);
	
	


	
	int num2 = 0;
	/* This sends the string plus the string terminator '\0' */
	for(int i = 0; i<num;i++){
		int sockfd = socket( PF_INET, SOCK_STREAM, 0 );
		if ( sockfd == -1 ) 
		{
			perror("Error opening socket\n");
			exit(1);
		}    

		if ( connect(sockfd, (void*)&serv_addr, sizeof(serv_addr) ) == -1 ) 
		{
			perror("Error connecting to socket\n");
			exit(1);
		}
		//tempo + process id univoco
		srand(time(NULL) + getpid());
		num2 = rand();
		printf("Send the number %d\n", num2);
		if ( send(sockfd, &num2, sizeof(num2), 0) == -1 ) 
		{
			perror("Error on send\n");
			exit(1);
		}

		printf("waiting response \n");
		
		if ( recv(sockfd, &answer, sizeof(answer), 0) == -1 ) 
		{
			perror("Error in receiving response from server\n");
			exit(1);
		}

		printf("response: %d\n", answer);
		close(sockfd);
		sleep(5);
	}
	

	

	return 0;
}


