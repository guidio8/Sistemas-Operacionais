#ifndef _SHELL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <fcntl.h>
// #include <signal.h>

#define _SHELL_H
#define LINE_BUFFER 512
#define FILE_BUFFER 64
#define DELIM "\t \n"

typedef struct cmd{
    char **arg;
    char *arquivo[2];
    int info[4], arg_size, arq_e, arq_s;
} cmd_t;

typedef struct token{
    int qtdtoken;
    cmd_t *commands;
} token_t;

char *lerlinha();

token_t tokenlinha(char *linha);

cmd_t criacmds();

void executecmds(token_t tokens);

void freeToken(token_t tokens);

void background(int num);

#endif