all: shellso
shellso: shell.o main.o 
		gcc -o shellso shell.o main.o
shell.o: shell.c
		gcc -o shell.o -c shell.c -W -Wall -pedantic
main.o: main.c shell.h
		gcc -o main.o -c main.c -W -Wall -pedantic
clean:
		rm -rf *.o
mrproper: clean
		rm -rf teste