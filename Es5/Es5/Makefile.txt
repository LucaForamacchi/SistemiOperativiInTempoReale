all:	attuatore sensore hub server 

attuatore:	attuatore.c
	gcc -Wall -o attuatore attuatore.c

sensore:	sensore.c
	gcc -Wall -o sensore sensore.c
	
hub:	hub.c list.o
	gcc -Wall -o hub hub.c list.o

server:	server.c list.o
	gcc -Wall -o server server.c list.o

.c.o:
	gcc -g -Wall -c $<

clean:
	rm -f attuatore sensore hub server *.o *~

