#ifndef _PROFILE_H
#define _PROFILE_H

#define BOOL int
#define FALSE 0
#define TRUE (!FALSE)

/*** Profilo del giocatore ***/
typedef struct {
    char* name;
    char* surname;
    int game;
    int score;
    int sockfd;
} Player;


/*** Costruttore/Distruttore ***/

/* Inizializza un nuovo giocatore */
Player NewPlayer(char* name, char* surname, int game, int score, int sockfd);

/* Cancella un giocatore */
Player DeletePlayer(char* name, char* surname);

/* ritorna il punteggio di un giocatore */
int getScore(Player p);

/*** Stampe ***/

/* Stampa a video un giocatore */
void PrintPlayer(Player* p);


#endif
