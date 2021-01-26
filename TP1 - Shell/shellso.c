#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <fcntl.h>
#include <signal.h>

#define BUFFER 512

void resetinfos(bool infos[4]){
    for(int i = 0; i < 4; i++) infos[i] = false;
}

void gettoken(char *cmd, char *delim, bool infos[4], char **comandos[2], char *arquivo[2]){
    // info 0        1      2           3
    // var  entrada, saida, background, pipes;
    int cont = 0;
    char *token;
    
    for(int i = 0; i < 2; i++) comandos[i] = (char**) calloc (30, sizeof(char));
    token =  strtok(cmd, delim);
    comandos[0][0] = token;
    cont = 1;
    token =  strtok(NULL, delim);
    //tratar entrada a seguir
    while(token != NULL){
        //printf("token: %s\n", token);
        if ((strcmp(token, "=>") != false) && (strcmp(token, "<=") != false) &&
            (strcmp(token, "|") != false) && (strcmp(token, "&") != false)){
            if (infos[3] == false){
                comandos[0][cont] = token;
            }else{
                comandos[1][cont] = token;
            }
            cont++;
            token = strtok(NULL, delim);
        }
        
        if(!strcmp(token, "<=")){
            infos[0] = true;
            token = strtok(NULL, delim);
            // printf(" Possui arquivo de Entrada\n");
        }
        
        if(!strcmp(token, "=>")){
            infos[1] = true;
            token = strtok(NULL, delim);
            for(int i = 0; i < 2; i++) arquivo[i] = (char*) calloc (50, sizeof(char));
            strcpy(arquivo[0], token); 
            // printf(" Possui arquivo de saida\n");
        }
        
        if(!strcmp(token, "|")){
            infos[3] = true;
            token = strtok(NULL, delim);
            // printf(" Possui comando com pipe\n");            
        }
        
        if(!strcmp(token, "&")){
            infos[2] = true;
            token = strtok(NULL, delim);
            // printf(" Possui comando em background\n");
        }
        token = strtok(NULL, delim);
    }
    for(int i = 0; i<cont; i++){
        printf("%s\n", comandos[0][i]);
    }
    comandos[0][cont+1] = NULL;
}


int main(int argc, char** argv){
    char delim[] = "\t \n";
    int fd[2], aux1 = 0, aux2 = 0, stats; //existem exatos dois descritores para a chamada de pipe
    bool infos[4];
    pid_t idprocesso; 
    char* cmd = (char*) calloc(BUFFER + 1, sizeof(char)); 
    char **comandos[2];
    char *arquivos[2];
    char c;
    //checar quantidade de argumentos
    if(argc > 1){
        printf("Passou mais argumentos do que poderia, tente novamente\n");
        exit(1);
    }

    pipe(fd);
  
    while(true){
        printf("$ ");
        resetinfos(infos);
        fgets(cmd, BUFFER, stdin);
        if(!strcmp(cmd, "fim\n")){
            exit(1);
        }
        gettoken(cmd, delim, infos, comandos, arquivos);
        idprocesso = getpid();
        switch (idprocesso = fork()){
            default: 
                //se o processo for o processo for o processo pai e ele nao for um processo de background ele precisa esperar que o processo filho finalize
                if(infos[2] == false){
                    waitpid(idprocesso, &stats, WUNTRACED);
                    //se houver pipe fecha o processo pai;
                    if(infos[3] == true){
                        close(fd[1]);
                    }
                break;
                }
            case 0:
                if(infos[0] == true){
                    if(access(arquivos[1], F_OK)){
                        printf("Arquivo não existe");
                    }else if(access(arquivos[1], R_OK)){
                        printf("Arquivo não pode ser lido");
                    }else{
                        freopen(arquivos[1], "r", stdin);
                    }
                }
                if(infos[1] == true){
                    if(infos[3] == false){
                        printf("%s\n", arquivos[0]);
                        freopen(arquivos[0], "w", stdout);
                    }
                }
                if(infos[3] == true){
                close(fd[0]);
                dup2(fd[1], 1);
                }
                char path[] = "/usr/bin/";
                strcat(path, comandos[0][0]); 
                execvp(comandos[0][0], comandos[0]);
                perror("Comando nao encontrado");
                exit (1);
            case -1:    
                printf("Erro no fork\n");
                exit(1);
        }
    }
    return 0;
}