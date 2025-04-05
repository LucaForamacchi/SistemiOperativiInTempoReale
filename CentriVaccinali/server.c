#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include "player.h"
#include "list.h"

#define BUF_SIZE 1000
#define PORT 8000

typedef struct {
    char name[1024];
    int quantity;
    int min_quantity;
} Fornitore;

typedef struct {
    char name[1024];
    int request_quantity;
} CentroVaccinale;

int main() {
    struct sockaddr_in serv_addr;
    struct sockaddr_in cli_addr;
    char *provider_welcome = "Benvenuto fornitore. I tuoi dati sono stati registrati.";
    char *center_welcome = "Benvenuto centro vaccinale. La tua richiesta è stata registrata.";
    char *distribution_msg = "Distribuzione vaccini iniziata.";
    char *waiting_msg = "In attesa di altre richieste o fornitori.";

    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("Errore apertura socket");
        exit(1);
    }

    int options = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &options, sizeof(options));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("Errore binding");
        exit(1);
    }

    if (listen(sockfd, 20) == -1) {
        perror("Errore listen");
        exit(1);
    }

    socklen_t address_size = sizeof(cli_addr);
    LIST providers = NewList();
    LIST vaccine_centers = NewList();
    int total_vaccines = 0;
    int total_requests = 0;

    while (1) {
        printf("\n---\npid %d: in attesa di connessioni\n", getpid());
        int newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &address_size);
        if (newsockfd == -1) {
            perror("Errore accept");
            continue;
        }

        char buffer[sizeof(Fornitore)];
        memset(buffer, 0, sizeof(buffer));
        ssize_t bytes_received = recv(newsockfd, buffer, sizeof(buffer), 0);
        if (bytes_received == -1) {
            perror("Errore ricezione dati");
            close(newsockfd);
            continue;
        }

        ItemType item;
        memset(&item, 0, sizeof(ItemType));

        if (bytes_received == sizeof(Fornitore)) {
            Fornitore fornitore;
            memcpy(&fornitore, buffer, sizeof(Fornitore));
            printf("Fornitore registrato: %s, Quantità: %d, Minimo: %d\n", fornitore.name, fornitore.quantity, fornitore.min_quantity);

            strcpy(item.name, fornitore.name);
            item.quantity = fornitore.quantity;
            item.min_quantity = fornitore.min_quantity;
	    item.sockfd = newsockfd;
            providers = EnqueueFirst(providers, item);
            send(newsockfd, provider_welcome, strlen(provider_welcome), 0);
            total_vaccines += fornitore.quantity;
            memset(&item, 0, sizeof(ItemType));
            
        } else if (bytes_received == sizeof(CentroVaccinale)) {
            CentroVaccinale center;
            memcpy(&center, buffer, sizeof(CentroVaccinale));
            printf("Centro vaccinale registrato: %s, Richiesta: %d\n", center.name, center.request_quantity);

            strcpy(item.name, center.name);
            item.quantity = center.request_quantity;
            item.min_quantity = 0;
	    item.sockfd = newsockfd;
	    
            vaccine_centers = EnqueueLast(vaccine_centers, item);
            send(newsockfd, center_welcome, strlen(center_welcome), 0);
            total_requests += center.request_quantity;
            memset(&item, 0, sizeof(ItemType));
        } else {
            printf("Ricevuti dati di lunghezza inattesa: %zd byte\n", bytes_received);
            close(newsockfd);
            continue;
        }

        int len = getLength(vaccine_centers);
        int len2 = getLength(providers);
            if(len > 0 && len2 > 0){
            	    LIST ordered_centers = NewList();
		    LIST tmp1 = vaccine_centers;
		    while (tmp1 != NULL) {
			LIST max_node = tmp1;
			LIST tmp2 = tmp1->next;

			while (tmp2 != NULL) {
			    if (tmp2->item.quantity > max_node->item.quantity) {
				max_node = tmp2;
			    }
			    tmp2 = tmp2->next;
			}
			ordered_centers = EnqueueLast(ordered_centers, max_node->item);
			tmp1 = tmp1->next;
		    }
		    tmp1 = ordered_centers;
		    LIST tmp2 = providers;
		    LIST fornitori = NewList();
		    LIST riceventi = NewList();
		    while(tmp1 != NULL){
		    	while(tmp2 != NULL){
		    		if(tmp1->item.quantity >= tmp2->item.min_quantity){
		    			tmp1->item.quantity -= tmp2->item.min_quantity;
		    			tmp2->item.quantity -= tmp2->item.min_quantity;
		    			if(Find(fornitori, tmp2->item) == NULL){	
		    				fornitori = EnqueueLast(fornitori, tmp2->item);
		    			}
		    			if(Find(riceventi, tmp1->item) == NULL){	
		    				riceventi = EnqueueLast(riceventi, tmp1->item);
		    			}
		    			
		    		}
		    		
		    		if(tmp1->item.quantity == 0){
		    			int len1 = getLength(fornitori);
		    			//printf("Lunghezza riceventi: %d\n nome: %s\n", len2, fornitori->item.name);
		    			send(tmp1->item.sockfd, &len1, sizeof(int), 0);
		    			while(fornitori != NULL){
		    				send(tmp1->item.sockfd, fornitori->item.name, strlen(fornitori->item.name), 0);
		    				
		    				fornitori = fornitori->next;
		    			}
		    			tmp1 = Dequeue(tmp1, tmp1->item);
		    			close(tmp1->item.sockfd);
		    		}
		    		if(tmp2->item.quantity == 0){
		    			int len2 = getLength(riceventi);
		    			//printf("Lunghezza riceventi: %d\n nome: %s\n", len2, riceventi->item.name);
		    			send(tmp2->item.sockfd, &len2, sizeof(int), 0);
		    			while(riceventi != NULL){
		    				send(tmp2->item.sockfd, riceventi->item.name, strlen(riceventi->item.name), 0);
		    				
		    				riceventi = riceventi->next;
		    			}
		    			tmp2 = Dequeue(tmp2, tmp2->item);
		    			close(tmp2->item.sockfd);
		    		}
		    		
		    		if(tmp1->item.quantity < tmp2->item.min_quantity){
		    			tmp2 = tmp2->next;
		    		}
		    	}
		    	
		    	tmp1 = tmp1->next;
		    }
		    
		    
		    
            }
    }
    return 0;
}

