#include "fat.h"

int main(){
    char cmd[6], string[100], entrada[100];

    printf("'help' para mais informações\n");
    while (1){
        printf("$ ");
        scanf("%s", cmd);

        if (strcmp(cmd, "init") == 0){
            init();
        }else if (strcmp(cmd, "load") == 0){
            load();
        }else if (strcmp(cmd, "exit") == 0){
            printf("Salvando os dados...\n");
            attArquivo();
            return 0;
        }else if (strcmp(cmd, "ls") == 0){
            scanf("%s", entrada);
            ls(entrada);
        }else if (strcmp(cmd, "mkdir") == 0){
            scanf("%s", entrada);
            //mkdir
            mkdir(entrada);
        }else if (strcmp(cmd, "create") == 0){
            scanf("%s", entrada);
            //create
            create(entrada);
        }else if (strcmp(cmd, "unlink") == 0){
            scanf("%s", entrada);
            //unlink
            unlink(entrada);
        }else if (strcmp(cmd, "write") == 0){
            scanf("%s", string);
            scanf("%s", entrada);
            write(string, entrada);

        }else if (strcmp(cmd, "append") == 0){
            scanf("%s", string);
            scanf("%s", entrada);
            append(string, entrada);
        }else if(strcmp(cmd, "read") == 0){
            scanf("%s", entrada);
            read(entrada);
        }else if(strcmp(cmd, "help") == 0){
            help();
        }else{
            printf("Comando inválido. Digite 'help' para mais informações.\n");
        }
    }

    return 0;
}