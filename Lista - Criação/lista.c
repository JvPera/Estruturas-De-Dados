#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Funcionario {
    int id;
    char nome[50];
    char cpf[12];
    int nota;
} TPessoa;

void gerarCPF (char *cpf){
    for (int i=0 ; i<11 ; i++){
        cpf[i] = '0' + rand() % 10;
    }
    cpf[11] = '\0';
}

TPessoa gerarPessoa (int id){
    TPessoa novo;

    // Criando nome:
    sprintf(novo.nome, "Aluno%d", id);

    // Criando CPF:
    gerarCPF (novo.cpf);

    // Criando id:
    char strid[10];
    strncpy(strid, novo.cpf, 9);
    strid[9] = '\0';
    novo.id = atoi(strid);

    // Criando Nota:
    novo.nota = rand() % 101;

    return novo;
}   

void CriarArquivo(){
    FILE *arq;

    arq = fopen ("listaalunos.bin", "wb");
    if (arq == NULL) {
        printf("Erro ao abrir arquivo!\n");
        exit(1);
    }
    
    for (int i=1 ; i<=10000 ; i++){
        TPessoa novo = gerarPessoa (i);
        fwrite(&novo, sizeof(TPessoa), 1, arq);
        printf ("\nPessoa[%d] criada com SUCESSO!", i);
    }
    fclose(arq);
}


int main (void){

    srand(time(NULL));

    CriarArquivo();

    printf ("\n\nARQUIVO CRIADO COM SUCESSO!");

    return 0;
}
