#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "player.h"

/************* Funzioni locali *********************************/

/* Confronta due elementi della lista:
   - ritorna un valore >0 se item1 > item2;
   - ritorna un valore <0 se item1 < item2;
   - ritorna un valore ==0 se item1 == item2.
  NB: puo' essere utilizzata nelle funzioni di ricerca e/o ordinamento
*/
/*
int itemCompare(ItemType item1, ItemType item2) {
    if (item1.value > item2.value) 
        return 1;
    else if (item1.value < item2.value)
        return -1;
    else
        return 0;
}
*/

/* Crea un nuovo giocatore */
Player NewPlayer(char* name, char* surname, int game, int score, int sockfd) {
    Player p;  
    
    p.name = strdup(name);  
    p.surname = strdup(surname);
    p.game = game;
    p.score = score;
    p.sockfd = sockfd;

    return p;  
}


/* dealloca il nodo p */
void deletePlayer(Player* p) {
    free(p);
}

/********** Funzioni standard *******************************/


/* Stampa a video un elemento della lista */
void PrintPlayer(Player* p) {
    /*** esempio ***/
    printf("%-10s %s\n", "Name:", p->name);
    printf("%-10s %s\n", "Surname:", p->surname);
    printf("%-10s %d\n", "Game:", p->game);
    printf("%-10s %d\n", "Score:", p->score);
    printf("%-10s %d\n", "Socket:", p->sockfd);
}


