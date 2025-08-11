#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define TAM 100000

typedef struct Funcionario {
    int id;
    char nome[50];
    char cpf[12];
    int nota;
} TPessoa;

int hash(int chave){
    return (rand() % TAM);
}

int buscaHash ( FILE *arq, int chave, int *achou ){
    *achou = 0;
    int final = -1;
    int pos = -1;
    int tentativas = 0;
    TPessoa novo; 
    srand(chave);

    while (tentativas < TAM){
        final = hash(chave);
        fseek(arq, final * sizeof(TPessoa), SEEK_SET);
        fread(&novo, sizeof(TPessoa), 1, arq);
        if (novo.id == chave && novo.nota != -1){
            *achou = 1;
            tentativas = TAM;
        }
        else{
            if (novo.nota == -1){
                if (pos == -1) pos = final;
            }
            tentativas++;
        }
    }
    if (*achou) return final;
    else return pos;
}

void inserirHash ( FILE *arquivo, int chave, TPessoa novo){
    int achou = 0, pos;
    pos = buscaHash(arquivo, chave, &achou);
    if (achou) {
        printf("Aluno ja incluso na tabela!\n");
        return;
    }
    if (pos == -1) {
        printf("Tabela cheia!\n");
        return;
    }
    fseek(arquivo, pos * sizeof(TPessoa), SEEK_SET);
    fwrite(&novo, sizeof(TPessoa), 1, arquivo);
    return;
}

void excluiHash (FILE *arquivo, int chave){
    int achou = 0, pos;
    pos = buscaHash(arquivo, chave, &achou);
    if (achou) {
        TPessoa vazio = {-1, "", "", -1};
        fseek(arquivo, pos * sizeof(TPessoa), SEEK_SET);
        fwrite(&vazio, sizeof(TPessoa), 1, arquivo);
        printf ("\nRegistro REMOVIDO com SUCESSO no arquivo!");
    } else {
        printf("Aluno nao encontrado. ERRO EM REMOVER!");
    }
    return;
}

void encherHash ( FILE *arquivo ){
    FILE *arq = fopen("listaalunos.bin", "rb");
    if (arq == NULL) {
        printf("Erro ao abrir arquivo!\n");
        return 0;
    }
    
    TPessoa novo;
    int i=1;
    while (fread(&novo, sizeof(TPessoa), 1, arq) == 1) {
        inserirHash(arquivo, novo.id, novo);
        printf ("\nRegistro[%d] INSERIDO com SUCESSO no arquivo!", i);
        i++;
    }
    
    fclose(arq);
}

void menu() {
    printf("\n\nMenu de Opcoes:\n");
    printf("1. Inserir Registro\n");
    printf("2. Excluir Registro\n");
    printf("3. Consultar Registro\n");
    printf("4. Imprimir Tabela Hash\n");
    printf("5. Sair\n");
}

int main() {
    FILE *arquivo = fopen("trabalhotabelahash.bin", "wb+");

    TPessoa vazio = {-1, "", "", -1};
    for (int i = 0; i < TAM; i++)
        fwrite(&vazio, sizeof(TPessoa), 1, arquivo);

    encherHash(arquivo);

    int op = 0;
    while (op != 5) {
        menu();
        printf("Escolha uma opcao: ");
        scanf("%d", &op);

        switch (op) {
            case 1: {
                TPessoa novo;
                printf("Digite o Nome: ");
                scanf("%49s", novo.nome);
                printf("Digite o CPF: ");
                scanf("%11s", novo.cpf);
                char strid[10];
                strncpy(strid, novo.cpf, 9);
                strid[9] = '\0';
                novo.id = atoi(strid);
                printf("Digite a Nota: ");
                scanf("%d", &novo.nota);
                inserirHash(arquivo, novo.id, novo);
                break;
            }
            case 2: {
                int id;
                printf("Digite o ID do registro a ser excluido: ");
                scanf("%d", &id);
                excluiHash(arquivo, id);
                break;
            }
            case 3: {
                int id, achou = 0, pos;
                printf("Digite o ID do registro a ser consultado: ");
                scanf("%d", &id);
                pos = buscaHash(arquivo, id, &achou);
                if (achou) {
                    TPessoa registro;
                    fseek(arquivo, pos * sizeof(TPessoa), SEEK_SET);
                    fread(&registro, sizeof(TPessoa), 1, arquivo);
                    printf("Registro encontrado: ID: %d, Nome: %s, CPF: %s, Nota: %d\n",
                           registro.id, registro.nome, registro.cpf, registro.nota);
                } else {
                    printf("Registro nao encontrado.\n");
                }
                break;
            }
            case 4: {
                TPessoa registro;
                fseek(arquivo, 0, SEEK_SET);
                for (int i = 0; i < TAM; i++) {
                    fread(&registro, sizeof(TPessoa), 1, arquivo);
                    if (registro.nota != -1) {
                        printf("ID: %d, Nome: %s, CPF: %s, Nota: %d\n",
                               registro.id, registro.nome, registro.cpf, registro.nota);
                    }
                }
                break;
            }
            case 5:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }
    }
    fclose(arquivo);

    return 0;
}