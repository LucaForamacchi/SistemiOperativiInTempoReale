all: client server player

client: client.c
	gcc -Wall -o client client.c player.o

server: server.c player.o list.o 
	gcc -Wall -o server server.c player.o list.o 

player: player.c
	gcc -Wall -c player.c -o player.o

list.o: list.c list.h 
	gcc -Wall -c list.c -o list.o

clean:
	rm -f client server player *.o *~

