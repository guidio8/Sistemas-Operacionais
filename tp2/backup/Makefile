all: tp2virtual
tp2virtual: main.o 
		gcc -o tp2virtual main.o
main.o: main.c 
		gcc -o main.o -c main.c -W -Wall -pedantic
clean:
		rm -rf *.o
mrproper: clean
		rm -rf teste