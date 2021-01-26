all: fat
fat: fat.o main.o 
		gcc -o tp3 fat.o main.o -lm
fat.o: fat.c
		gcc -o fat.o -c fat.c -W -Wall -pedantic -lm
main.o: main.c fat.h
		gcc -o main.o -c main.c -W -Wall -pedantic -lm
clean:
		rm -rf *.o
mrproper: clean
		rm -rf teste