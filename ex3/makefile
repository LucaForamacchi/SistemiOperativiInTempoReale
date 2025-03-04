main: main.o fast_trigo.o
	gcc -o main main.o fast_trigo.o
main.o: main.c fast_trigo.h
	gcc -c main.c
fast_trigo.o: fast_trigo.c fast_trigo.h
	gcc -c fast_trigo.c
clean:
	rm *.o a.out main