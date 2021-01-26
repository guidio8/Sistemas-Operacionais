#include <stdio.h>
#include "shell.h"

int main(int argc, char** argv){
    char *linha;
    token_t tokens;
    // cat -n <= arquivo | wc -l => numerodelinhas &
    
    //checando quantidade de argumentos
    if(argc > 2){
        printf("Passou mais argumentos do que poderia, tente novamente\n");
        exit(1);
    }else if(argc == 2){
        FILE *f;
        f = fopen(argv[1], "r");
        if(f != NULL){
            while(!feof(f)) {
            
            linha = lerlinha();

            tokens = tokenlinha(linha);
            
            if(strcmp(linha, "fim") == 0) exit(0);
            
            executecmds(tokens);

            freeToken(tokens);        
        }

        free(linha);
        fclose(f);
        }else{
            printf("Arquivo inexistente\n");
            exit(1);
        }
    }else if(argc == 1){

        while(!feof(stdin)) {
            printf("$ ");
            
            linha = lerlinha(NULL);

            tokens = tokenlinha(linha);
            
            if(strcmp(linha, "fim") == 0) exit(0);
            
            executecmds(tokens);

            freeToken(tokens);        
        }

        free(linha);
    }
    return 0;
}