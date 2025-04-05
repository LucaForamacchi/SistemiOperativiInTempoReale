#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <netdb.h>
#include "player.h"

#define BUF_SIZE 1000


char *host_name = "127.0.0.1"; /* local host */
int port = 8000;

typedef struct {
    char name[1024];
    int quantity;
} CentroVaccinale;

int main(int argc, char *argv[]) 
{
	struct sockaddr_in serv_addr;
 	struct hostent* server;	
	int q;
	int min;
	char answer[1024] = {0};
	char name[1024] = {0};
	char* send_s;
	size_t len = 0;
	//Invia il proprio nome (una stringa che si suppone 
	//univoca), la quantità di vaccini
	if (argc < 3) { 
		printf("Usage: %s integer\n", argv[0]);
		exit(-1);
	}
	CentroVaccinale data;
	memset(&data, 0, sizeof(data));
	strncpy(data.name, argv[1], sizeof(data.name) - 1);
	data.quantity = atoi(argv[2]);
	if ( ( server = gethostbyname(host_name) ) == 0 ) 
	{
		perror("Error resolving local host\n");
		exit(1);
	}
	bzero(&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = ((struct in_addr *)(server->h_addr))->s_addr;
	serv_addr.sin_port = htons(port);
	
	


	
	//int num2 = 0;
	/* This sends the string plus the string terminator '\0' */
	//for(int i = 0; i<num;i++){
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
	
	if (send(sockfd, &data, sizeof(data), 0) == -1) 
        {
                perror("Error sending data to server\n");
                exit(1);
        }
	
	if ( recv(sockfd, &answer, sizeof(answer), 0) == -1 ) 
	{
		perror("Error in receiving response from server\n");
		exit(1);
	}
	printf("%s \n",answer);
	memset(answer, 0, sizeof(answer));
	
	int len1;
	recv(sockfd, &len1, sizeof(int), 0);
	
	//printf("Lunghezza: %d\n socket: %d\n", len1, sockfd);
	for (int i = 0; i < len1; i++) {
	    char nomeFornitore[1024] = {0};  
	    recv(sockfd, &nomeFornitore, sizeof(nomeFornitore), 0);
	    printf("Vaccini ricevuti da: %s\n", nomeFornitore);
	    if(i==len1-1){
	    	close(sockfd);
	    }
	}
	
	
	
	
	//sleep(5);
	//}
	

	

	return 0;
}



