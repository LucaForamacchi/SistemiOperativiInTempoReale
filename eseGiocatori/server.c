#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <time.h>
#include <unistd.h>
#include <netdb.h>

#include "player.h"
#include "list.h"
#define BUF_SIZE 1000
#define N 3

int port = 8000;



int main() 
{
	struct sockaddr_in serv_addr;
	struct sockaddr_in cli_addr;
	char *usage = "Welcome, in order to play you have to insert your name and surname, insert your Name:";
	char *n_recv = "Insert your Surname:";
	char *s_recv = "You're now ready to play, wait until other player join the match.";
	
	char *winMessage = "Congrats, you win";
        char *secondMessage = "Congrats, you arrive second";
	char *thirdMessage = "Congrats, you arrive third";
	
	char name[1024] = {0};
	char surname[1024] = {0};
	int player_count = 0;
	// Socket opening
	int sockfd = socket( PF_INET, SOCK_STREAM, 0 );  
	if ( sockfd == -1 ) 
	{
		perror("Error opening socket");
		exit(1);
	}
	
	int options = 1;
	if(setsockopt (sockfd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof (options)) < 0) {
		perror("Error on setsockopt");
		exit(1);
	}

	bzero( &serv_addr, sizeof(serv_addr) );
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port);

	// Address bindind to socket
	if ( bind( sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr) ) == -1 ) 
	{
		perror("Error on binding");
		exit(1);
	}
	
	// Maximum number of connection kept in the socket queue
	if ( listen( sockfd, 20 ) == -1 ) 
	{
		perror("Error on listen");
		exit(1);
	}


	socklen_t address_size = sizeof( cli_addr );	

	LIST players = NewList();
	LIST active_players = NewList();
	while(1) 
	{
		printf("\n---\npid %d: waiting connections\n", getpid());
		
		// New connection acceptance		
		int newsockfd = accept( sockfd, (struct sockaddr *)&cli_addr, &address_size );      
		if (newsockfd == -1) 
		{
			perror("Error on accept");
			exit(1);
		}
		
		
		
		  printf("pid %d child of %d: serving active connection\n", getpid(), getppid());
		  
		  //Usage
		  if (send(newsockfd, usage, strlen(usage), 0 ) == -1) {
		    perror("Error on send");
		    exit(1);
	          }
		  
		  // Name reception
		  if (recv(newsockfd, &name, sizeof(name), 0 ) == -1) {
		    perror("Error on receive name");
		    exit(1);
		  }
		  printf("Nome ricevuto: %s \n", name);
		  if (send(newsockfd, n_recv, strlen(n_recv), 0 ) == -1) {
		    perror("Error on send");
		    exit(1);
	          }
		  
		  // Surname reception
		  if (recv(newsockfd, &surname, sizeof(surname), 0 ) == -1) {
		    perror("Error on receive name");
		    exit(1);
		  }
		  printf("Cognome ricevuto: %s\n", surname);
		  if (send(newsockfd, s_recv, strlen(s_recv), 0 ) == -1) {
		    perror("Error on send");
		    exit(1);
	          }
	          
	          Player p = NewPlayer(name, surname, 0, 0, newsockfd);
	          printf("Profilo del giocatore creato:\n");
		  PrintPlayer(&p);
		  player_count++;
		  ItemType item;
		  item.player = p;
		  if(Find(players, item) == NULL){
		  	players = EnqueueLast(players, item);
		  	  
		  }
		  active_players = EnqueueLast(active_players, item);
		  printf("Lista giocatori:\n");
		  PrintList(&players);
		  printf("Lista giocatori attivi:\n");
		  PrintList(&active_players);
		  if (player_count == 3) {
		  printf("Partita iniziata\n");
		  
		    player_count = 0;
		    srand(time(NULL));
		    int randomn = (rand() % 3) + 1; 
		    
			ItemType first = getHead(active_players);
			active_players = DequeueFirst(active_players);

			ItemType second = getHead(active_players);
			active_players = DequeueFirst(active_players);

			ItemType third = getHead(active_players);
			active_players = DequeueFirst(active_players);

			// Cerca gli stessi giocatori nella lista players
			ItemType* winner = Find(players, first);
			ItemType* secondPlace = Find(players, second);
			ItemType* thirdPlace = Find(players, third);

			

			if (randomn == 1) {
			    winner = Find(players, first);
			    secondPlace = Find(players, second);
			    thirdPlace = Find(players, third);
			} else if (randomn == 2) {
			    winner = Find(players, second);
			    secondPlace = Find(players, third);
			    thirdPlace = Find(players, first);
			} else {
			    winner = Find(players, third);
			    secondPlace = Find(players, first);
			    thirdPlace = Find(players, second);
			}

			// Aggiorna il punteggio nei giocatori dentro players
			winner->player.score += 3;
			secondPlace->player.score += 2;
			thirdPlace->player.score += 1;
			winner->player.game += 1;
			secondPlace->player.game += 1;
			thirdPlace->player.game += 1;

		    send(winner->player.sockfd, winMessage, strlen(winMessage), 0);
		    send(secondPlace->player.sockfd, secondMessage, strlen(secondMessage), 0);
		    send(thirdPlace->player.sockfd, thirdMessage, strlen(thirdMessage), 0);
			sleep(1);
			
		    // Chiudi le connessioni
		    close(winner->player.sockfd);
		    close(secondPlace->player.sockfd);
		    close(thirdPlace->player.sockfd);

		    
		}

		
	}
	return 0;
}



