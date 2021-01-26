#include "mem_virtual.h"
#include "hash.h"

int main(int argc, char *argv[]){
    
    validadeArgumentos(argc, argv);

    char *pol_sub = argv[1], *file_name = argv[2];
    int page_size = atoi(argv[3]), mem_size = atoi(argv[4]);

    simularMem(page_size, mem_size, pol_sub, file_name);
    
    return 0;
}