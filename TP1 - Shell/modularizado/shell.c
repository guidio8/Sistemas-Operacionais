#include "shell.h"

char *lerlinha(FILE *arq){
    char *line = NULL;
    size_t bufsize = 0;
    if(arq == NULL){
        getline(&line, &bufsize, stdin);
    }else{
        getline(&line, &bufsize, arq);
    }
    
    return line;
}

token_t tokenlinha(char *linha){
    token_t tokens;
    int pos = 0, numPipes = 0;
    char *token;
    
    // Alocando a quantidade de comandos existentes
    // tokens.arg = malloc(sizeof(char**));
    // tokens.arg[0] = malloc(TOKEN_BUFFER * sizeof(char*));

    tokens.commands = (cmd_t*)malloc(sizeof(cmd_t));

    tokens.commands[0] = criacmds();
    
    token = strtok(linha, DELIM);

    while (token != NULL) {
        if ((strcmp(token, "=>") != false) && (strcmp(token, "<=") != false) &&
            (strcmp(token, "|")  != false) && (strcmp(token, "&")  != false)){
            tokens.commands[numPipes].arg[pos] = token;
            tokens.commands[numPipes].arg_size++;
            pos++;
        }
        if(!strcmp(token, "<=")){
            token =  strtok(NULL, DELIM);

            // Alocando espaço de memória para arquivo de entrada
            tokens.commands[numPipes].arquivo[0] = (char*)malloc(FILE_BUFFER*sizeof(char));
            if(!tokens.commands[numPipes].arquivo[0]){
                perror("Falha ao alocar espaco para arquivo.\n");
                exit(1);
            }
            strcpy(tokens.commands[numPipes].arquivo[0], token);

            // Atribuindo valor true para sua info
            tokens.commands[numPipes].info[0] = 1;
        }
        if(!strcmp(token, "=>")){
            token =  strtok(NULL, DELIM);

            // Alocando espaço de memória para arquivo de saída
            tokens.commands[numPipes].arquivo[1] = (char*)malloc(FILE_BUFFER*sizeof(char));
            if(!tokens.commands[numPipes].arquivo[1]){
                perror("Falha ao alocar espaco para arquivo.\n");
                exit(1);
            }
            strcpy(tokens.commands[numPipes].arquivo[1], token);
            
            // Atribuindo valor true para sua info
            tokens.commands[numPipes].info[1] = 1;
        }
        if(!strcmp(token, "&")){
            // Atribuindo valor true para sua info
            tokens.commands[numPipes].info[2] = 1;
        }
        if(!strcmp(token, "|")){
            tokens.commands[numPipes].arg[pos] = NULL;
            
            // Atribuindo valor true para sua info
            tokens.commands[numPipes].info[3] = 1;
            
            numPipes++;
            pos = 0;
            
            // Alocando espaço para novo comando
            tokens.commands = (cmd_t*)realloc(tokens.commands, (numPipes+1)*sizeof(cmd_t));

            tokens.commands[numPipes] = criacmds();
        }
        token = strtok(NULL, DELIM);
    }

    tokens.commands[numPipes].arg[pos] = NULL;
    tokens.qtdtoken = numPipes;

    return tokens;
}

cmd_t criacmds(){
    cmd_t cmds;
    for(int i = 0; i < 4; i++) cmds.info[i] = 0;
    cmds.arg_size = 0;
    cmds.arg = malloc(FILE_BUFFER * sizeof(char*));

    if (!cmds.arg) {
        perror("Falha ao criar token.\n");
        exit(1);
    }
    return cmds;
}

void executecmds(token_t tokens){
    int stats, cmdatual = 0;
    pid_t idprocesso, processo2;
    char ch;
    //int wpid;

    int fd[2], fdd = 0;
    pipe(fd);   
    do{ 
        idprocesso = fork();
        if (idprocesso == 0) {
            // Se houver arquivo de entrada
            // Verificar permissão de escrita
            if(tokens.commands[cmdatual].info[0] == true){
                if(access(tokens.commands[cmdatual].arquivo[0], F_OK)){
                    printf("Arquivo %s inexistente.\n", tokens.commands[cmdatual].arquivo[0]);
                }else if(access(tokens.commands[cmdatual].arquivo[0], R_OK)){
                    printf("Arquivo nao pode ser lido.\n");
                }else{
                    freopen(tokens.commands[cmdatual].arquivo[0], "r", stdin);
                }
            }
            // Se houver arquivo de saída
            if(tokens.commands[cmdatual].info[1] == true){
                if(tokens.commands[cmdatual].info[3] == false){
                    freopen(tokens.commands[cmdatual].arquivo[1], "w", stdout);
                }
            }
            // Se o comando for executado em background
            if(tokens.commands[cmdatual].info[2] == true){
                signal(SIGCHLD, background); 
                waitpid(idprocesso, &stats, WUNTRACED); 
                // //printf("BACKGROUND\n");
            }
            // Se houver pipeline
            if(tokens.qtdtoken != 0){
                // criar as chamadas de pipe
                switch(processo2 = fork()) {
                case -1:
                    perror("fork");
                    exit(1);
                case 0:
                    if (tokens.commands[cmdatual].info[1] == true)
                        freopen(tokens.commands[cmdatual].arquivo[1], "w", stdout);

                    /*fecha a saida e duplica a entrada*/
                    close(fd[1]);
                    dup2(fd[0], 0);

                    while(read(fd[0], &ch, 1) > 0){
                        write(1, &ch, 1);
                    }
                    execvp(tokens.commands[cmdatual].arg[0], tokens.commands[cmdatual].arg);
                    perror("Comando nao encontrado");
                    exit(1);
                default:
                    if (tokens.commands[cmdatual].info[2] == false){
                    waitpid(idprocesso, &stats, WUNTRACED);
                    close(fd[0]);
                    }
                    break;
                }
            }
            execvp(tokens.commands[cmdatual].arg[0], tokens.commands[cmdatual].arg);
            perror("Comando nao reconhecido");
            exit(1);
        }else if (idprocesso < 0) {
            // Error forking
            perror("Falha ao criar o fork.\n");
            exit(1);
        }else{
            // do{
            //     wpid = waitpid(idprocesso, &stats, WUNTRACED);
            // }while (!WIFEXITED(stats) && !WIFSIGNALED(stats));
            // wait(NULL);
            if(tokens.commands[cmdatual].info[2] == false){
                waitpid(idprocesso, &stats, WUNTRACED); 
            }
            close(fd[1]);
            fdd = fd[0];
        }
        cmdatual++;
    }while(cmdatual < tokens.qtdtoken);
}

void freeToken(token_t tokens){
    for(int i = 0; i < tokens.qtdtoken; i++){
        for(int j = 0; j < tokens.commands[i].arg_size; j++){
            free(tokens.commands[i].arg[j]);
        }
        free(tokens.commands[i].arg);
        if(tokens.commands[i].arq_e == 1 || tokens.commands[i].arq_s == 1) 
            free(tokens.commands[i].arquivo);
    }
}

void background(int num){
    wait(NULL);
}