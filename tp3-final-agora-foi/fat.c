#include "fat.h"

void init(){
	FILE* ptr_file;
	ptr_file = fopen(fat_name,"wb");
	for(int i= 0; i < 2; ++i)
		boot_block[i] = 0xbb;
    
    fat[0] = 0xfffd;
    for(int i = 1; i < 9; i++)
		fat[i] = 0xfffe;
    fat[9] = 0xffff;
	for(int i= 10; i < NUM_CLUSTER; i++)
		fat[i] = 0;

	fwrite(&boot_block, sizeof(boot_block), 1,ptr_file);
	fwrite(&fat, sizeof(fat), 1, ptr_file);

    memset(root_dir, 0x00, sizeof(root_dir));
	fwrite(&root_dir, sizeof(root_dir), 1,ptr_file);

    memset(clusters, 0x00, sizeof(clusters));
    fwrite(&clusters, sizeof(clusters), 1,ptr_file);
    
	fclose(ptr_file);
}

void load(){
	FILE* ptr_file;
	ptr_file = fopen(fat_name, "rb+");
	fseek(ptr_file, sizeof(boot_block), SEEK_SET);
	fread(fat, sizeof(fat), 1, ptr_file);
	fread(root_dir, sizeof(root_dir), 1, ptr_file);
	fclose(ptr_file);
}

void attArquivo(){
	FILE* ptr_file;
	ptr_file = fopen(fat_name, "rb+");
	fwrite(&boot_block, sizeof(boot_block), 1,ptr_file);
	fwrite(&fat, sizeof(fat), 1,ptr_file);
	fwrite(&root_dir, sizeof(root_dir), 1,ptr_file);
	fwrite(&clusters, sizeof(clusters), 1,ptr_file);

	fclose(ptr_file);

}

void mkdir(char *param){
    if(param[0] != '/'){
        printf("O caminho deve conter o diretorio raiz '\'\n");
        return;
    }
    if(param[strlen(param)-1] != '/'){
        printf("O caminho deve terminar com '/'\n");
        return;
    }

    char *cpy = (char*)malloc(sizeof(char)*strlen(param));
    strcpy(cpy, param);
    char *tkn = strtok(cpy, "/");
    int dir_amt = -1;
    while(tkn != NULL){
        tkn = strtok(NULL, "/");
        dir_amt++;
    }
    free(cpy);

    tkn = strtok(param, "/");

    int rd_pos = -1, found = 0;

    // percorrendo o root_dir
    for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
        // verificando se já existe
        if(root_dir[i].dir.size > 0 && strcmp((char*)root_dir[i].dir.filename, tkn) == 0){
            if(dir_amt == 0 && root_dir[i].dir.attributes == 1){
                // criando no root
                found = 1;
            }else{
                // criando em subdiretorios
                rd_pos = root_dir[i].dir.first_block;
            }
        }
    }

    if(dir_amt == 0){
        if(found == 1){
            printf("Diretório existente.\n");
            return;
        }
        // criando no root
        for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
            // atribuindo o valor no primeiro espaço vazio do root
            if(root_dir[i].dir.size == 0){
                // procura  posição vazia na fat preenche também
                int fat_pos = __freeSpaceFAT();
                if(fat_pos != -1){
                    strcpy((char*)root_dir[i].dir.filename, tkn);
                    root_dir[i].dir.first_block = fat_pos;
                    root_dir[i].dir.attributes = 1;
                    root_dir[i].dir.size = 2;
                    fat[fat_pos] = 1;
                }else{
                    printf("Sistema de arquivos cheio.\n");
                }
                return;
            }
        }
        printf("Limite de diretorios atingido.\n");
        return;
    }

    // encontrou a primeira subpasta
    if(rd_pos > -1){
        // percorrer o caminho até o final
        char *curr_dir, *next_dir = NULL;
        int block = rd_pos;
        tkn = strtok(NULL, "/");
        // caso tenha mais de um diretório além do root no caminho
        while(tkn != NULL){
            curr_dir = tkn;
            tkn = strtok(NULL, "/");
            next_dir = tkn;

            if(next_dir == NULL){
                // verificar existencia
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    if(clusters[block].dir[i].size != 0){
                        if(strcmp((char*)clusters[block].dir[i].filename, curr_dir) == 0){
                            if(clusters[block].dir[i].attributes == 1){
                                printf("Diretorio existente.\n");
                                return;
                            }
                        }
                    }else{
                        // procura  posição vazia na fat preenche também
                        int fat_pos = __freeSpaceFAT();
                        if(fat_pos != -1){
                            strcpy((char*)clusters[block].dir[i].filename, curr_dir);
                            clusters[block].dir[i].first_block = fat_pos;
                            clusters[block].dir[i].attributes = 1;
                            clusters[block].dir[i].size = 2;
                            fat[fat_pos] = 1;
                            
                        }else{
                            printf("Sistema de arquivos cheio.\n");
                        }
                        return;
                    }
                }
                printf("Limite de diretorios atingido.\n");
                return;
            }else{
                // buscar pelo dir seguinte
                found = 0;
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    if(clusters[block].dir[i].size != 0  && clusters[block].dir[i].attributes == 1){
                        if(strcmp((char*)clusters[block].dir[i].filename, next_dir) == 0){
                            block = clusters[block].dir[i].first_block;
                            found = 1;
                        }
                    }                    
                }
                if(found == 0){
                    printf("Diretorio inexistente.\n");
                    return;
                }
            }
        }
    }
}

void create(char *param){
    if(param[0] != '/'){
        printf("O caminho deve conter o diretorio raiz '\'\n");
        return;
    }
    if(param[strlen(param)-1] == '/'){
        printf("O caminho não deve terminar com '/'\n");
        return;
    }

    char *cpy = (char*)malloc(sizeof(char)*strlen(param));
    strcpy(cpy, param);
    char *tkn = strtok(cpy, "/");
    int dir_amt = -1;
    while(tkn != NULL){
        tkn = strtok(NULL, "/");
        dir_amt++;
    }
    free(cpy);

    tkn = strtok(param, "/");

    int rd_pos = -1, found = 0;

    for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
        if(root_dir[i].dir.size != 0){
            if(strcmp((char*)root_dir[i].dir.filename, tkn) == 0){
                if(dir_amt == 0 && root_dir[i].dir.attributes == 0){
                    found = 1;
                }else{
                    rd_pos = root_dir[i].dir.first_block;
                }
            }
        }
    }

    if(dir_amt == 0){
        if(found == 1){
            printf("Arquivo existente.\n");
            return;
        }
        
        for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
            if(root_dir[i].dir.size == 0){
                int fat_pos = __freeSpaceFAT();
                if(fat_pos != -1){
                    strcpy((char*)root_dir[i].dir.filename, tkn);
                    root_dir[i].dir.first_block = fat_pos;
                    root_dir[i].dir.attributes = 0;
                    root_dir[i].dir.size = 2;
                    fat[fat_pos] = 1;

                }else{
                    printf("Sistema de arquivos cheio.\n");
                }
                return;
            }
        }
        printf("Limite de diretorios atingido.\n");
        return;
    }

    if(rd_pos > -1){
        char *curr_dir, *next_dir = NULL;
        int block = rd_pos;
        tkn = strtok(NULL, "/");
        while(tkn != NULL){
            curr_dir = tkn;
            tkn = strtok(NULL, "/");
            next_dir = tkn;

            if(next_dir == NULL){
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    if(clusters[block].dir[i].size != 0){
                        if(strcmp((char*)clusters[block].dir[i].filename, curr_dir) == 0){
                            if(clusters[block].dir[i].attributes == 0){
                                printf("Arquivo existente.\n");
                                return;
                            }
                        }
                    }else{
                        int fat_pos = __freeSpaceFAT();
                        if(fat_pos != -1){
                            strcpy((char*)clusters[block].dir[i].filename, curr_dir);
                            clusters[block].dir[i].first_block = fat_pos;
                            clusters[block].dir[i].attributes = 0;
                            clusters[block].dir[i].size = 2;
                            fat[fat_pos] = 1;
                            
                        }else{
                            printf("Sistema de arquivos cheio.\n");
                        }
                        return;
                    }
                }
                printf("Limite de diretórios atingido.\n");
                return;
            }else{
                found = 0;
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    if(clusters[block].dir[i].size != 0  && clusters[block].dir[i].attributes == 1){
                        if(strcmp((char*)clusters[block].dir[i].filename, curr_dir) == 0){
                            block = clusters[block].dir[i].first_block;
                            found = 1;
                        }
                    }                    
                }
                if(found == 0){
                    printf("Diretório inexistente.\n");
                    return;
                }
            }
        }
    }
}

int __freeSpaceFAT(){
    for(int i = 0; i < NUM_CLUSTER; i++){
        if(fat[i] == 0) return i;
    }
    return -1;
}

void ls(char *param){
    if(param[0] != '/'){
        printf("O caminho deve conter o diretorio raiz '\'\n");
        return;
    }
    if(param[strlen(param)-1] != '/'){
        printf("O caminho deve terminar com '/'\n");
        return;
    }

    char *cpy = (char*)malloc(sizeof(char)*strlen(param));
    strcpy(cpy, param);
    char *tkn = strtok(cpy, "/");
    int dir_amt = 0;
    while(tkn != NULL){
        tkn = strtok(NULL, "/");
        dir_amt++;
    }
    free(cpy);

    tkn = strtok(param, "/");
    int rd_pos = -1;

    // percorrendo o root_dir
    for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
        // verificando se já existe
        if(root_dir[i].dir.size > 0){
            if(dir_amt == 0){
                // arquivos no root
                if(root_dir[i].dir.attributes == 0){
                    printf("<A> %s\n", root_dir[i].dir.filename);
                }else 
                    printf("<D> %s%s%s\n", BLUE_COLOR, root_dir[i].dir.filename, RESET_COLOR);
            }else{
                // criando em subdiretorios
                rd_pos = root_dir[i].dir.first_block;
            }
        }
    }

    if(rd_pos > -1){
        // percorrer o caminho até o final
        char *next_dir = NULL;
        int block = rd_pos;
        
        while (tkn != NULL){
            tkn = strtok(NULL, "/");
            next_dir = tkn;
            if(next_dir == NULL){
                // imprimir conteúdo do curr_dir
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    if(clusters[block].dir[i].size != 0){
                        if(clusters[block].dir[i].attributes == 0)
                            printf("<A> %s\n", clusters[block].dir[i].filename);
                        else
                            printf("<D> %s%s%s\n", BLUE_COLOR, clusters[block].dir[i].filename, RESET_COLOR);
                    }
                }
            }else{
                int found = 0;
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    if(clusters[block].dir[i].size != 0  && clusters[block].dir[i].attributes == 1){
                        if(strcmp((char*)clusters[block].dir[i].filename, next_dir) == 0){
                            block = clusters[block].dir[i].first_block;
                            found = 1;
                        }
                    }                    
                }
                if(found == 0){
                    printf("Diretorio inexistente.\n");
                    return;
                }
            }
        }
    }

}

void unlink(char param[]){
    if(param[0] != '/'){
        printf("O caminho deve conter o diretorio raiz '\'\n");
        return;
    }
    int file = 0;
    if(param[strlen(param)-1] == '/'){
        file = 1;
    }

    char *cpy = (char*)malloc(sizeof(char)*strlen(param));
    strcpy(cpy, param);
    char *tkn = strtok(cpy, "/");
    int dir_amt = -1;
    while(tkn != NULL){
        tkn = strtok(NULL, "/");
        dir_amt++;
    }
    free(cpy);

    tkn = strtok(param, "/");

    int rd_pos = -1, found = 0;

    for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
        if(root_dir[i].dir.size > 0 && strcmp((char*)root_dir[i].dir.filename, tkn) == 0){
            if(dir_amt == 0){
                found = 1;
            }else{
                rd_pos = root_dir[i].dir.first_block;
            }
        }
    }

    if(dir_amt == 0){
        if(found == 0){
            printf("Arquivo inexistente.\n");
            return;
        }
        for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
            if(root_dir[i].dir.size != 0 && strcmp((char*)root_dir[i].dir.filename, tkn) == 0){
                if(file == root_dir[i].dir.attributes){
                    // se diretório verificar se contém arquivos
                    if(root_dir[i].dir.attributes == 1){
                        int block = root_dir[i].dir.first_block;
                        // verificando se diretório está vazio
                        for(int j = 0; j < ENTRY_BY_CLUSTER; j++){
                            if(clusters[block].dir[j].size != 0){
                                printf("O diretorio precisa estar vazio.\n");
                                return;
                            }
                        }
                        root_dir[i].dir.attributes = 2;
                        root_dir[i].dir.size = 0;
                        fat[root_dir[i].dir.first_block] = 0;
                    }
                    // se arquivo simplesmente excluir
                    else{
                        root_dir[i].dir.attributes = 2;
                        root_dir[i].dir.size = 0;
                        fat[root_dir[i].dir.first_block] = 0;
                    }
                    return;
                }
            }
        }
    }

    if(rd_pos > -1){
        char *curr_dir, *next_dir;
        int block = rd_pos;
        tkn = strtok(NULL, "/");
        while(tkn != NULL){
            curr_dir = tkn;
            tkn = strtok(NULL, "/");
            next_dir = tkn;
            if(next_dir == NULL){
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    if(clusters[block].dir[i].size != 0 && strcmp((char*)clusters[block].dir[i].filename, curr_dir) == 0){
                        if(file == clusters[block].dir[i].attributes){
                            // se diretório verificar existencia de arquivos
                            if(clusters[block].dir[i].attributes == 1){
                                int bloco = clusters[block].dir[i].first_block;
                                for(int j = 0; j < ENTRY_BY_CLUSTER; j++){
                                    if(clusters[bloco].dir[j].size != 0){
                                        printf("Diretorio precisa estar vazio.\n");
                                        return;
                                    }
                                }
                                // desreferenciar
                                clusters[block].dir[i].attributes = 2;
                                clusters[block].dir[i].size = 0;
                                fat[clusters[block].dir[i].first_block] = 0;
                                return;
                            }
                            // se arquivo simplesmente exluir
                            else{
                                clusters[block].dir[i].attributes = 2;
                                clusters[block].dir[i].size = 0;
                                fat[clusters[block].dir[i].first_block] = 0;
                                return;
                            }

                            printf("Diretorio precisa estar vazio.\n");
                            return;
                        }
                    }
                }
                printf("Arquivo nao encontrado.\n");
                return;
            }else{
                found = 0;
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    if(clusters[block].dir[i].size != 0 && clusters[block].dir[i].attributes == 1){
                        if(strcmp((char*)clusters[block].dir[i].filename, curr_dir) == 0){
                            block = clusters[block].dir[i].first_block;
                            found = 1;
                        }
                    }                    
                }
                if(found == 0){
                    printf("Diretorio inexistente.\n");
                    return;
                }
            }
        }
    }

    free(cpy);
}

void read(char param[]){
    if(param[0] != '/'){
        printf("O caminho deve conter o diretorio raiz '\'\n");
        return;
    }
    if(param[strlen(param)-1] == '/'){
        printf("O caminho nao deve terminar com '/'\n");
        return;
    }

    char *cpy = (char*)malloc(sizeof(char)*strlen(param));
    strcpy(cpy, param);
    char *tkn = strtok(cpy, "/");
    int dir_amt = -1;
    while(tkn != NULL){
        tkn = strtok(NULL, "/");
        dir_amt++;
    }
    free(cpy);

    tkn = strtok(param, "/");

    int rd_pos = -1, found = 0;

    // percorrendo o root_dir
    for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
        if(root_dir[i].dir.size > 0 && strcmp((char*)root_dir[i].dir.filename, tkn) == 0){
            if(dir_amt == 0){
                // arquivo na raiz
                found = 1;
            }else{
                // procurando em subdiretorios
                rd_pos = root_dir[i].dir.first_block;
            }
        }
    }

    if(dir_amt == 0){
        if(found == 0){
            printf("Arquivo inexistente.\n");
            return;
        }
        // criando no root
        for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
            // escrevendo no arquivo encontrado
            if(root_dir[i].dir.attributes == 0 && strcmp((char*)root_dir[i].dir.filename, tkn) == 0){
                printf("%s\n", root_dir[i].data);
                return;
            }
            
        }
    }

    // encontrou a primeira subpasta
    if(rd_pos > -1){
        // percorrer o caminho até o final
        char *curr_dir, *next_dir;
        int block = rd_pos;
        tkn = strtok(NULL, "/");
        // caso tenha mais de um diretório além do root no caminho
        while(tkn != NULL){
            curr_dir = tkn;
            tkn = strtok(NULL, "/");
            next_dir = tkn;

            if(next_dir == NULL){
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    // lê o arquivo quando encontrado
                    if(clusters[block].dir[i].attributes == 0 && strcmp((char*)clusters[block].dir[i].filename, curr_dir) == 0){
                        printf("%s\n",clusters[block].data);
                        return;
                    }
                }
                printf("Arquivo nao encontrado.\n");
                return;
            }else{
                // buscar pelo dir seguinte
                found = 0;
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    if(clusters[block].dir[i].size != 0 && clusters[block].dir[i].attributes == 1){
                        if(strcmp((char*)clusters[block].dir[i].filename, curr_dir) == 0){
                            block = clusters[block].dir[i].first_block;
                            found = 1;
                        }
                    }                    
                }
                if(found == 0){
                    printf("Diretorio inexistente.\n");
                    return;
                }
            }
        }
    }

    //caso o caminho dado nao seja de um arquivo existente, avisa o erro
    printf("Nao existe o arquivo para leitura\n");
}

void write(char str[], char param[]){
    if(param[0] != '/'){
        printf("O caminho deve conter o diretorio raiz '\'\n");
        return;
    }
    if(param[strlen(param)-1] == '/'){
        printf("O caminho nao deve terminar com '/'\n");
        return;
    }

    char *cpy = (char*)malloc(sizeof(char)*strlen(param));
    strcpy(cpy, param);
    char *tkn = strtok(cpy, "/");
    int dir_amt = -1;
    while(tkn != NULL){
        tkn = strtok(NULL, "/");
        dir_amt++;
    }
    free(cpy);

    tkn = strtok(param, "/");

    int rd_pos = -1, found = 0;

    // percorrendo o root_dir
    for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
        if(root_dir[i].dir.size > 0 && strcmp((char*)root_dir[i].dir.filename, tkn) == 0){
            if(dir_amt == 0){
                // arquivo na raiz
                found = 1;
            }else{
                // procurando em subdiretorios
                rd_pos = root_dir[i].dir.first_block;
            }
        }
    }

    if(dir_amt == 0){
        if(found == 0){
            printf("Arquivo inexistente.\n");
            return;
        }
        // criando no root
        for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
            // escrevendo no arquivo encontrado
            if(root_dir[i].dir.attributes == 0 && strcmp((char*)root_dir[i].dir.filename, tkn) == 0){
                memset(root_dir[i].data, '\0', 1000);
                memcpy(root_dir[i].data,str,strlen(str));
                return;
            }
        }
    }

    // encontrou a primeira subpasta
    if(rd_pos > -1){
        // percorrer o caminho até o final
        char *curr_dir, *next_dir;
        int block = rd_pos;
        tkn = strtok(NULL, "/");
        // caso tenha mais de um diretório além do root no caminho
        while(tkn != NULL){
            curr_dir = tkn;
            tkn = strtok(NULL, "/");
            next_dir = tkn;

            if(next_dir == NULL){
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    // escrever no arquivo quando encontrado
                    if(clusters[block].dir[i].attributes == 0 && strcmp((char*)clusters[block].dir[i].filename, curr_dir) == 0){
                        memset(clusters[block].data, '\0', 1000);
                        memcpy(clusters[block].data,str,strlen(str));
                        return;
                    }
                }
                printf("Arquivo nao encontrado.\n");
                return;
            }else{
                // buscar pelo dir seguinte
                found = 0;
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    if(clusters[block].dir[i].size != 0 && clusters[block].dir[i].attributes == 1){
                        if(strcmp((char*)clusters[block].dir[i].filename, curr_dir) == 0){
                            block = clusters[block].dir[i].first_block;
                            found = 1;
                        }
                    }                    
                }
                if(found == 0){
                    printf("Diretorio inexistente.\n");
                    return;
                }
            }
        }
    }

    free(cpy);
    printf("Nao existe o arquivo para ser escrito\n");
    return;
}

void append(char str[], char param[]){
    if(param[0] != '/'){
        printf("O caminho deve conter o diretorio raiz '\'\n");
        return;
    }
    if(param[strlen(param)-1] == '/'){
        printf("O caminho nao deve terminar com '/'\n");
        return;
    }

    char *cpy = (char*)malloc(sizeof(char)*strlen(param));
    strcpy(cpy, param);
    
    char *tkn = strtok(cpy, "/");
    int dir_amt = -1;
    while(tkn != NULL){
        tkn = strtok(NULL, "/");
        dir_amt++;
    }

    tkn = strtok(param, "/");

    int rd_pos = -1, found = 0;

    // percorrendo o root_dir
    for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
        if(root_dir[i].dir.size > 0 && strcmp((char*)root_dir[i].dir.filename, tkn) == 0){
            if(dir_amt == 0){
                // arquivo na raiz
                found = 1;
            }else{
                // procurando em subdiretorios
                rd_pos = root_dir[i].dir.first_block;
            }
        }
    }

    if(dir_amt == 0){
        if(found == 0){
            printf("Arquivo inexistente.\n");
            return;
        }
        // criando no root
        for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
            // escrevendo no arquivo encontrado
            if(root_dir[i].dir.attributes == 0 && strcmp((char*)root_dir[i].dir.filename, tkn) == 0){
                int final = 0;
                for(int j = 0; j < 1000; j++){
                    if(root_dir[i].data[j] == '\0'){
                        final = j;
                        break;
                    }
                }
                memcpy(&root_dir[i].data[final],str,strlen(str));
                return;
            }
        }
    }

    // encontrou a primeira subpasta
    if(rd_pos > -1){
        // percorrer o caminho até o final
        char *curr_dir, *next_dir;
        int block = rd_pos;
        tkn = strtok(NULL, "/");
        // caso tenha mais de um diretório além do root no caminho
        while(tkn != NULL){
            curr_dir = tkn;
            tkn = strtok(NULL, "/");
            next_dir = tkn;

            if(next_dir == NULL){
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    // escrever no arquivo quando encontrado
                    if(clusters[block].dir[i].attributes == 0 && strcmp((char*)clusters[block].dir[i].filename, curr_dir) == 0){
                        int final = 0;
                        for(int i = 0; i < 1000; i++){
                            if(clusters[block].data[i] == '\0'){
                                final = i;
                                break;
                            }
                        }
                        memcpy(&clusters[block].data[final],str,strlen(str));
                        return;
                    }
                }
                printf("Arquivo nao encontrado.\n");
                return;
            }else{
                // buscar pelo dir seguinte
                found = 0;
                for(int i = 0; i < ENTRY_BY_CLUSTER; i++){
                    if(clusters[block].dir[i].size != 0 && clusters[block].dir[i].attributes == 1){
                        if(strcmp((char*)clusters[block].dir[i].filename, curr_dir) == 0){
                            block = clusters[block].dir[i].first_block;
                            found = 1;
                        }
                    }                    
                }
                if(found == 0){
                    printf("Diretorio inexistente.\n");
                    return;
                }
            }
        }
    }

    free(cpy);
    printf("Nao existe o arquivo para ser escrito\n");
    return;
}

void help(){
    printf("Os comandos disponíveis são:\n");
    printf("init - inicializar o sistema de arquivos\n");
    printf("load - carregar o sistema de arquivos do disco\n");
    printf("ls [/caminho/diretorio] - listar diretório\n");
    printf("mkdir [/caminho/diretorio] - criar diretório\n");
    printf("create [/caminho/arquivo] - criar arquivo\n");
    printf("unlink [/caminho/arquivo] - excluir arquivo ou diretório\n");
    printf("write 'string' [/caminho/arquivo] - escrever dados em um arquivo\n");
    printf("append 'string' [/caminho/arquivo] - anexar dados em um arquivo\n");
    printf("read [/caminho/arquivo] - ler o conteúdo de um arquivo\n");
    printf("exit - sair do sistema e salvar alterações.\n");
}