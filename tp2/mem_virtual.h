#ifndef _MEM_VIRTUAL_H
#define _MEM_VIRTUAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define CLOCK_INTERRUPT 20

typedef struct pagina{
    int page_number, frame, mod, ref, used_time;
} Pagina;

typedef struct hash{
    int hash_size, amt;
    Pagina **itens;
} Hash;

Pagina criaPagina(int page_number, int used_time);

int buscaFrameVazio(int *mem, int size);

void validadeArgumentos(int argc, char **argv);

unsigned int lsBits(unsigned int page_size);

int nru(Hash *page_table, int class);

int substituicao(Hash *page_table, char *pol_sub, int *vetor_auxiliar);

void resetReferenced(Hash *page_table);

void simularMem(int page_size, int mem_size, char *pol_sub, char *file_name);

#endif