#include "mem_virtual.h"
#include "hash.h"

Pagina criaPagina(int page_number, int used_time){
    Pagina aux;
    aux.page_number = page_number;
    aux.used_time = used_time;
    aux.frame = -1;
    aux.mod = 0;
    aux.ref = 0;
    return aux;
}

int buscaFrameVazio(int *mem, int size){
    for(int i = 0; i < size; i++){
        // Frame encontrado
        if(mem[i] == 0)
            return i;
    }
    // Sem frames vazios
    return -1;
}

void validadeArgumentos(int argc, char **argv){

    if(argc < 4){
        printf("Quantidade de argumentos invalidos.");
        exit(5);
    }
    // política de substituição
    if((strcmp(argv[1], "lru") != 0) && (strcmp(argv[1], "nru") != 0) && (strcmp(argv[1], "segunda_chance") != 0)){
        printf("Politica de substituicao %s nao reconhecida.\n",  argv[1]);
        exit(1);
    }
    // existência do arquivo
    if((access(argv[2], R_OK)) != 0){
        printf("Arquivo %s nao encontrado.\n",  argv[2]);
        exit(2);
    }
    // tamanho de página
    if(atoi(argv[3]) < 2 || atoi(argv[3]) > 64){ 
        perror("Tamanho de pagina de memoria invalido.\n");
        exit(3);
    }
    // tamanho de memória
    if(atoi(argv[4]) < 128 || atoi(argv[4]) > 16384){ 
        perror("Tamanho da memoria física invalido.\n");
        exit(4);
    }
}

unsigned int lsBits(unsigned int page_size){
    unsigned s, tmp; 

    tmp = page_size*1024;
    s = 0;
    while (tmp>1) {
        tmp = tmp>>1;
        s++;
    }
    return s;
}

int nru(Hash *page_table, int class){
    srand(time(NULL));
    int r, m, change;
    for(int i = 0; i < page_table->hash_size; i++){
        if(page_table->itens[i] != NULL){
            r = page_table->itens[i]->ref;
            m = page_table->itens[i]->mod;
            change = rand() % 10;
            // classe 0
            if(r == 0 && m == 0 && class == 0 && change >= 5){
                return page_table->itens[i]->frame;
            }
            // classe 1
            if(r == 0 && m == 1 && class == 1 && change >= 5){
                return page_table->itens[i]->frame;
            }
            // classe 2
            if(r == 1 && m == 0 && class == 2 && change >= 5){
                return page_table->itens[i]->frame;
            }
            // classe 3
            if(r == 1 && m == 1 && class == 3 && change >= 5){
                return page_table->itens[i]->frame;
            }
        }
    }
    return -1;
}

int substituicao(Hash *page_table, char *pol_sub, int *vetor_auxiliar){
    int frame_tbc = -1, aux, k;  
    if(strcmp(pol_sub, "segunda_chance") == 0){
        while(frame_tbc < 0){
            k = buscaHash(page_table, vetor_auxiliar[0]);
            if(page_table->itens[k]->ref == 1){
                page_table->itens[k]->ref = 0;
                aux = vetor_auxiliar[0];
                for(int i = 0; i<(page_table->hash_size)-1; i++){
                    vetor_auxiliar[i] = vetor_auxiliar[i+1];
                }
                vetor_auxiliar[(page_table->hash_size)-1] = aux;

            }else{  
                frame_tbc = page_table->itens[k]->frame;
                page_table->itens[k]->ref = 1;
            }
        }
    }else
    if(strcmp(pol_sub, "lru") == 0){
        int minor_time = page_table->itens[0]->used_time;
        frame_tbc = page_table->itens[0]->frame;
        for(int i = 0; i < page_table->hash_size; i++){
            if(page_table->itens[i] != NULL){
                if(minor_time > page_table->itens[i]->used_time){
                    minor_time = page_table->itens[i]->used_time;
                    frame_tbc = page_table->itens[i]->frame;
                }
            }
        }
    }else
    if(strcmp(pol_sub, "nru") == 0){
        int class = 0;
        while(frame_tbc == -1 && class < 4){
            frame_tbc = nru(page_table, class);
            class++;
        }
    }

    return frame_tbc;
}

void resetReferenced(Hash *page_table){
    for(int i = 0; i < page_table->hash_size; i++){
        if(page_table->itens[i] != NULL)
            page_table->itens[i]->ref = 0;
    }
}

void simularMem(int page_size, int mem_size, char *pol_sub, char *file_name){
    int frames = mem_size/page_size;
    
    FILE *f = fopen(file_name, "r");
    Hash *page_table = criaHash(frames);
    int page_number, frame;
    int operations = 0, page_faults = 0, page_hits = 0, dirty_page = 0, copy_vector = 0;
    int *memory = (int*)calloc(frames, sizeof(int));
    int *vetor_auxiliar = (int*)calloc(frames, sizeof(int));
    unsigned v_address, offset = lsBits(page_size);
    char open_type;
    
    printf("Executando o simulador...\n");
    if(memory != NULL && vetor_auxiliar != NULL){
        while(!feof(f)){
            int pos_hash = buscaHash(page_table, page_number);
            Pagina pagina = criaPagina(page_number, operations);

            fseek(f, 0, SEEK_CUR);
            fscanf(f, "%x %c\n", &v_address, &open_type);
            
            page_number = v_address >> offset;

            if(operations % CLOCK_INTERRUPT == 0) resetReferenced(page_table);

            if(open_type == 'w' || open_type == 'W'){
                switch (pos_hash){
                case -3:
                    printf("Hash nao existente.\n");
                    exit(8);
                    break;
                case -2:
                    // substituir
                    if(copy_vector == 0){
                        vetor_auxiliar = memory;
                        copy_vector++;
                    }
                    frame = substituicao(page_table, pol_sub, vetor_auxiliar);
                    if(frame == -1){
                        printf("Erro durante a politica de substituicao.\n");
                        exit(9);
                    }
                    pagina.mod = 0;
                    pagina.ref = 1;
                    pagina.frame = frame;
                    pagina.used_time = operations;
                    
                    // Atualizando TP
                    *page_table->itens[buscaHash(page_table, memory[frame])] = pagina;
                    
                    memory[frame] = pagina.page_number;
                    break;
                case -1:
                    // Procurar frame vazio
                    frame = buscaFrameVazio(memory, frames);
                    if(frame == -1){
                        printf("Falha ao encontrar frame vazio.\n");
                        exit(10);
                    }else{
                        // Atualizar bits da página
                        pagina.mod = 0;
                        pagina.ref = 1;
                        pagina.frame = frame;
                        pagina.used_time = operations;
                        
                        // Atualizando TP
                        memory[frame] = pagina.page_number;
                        switch(insereHash(page_table, pagina)){
                            case -4:
                                printf("Hash nao existente.\n");
                                exit(8);
                                break;
                            case -2:
                                printf("Falha na alocação de uma nova pág.\n");
                                exit(9);
                                break;
                            case -1:
                            case -3:
                                printf("Erro durante a politica de substituicao.\n");
                                exit(9);
                                break;
                            default:
                                break;
                        }
                        
                    }
                    page_faults++;
                    break;
                default:
                    // Atualizar bits da página
                    pagina.mod = 1;
                    pagina.ref = 1;
                    pagina.frame = page_table->itens[pos_hash]->frame;
                    pagina.used_time = operations;

                    *page_table->itens[pos_hash] = pagina;

                    dirty_page++;
                    break;
                }
            }else
            if(open_type == 'r' || open_type == 'R'){
                switch (pos_hash){
                case -3:
                    printf("Hash nao existente.\n");
                    exit(8);
                    break;
                case -2:
                    page_faults++;
                    break;
                case -1:
                    page_faults++;
                    break;
                default:
                    // Atualiza bits da página
                    pagina.mod = 0;
                    pagina.ref = 1;
                    pagina.frame = page_table->itens[pos_hash]->frame;
                    pagina.used_time = operations;

                    // Atualiza TP
                    *page_table->itens[pos_hash] = pagina;
                    
                    page_hits++;
                    break;
                }
            }else{
                printf("Tipo de acesso nao existente.\n");
                exit(6);
            }

            operations++;
        }
        free(memory);
        //free(vetor_auxiliar);
    }
    else{
        printf("Falha ao alocar memoria.\n");
        exit(7);
    }
    freeHash(page_table);
    fclose(f);
    
    printf("Arquivo de entrada: %s\n", file_name);
    printf("Tamanho da memoria: %d KB\n", mem_size);
    printf("Tamanho das páginas: %d KB\n", page_size);
    printf("Tecnica de reposicao: %s\n", pol_sub);
    printf("Operacoes: %d\n", operations);
    printf("Acessos a memoria: %d\n", page_hits+page_faults);
    printf("Page faults: %d\n", page_faults);
    printf("Page hits: %d\n", page_hits);
    printf("Paginas 'sujas': %d\n", dirty_page);
}
