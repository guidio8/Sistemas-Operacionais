#ifndef __HASH_H
#define __HASH_H
#include "mem_virtual.h"

Hash *criaHash(int size);

void freeHash(Hash *h);

int sondagemLinear(int pos, int i, int hash_size);

int hashFunction(int chave, int size);

int buscaHash(Hash *h, int page_number);

int insereHash(Hash *h, Pagina item);


#endif