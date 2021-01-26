all: tp2virtual
tp2virtual: hash.o mem_virtual.o main.o 
		gcc -o tp2virtual hash.o mem_virtual.o main.o
mem_virtual.o: mem_virtual.c
		gcc -o mem_virtual.o -c mem_virtual.c -W -Wall -pedantic
hash.o: hash.c
		gcc -o hash.o -c hash.c -W -Wall -pedantic
main.o: main.c hash.h mem_virtual.h
		gcc -o main.o -c main.c -W -Wall -pedantic
clean:
		rm -rf *.o
mrproper: clean
		rm -rf teste