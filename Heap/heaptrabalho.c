#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct Funcionario {
    int id;
    char nome[50];
    char cpf[12];
    int nota;
} TPessoa;

int Pai(int i){
    return (i/2);
}
int Esq(int i){
    return (i*2);
}
int Dir(int i){
    return (i*2+1);
}

void subirHeap (TPessoa *heap, int final){
    int novo = Pai(final);
    if (novo >= 1){
        if ((heap[final]).nota > (heap[novo]).nota){
            TPessoa temp = heap[final];
            heap[final] = heap[novo];
            heap[novo] = temp;
            subirHeap (heap, novo);
        }
    }
}

void descerHeap(TPessoa *heap, int i, int final) {
    int esq = Esq(i);
    int dir = Dir(i);
    int maior = i;

    if (esq <= final && heap[esq].nota > heap[maior].nota)
        maior = esq;

    if (dir <= final && heap[dir].nota > heap[maior].nota)
        maior = dir;

    if (maior != i) {
        TPessoa temp = heap[i];
        heap[i] = heap[maior];
        heap[maior] = temp;
        descerHeap(heap, maior, final);
    }
}

void inserirHeap (TPessoa *heap, int *final, TPessoa registro){
    heap[*final] = registro;
    subirHeap(heap, *final);
    (*final)++;
    printf("Registro inserido com sucesso!\n");
}

void excluirHeap (TPessoa *heap, int *final){
    heap[1] = heap[*final - 1];
    (*final)--;
    descerHeap(heap, 1, *final);
    printf("Registro excluido com sucesso!\n");
}

void encherLista (TPessoa *heap, int *final){
    FILE *arqLista;
    TPessoa registro;

    arqLista = fopen ("listaalunos.bin", "rb");
    if (arqLista == NULL) {
        printf("Erro ao abrir arquivo!\n");
        return;
    }

    for (int i = 1; i <= 10000; i++) {
        if (fread(&registro, sizeof(TPessoa), 1, arqLista) == 1) {
            inserirHeap(heap, final, registro);
            printf ("\nRegistro[%d] inserido com SUCESSO no arquivo!", i);
        } else {
            break;  // EOF ou erro
        }
    }

    fclose(arqLista);
}

void imprimirHeap(TPessoa *heap, int final) {
    for (int i = 1; i < final; i++) {
        printf("ID: %d, Nome: %s, CPF: %s, Nota: %d\n", heap[i].id, heap[i].nome, heap[i].cpf, heap[i].nota);
    }
}

void menu() {
    printf("\n\n");
    printf("1. Inserir registro\n");
    printf("2. Excluir registro\n");
    printf("3. Imprimir heap\n");
    printf("4. Consultar heap\n");
    printf("5. Tamanho do heap\n");
    printf("6. Sair\n");
}

int main(){
    FILE *arq;
    
    int final = 1, op = 1;
    TPessoa heap[10001]; 
    encherLista (heap, &final);

    while (op != 6) {
        menu();
        printf("Escolha uma opcao: ");
        scanf("%d", &op);

        switch (op) {
            case 1: {
                if (final >= 10000) {
                    printf("Heap cheia! Nao e possivel inserir mais registros.\n");
                    break;
                }
                TPessoa registro;
                printf("Digite o ID: ");
                scanf("%d", &registro.id);
                printf("Digite o Nome: ");
                scanf("%s", registro.nome);
                printf("Digite o CPF: ");
                scanf("%s", registro.cpf);
                printf("Digite a Nota: ");
                scanf("%d", &registro.nota);
                inserirHeap(heap, &final, registro);
                break;
            }
            case 2:
                excluirHeap(heap, &final);
                break;
            case 3:
                imprimirHeap(heap, final);
                break;
            case 4:
                printf("Registro consultado: ID: %d, Nome: %s, CPF: %s, Nota: %d\n", heap[1].id, heap[1].nome, heap[1].cpf, heap[1].nota);
                break;
            case 5:
                printf("Tamanho do heap: %d\n", final - 1);
                break;
            case 6:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }
    }

    arq = fopen ("trabalhoheap.bin", "wb");
    if (arq == NULL) {
        printf("Erro ao abrir arquivo!\n");
        return 0;
    }

    for (int i = 1 ; i < final ; i++){
        fwrite(&heap[i], sizeof(TPessoa), 1, arq);
    }

    fclose(arq);

    printf("Heap com %d registros salva com sucesso.\n", final - 1);


    return 0;
}