#ifndef _FAT_H
#define _FAT_H

/*DEFINE*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define SECTOR_SIZE         512
#define CLUSTER_SIZE	    1024
#define ENTRY_BY_CLUSTER    32
#define NUM_CLUSTER	        4096
#define fat_name	        "fat.part"
#define BLUE_COLOR          "\x1b[31m\x1b[44m"
#define RESET_COLOR         "\x1b[0m"

typedef struct _dir_entry_t{
    unsigned char filename[18];
    unsigned char attributes;
    unsigned char reserved[7];
    unsigned short first_block;
    unsigned int size;
} dir_entry_t;

typedef struct _data_cluster{
    dir_entry_t dir[ENTRY_BY_CLUSTER];
    char data[1000];
} data_cluster;

typedef struct _root_data{
    dir_entry_t dir;
    char data[1000];
} root_data;

/*DATA DECLARATION*/
unsigned short fat[NUM_CLUSTER];
unsigned char boot_block[CLUSTER_SIZE];
root_data root_dir[ENTRY_BY_CLUSTER];
data_cluster clusters[NUM_CLUSTER-10];

void init();
void load();
void ls(char param[]);
void mkdir(char param[]);
void create(char param[]);
void unlink(char param[]);
void read(char param[]);
void write(char str[], char param[]);
void append(char str[], char param[]);
void attArquivo();
int __freeSpaceFAT();
void help();

#endif //TP3_SO__FAT_H