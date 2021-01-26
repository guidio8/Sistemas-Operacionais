#include "hash.h"

Hash *criaHash(int size){
    Hash *h = (Hash*)calloc(1, sizeof(Hash));
    if(h != NULL){
        h->hash_size = size;
        h->itens = (Pagina**) calloc(size, sizeof(Pagina*));
        if(h->itens == NULL){
            free(h);
            return NULL;
        }
        h->amt = 0;
        for(int i = 0; i < h->hash_size; i++) h->itens[i] = NULL;
    }
    return h;
}

void freeHash(Hash *h){
    if(h != NULL){
        for(int i = 0; i < h->hash_size; i++)
            if(h->itens[i] != NULL) free(h->itens[i]);
        free(h->itens);
        free(h);
    }
}

int sondagemLinear(int pos, int i, int hash_size){
    return ((pos + i) & 0x7FFFFFFF) % hash_size;
}

int hashFunction(int chave, int size){
    return (chave & 0x7FFFFFFF) % size;
}

int buscaHash(Hash *h, int page_number){
    // Hash não existe
    if(h == NULL) return -3;
    int pos = hashFunction(page_number, h->hash_size);
    for(int i = 0; i < h->hash_size; i++){
        int newpos = sondagemLinear(pos, i, h->hash_size);
        // Item não encontrado
        if(h->itens[newpos] == NULL)
            return -1;
        // Sucesso na busca do item
        if(h->itens[newpos]->page_number == page_number)
            return newpos;
    }
    // Erro na inserção anterior do item
    // Hash cheia e item não encontrado
    return -2;
}

int insereHash(Hash *h, Pagina item){
    // Hash não existe
    if(h == NULL) return -4;
    // Falha na substituição
    if(h->amt == h->hash_size) return -3;
    
    int chave = item.page_number;
    int pos = hashFunction(chave, h->hash_size);
    
    for(int i = 0; i < h->hash_size; i++){
        int newpos = sondagemLinear(pos, i, h->hash_size);
        if(h->itens[newpos] == NULL){
            Pagina* novo = (Pagina*)malloc(sizeof(Pagina));
            // Falha na alocação de uma nova pág.
            if(novo == NULL) return -2;
            *novo = item;
            h->itens[newpos] = novo;
            h->amt++;
            // sucesso na inserção
            return newpos;
        }
    }
    // Posição livre não encontrada
    return -1;
}