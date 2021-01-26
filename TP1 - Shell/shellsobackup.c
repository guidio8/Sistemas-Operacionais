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

//criar as variaveis que dirão se existe entrada, saída, background, pipe

int main(int argc, char** argv){
    char delim[] = "\t \n";
    bool entrada, saida, background, pipes;
    int fd[2], aux1 = 0, aux2 = 0, stats; //existem exatos dois descritores para a chamada de pipe
    pid_t idprocesso; 
    char* cmd = (char*) calloc(BUFFER + 1, sizeof(char)); 
    char* token = NULL;
    char** comandos;
    char** comandosPipe = NULL;
    char arqSaida[50], arqEntrada[50];
    char ch;
    //checar quantidade de argumentos
    if(argc > 1){
        printf("Passou mais argumentos do que poderia, tente novamente\n");
        exit(1);
    }

    pipe(fd);
  
   /* while(true){
        entrada = false;
        saida = false;
        background = false;
        pipes = false;
        fgets(cmd, BUFFER, stdin);
        if(!strcmp(cmd, "fim\n")){
            exit(1);
        }
        token = strtok(cmd, delim); //primeiro comando
        comandos = (char**) calloc (20, sizeof(char));
        comandos[aux1] = token;
        aux1 ++;
        //tratar entrada a seguir
        while(token = strtok('\0', delim)){
            
            if(!strcmp(token, "=>")){
                saida = true;
                token = strtok('\0', delim);
                strcpy(arqSaida, token);
                printf("Arquivo de saida: %s\n", arqSaida);
            }else if(!strcmp(token, "<=")){
                entrada = true;
                token = strtok('\0', delim);
                strcpy(arqEntrada, token);
                printf("Arquivo de Entrada: %s\n", arqEntrada);
            }else if(!strcmp(token, "|")){
                pipes = true;
                aux2 = aux1;
                aux1 = 0;
                comandosPipe = (char**) calloc (20, sizeof(char));

            }else if(!strcmp(token, "&")){
                background = true;
                printf("comando em background\n");
            }else{
                comandos[aux1] = token;
                aux1++;
            }
        }
        printf("teste\n");
    }

    for(int i = 0; i<aux1; i++){
        printf("%s\n", comandos[i]);
    }*/
    entrada = false;
    saida = false;
    background = false;
    pipes = false;
    comandos = (char**) calloc (20, sizeof(char));
    comandos[0] = "ls";
    comandos[1] = "-l";
    comandos[2] = NULL;

    idprocesso = getpid();
    switch (idprocesso = fork()){
        default: 
            //se o processo for o processo for o processo pai e ele nao for um processo de background ele precisa esperar que o processo filho finalize
            if(background == false){
                waitpid(idprocesso, &stats, WUNTRACED);
                //se houver pipe fecha o processo pai;
                if(pipes == true){
                    close(fd[1]);
                }
            }
        case 0:
            if(entrada == true){
                if(access(arqEntrada, F_OK)){
                    printf("Arquivo não existe");
                }else if(access(arqEntrada, R_OK)){
                    printf("Arquivo não pode ser lido");
                }else{
                    freopen(arqEntrada, "r", stdin);
                }
            }
            if(saida == true){
                if(pipes == false){
                    freopen(arqSaida, "w", stdout);
                }
            }
            if(pipes == true){
              close(fd[0]);
              dup2(fd[1], 1);
            }
            execvp(comandos[0], comandos);
            perror("Comando nao encontrado");
            exit (1);
        case -1:
            printf("Erro no fork\n");
            exit(1);
    }
    free(comandos);
    free(comandosPipe);
    return 0;
}