all: clientC clientF server player

clientC: clientC.c
	gcc -Wall -o clientC clientC.c player.o

clientF: clientF.c
	gcc -Wall -o clientF clientF.c player.o

server: server.c player.o list.o 
	gcc -Wall -o server server.c player.o list.o 

player: player.c
	gcc -Wall -c player.c -o player.o

list.o: list.c list.h 
	gcc -Wall -c list.c -o list.o

clean:
	rm -f clientC clientF server player *.o *~

