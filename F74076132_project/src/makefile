all: main
main: main.o
	gcc -o main main.o -lm 
main.o: main.c unit.h
	gcc -c main.c 

clean:
	rm -f main.o main