#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ORDEM 3

typedef struct Funcionario {
    int id;
    char nome[50];
    char cpf[12];
    int nota;
} TPessoa;

typedef struct ArvoreBMais {
    int folha;
    int n;
    int chave[ORDEM - 1];
    long ponteiros[ORDEM];
    long prox;
} NoBMais;

long escreverPessoa(TPessoa *p, FILE *arqDados);
long escreverNo(NoBMais *no, FILE *arqIndices);
void atualizarNo(long pos, NoBMais *no, FILE *arqIndices);
long buscarNoFolha(long raiz, int chave, NoBMais *no, FILE *arqIndices);

void salvarRaiz(long raiz) {
    FILE *f = fopen("BmaisRaiz.bin", "wb");
    fwrite(&raiz, sizeof(long), 1, f);
    fclose(f);
}

long carregarRaiz() {
    FILE *f = fopen("BmaisRaiz.bin", "rb");
    if (!f) return -1;
    long r;
    fread(&r, sizeof(long), 1, f);
    fclose(f);
    return r;
}

void redistribuirOuFundir(long *raiz, long posFolha, FILE *arqIndices) {
    NoBMais folha;
    fseek(arqIndices, posFolha, SEEK_SET);
    fread(&folha, sizeof(NoBMais), 1, arqIndices);

    if (folha.n >= (ORDEM - 1) / 2 || posFolha == *raiz) return;

    long posAnt = -1, pos = *raiz;
    NoBMais atual;
    while (pos != -1) {
        fseek(arqIndices, pos, SEEK_SET);
        fread(&atual, sizeof(NoBMais), 1, arqIndices);
        if (atual.folha && atual.prox == posFolha) {
            posAnt = pos;
            break;
        }
        pos = atual.prox;
    }

    if (posAnt != -1) {
        NoBMais anterior;
        fseek(arqIndices, posAnt, SEEK_SET);
        fread(&anterior, sizeof(NoBMais), 1, arqIndices);

        // Tentar redistribuição
        if (anterior.n > (ORDEM - 1) / 2) {
            for (int i = folha.n; i > 0; i--) {
                folha.chave[i] = folha.chave[i - 1];
                folha.ponteiros[i] = folha.ponteiros[i - 1];
            }
            folha.chave[0] = anterior.chave[anterior.n - 1];
            folha.ponteiros[0] = anterior.ponteiros[anterior.n - 1];
            folha.n++;
            anterior.n--;
            atualizarNo(posFolha, &folha, arqIndices);
            atualizarNo(posAnt, &anterior, arqIndices);
            return;
        }

        // Fusão
        for (int i = 0; i < folha.n; i++) {
            anterior.chave[anterior.n] = folha.chave[i];
            anterior.ponteiros[anterior.n] = folha.ponteiros[i];
            anterior.n++;
        }
        anterior.prox = folha.prox;
        atualizarNo(posAnt, &anterior, arqIndices);
    }
}

void remover(long raiz, int chave, FILE *arqIndices, FILE *arqDados) {
    if (raiz == -1) {
        printf("Arvore vazia.\n");
        return;
    }

    NoBMais no;
    long posFolha = buscarNoFolha(raiz, chave, &no, arqIndices);
    if (posFolha == -1) {
        printf("Nao encontrado para remocao.\n");
        return;
    }

    int i, j;
    for (i = 0; i < no.n; i++) {
        if (no.chave[i] == chave) {
            for (j = i; j < no.n - 1; j++) {
                no.chave[j] = no.chave[j + 1];
                no.ponteiros[j] = no.ponteiros[j + 1];
            }
            no.n--;
            atualizarNo(posFolha, &no, arqIndices);
            printf("Remocao fisica realizada.\n");
            redistribuirOuFundir(&raiz, posFolha, arqIndices);
            return;
        }
    }
    printf("Nao encontrado para remocao.\n");
}

void inserir(long *raiz, int chave, TPessoa *valor, FILE *arqDados, FILE *arqIndices) {
    if (*raiz == -1) {
        NoBMais raizInicial = {1, 0, {0}, {0}, -1};
        *raiz = escreverNo(&raizInicial, arqIndices);
        salvarRaiz(*raiz);
    }

    long posFolha;
    NoBMais folha;
    posFolha = buscarNoFolha(*raiz, chave, &folha, arqIndices);

    if (posFolha == -1) {
        printf("Erro: folha nao encontrada na arvore.\n");
        return;
    }

    if (folha.n < ORDEM - 1) {
        int i = folha.n - 1;
        while (i >= 0 && folha.chave[i] > chave) {
            folha.chave[i + 1] = folha.chave[i];
            folha.ponteiros[i + 1] = folha.ponteiros[i];
            i--;
        }
        folha.chave[i + 1] = chave;
        folha.ponteiros[i + 1] = escreverPessoa(valor, arqDados);
        folha.n++;
        atualizarNo(posFolha, &folha, arqIndices);
    } else {
        long offset = escreverPessoa(valor, arqDados);
        dividirFolha(raiz, posFolha, &folha, chave, offset, arqIndices);
    }
}

long escreverPessoa(TPessoa *p, FILE *arqDados) {
    fseek(arqDados, 0, SEEK_END);
    long pos = ftell(arqDados);
    fwrite(p, sizeof(TPessoa), 1, arqDados);
    fflush(arqDados);
    return pos;
}

long escreverNo(NoBMais *no, FILE *arqIndices) {
    fseek(arqIndices, 0, SEEK_END);
    long pos = ftell(arqIndices);
    fwrite(no, sizeof(NoBMais), 1, arqIndices);
    fflush(arqIndices);
    return pos;
}

void atualizarNo(long pos, NoBMais *no, FILE *arqIndices) {
    fseek(arqIndices, pos, SEEK_SET);
    fwrite(no, sizeof(NoBMais), 1, arqIndices);
    fflush(arqIndices);
}

long buscarNoFolha(long raiz, int chave, NoBMais *no, FILE *arqIndices) {
    if (raiz == -1) return -1;
    fseek(arqIndices, raiz, SEEK_SET);
    fread(no, sizeof(NoBMais), 1, arqIndices);

    while (!no->folha) {
        int i = 0;
        while (i < no->n && chave >= no->chave[i]) i++;
        fseek(arqIndices, no->ponteiros[i], SEEK_SET);
        fread(no, sizeof(NoBMais), 1, arqIndices);
    }

    long pos = ftell(arqIndices) - sizeof(NoBMais);
    return pos;
}

void dividirFolha(long *raiz, long posFolha, NoBMais *folha, int chave, long offset, FILE *arqIndices) {
    int tempChaves[ORDEM];
    long tempPonteiros[ORDEM];

    for (int i = 0; i < ORDEM - 1; i++) {
        tempChaves[i] = folha->chave[i];
        tempPonteiros[i] = folha->ponteiros[i];
    }

    int i = ORDEM - 2;
    while (i >= 0 && tempChaves[i] > chave) {
        tempChaves[i + 1] = tempChaves[i];
        tempPonteiros[i + 1] = tempPonteiros[i];
        i--;
    }
    tempChaves[i + 1] = chave;
    tempPonteiros[i + 1] = offset;

    int meio = ORDEM / 2;

    folha->n = 0;
    for (i = 0; i < meio; i++) {
        folha->chave[i] = tempChaves[i];
        folha->ponteiros[i] = tempPonteiros[i];
        folha->n++;
    }

    NoBMais novaFolha;
    novaFolha.folha = 1;
    novaFolha.n = 0;
    novaFolha.prox = folha->prox;
    for (i = meio; i < ORDEM; i++) {
        novaFolha.chave[novaFolha.n] = tempChaves[i];
        novaFolha.ponteiros[novaFolha.n] = tempPonteiros[i];
        novaFolha.n++;
    }

    long novaFolhaPos = escreverNo(&novaFolha, arqIndices);
    folha->prox = novaFolhaPos;
    atualizarNo(posFolha, folha, arqIndices);

    int chaveSubida = novaFolha.chave[0];
    inserirInterno(raiz, chaveSubida, posFolha, novaFolhaPos, arqIndices);
}

void inserirInterno(long *raiz, int chave, long esq, long dir, FILE *arqIndices) {
    if (*raiz == -1) {
        NoBMais novaRaiz = {0, 1, {chave}, {esq, dir}, -1};
        *raiz = escreverNo(&novaRaiz, arqIndices);
        salvarRaiz(*raiz);
        return;
    }

    NoBMais no;
    fseek(arqIndices, *raiz, SEEK_SET);
    fread(&no, sizeof(NoBMais), 1, arqIndices);

    if (no.folha) {
        NoBMais novaRaiz = {0, 1, {chave}, {esq, dir}, -1};
        *raiz = escreverNo(&novaRaiz, arqIndices);
        salvarRaiz(*raiz);
        return;
    }

    long posAtual = *raiz;
    NoBMais atual;
    while (1) {
        fseek(arqIndices, posAtual, SEEK_SET);
        fread(&atual, sizeof(NoBMais), 1, arqIndices);

        if (atual.folha) break;

        int i = 0;
        while (i < atual.n && chave >= atual.chave[i]) i++;

        long proximo = atual.ponteiros[i];
        fseek(arqIndices, proximo, SEEK_SET);
        fread(&atual, sizeof(NoBMais), 1, arqIndices);

        if (atual.n < ORDEM - 1) {
            i = atual.n - 1;
            while (i >= 0 && atual.chave[i] > chave) {
                atual.chave[i + 1] = atual.chave[i];
                atual.ponteiros[i + 2] = atual.ponteiros[i + 1];
                i--;
            }
            atual.chave[i + 1] = chave;
            atual.ponteiros[i + 1] = dir;
            atual.n++;
            atualizarNo(proximo, &atual, arqIndices);
            return;
        } else {
            dividirInterno(raiz, proximo, &atual, chave, dir, arqIndices);
            return;
        }
    }
}

void dividirInterno(long *raiz, long posNo, NoBMais *no, int chave, long dir, FILE *arqIndices) {
    int tempChaves[ORDEM];
    long tempPonteiros[ORDEM + 1];

    for (int i = 0; i < ORDEM - 1; i++) tempChaves[i] = no->chave[i];
    for (int i = 0; i < ORDEM; i++) tempPonteiros[i] = no->ponteiros[i];

    int i = ORDEM - 2;
    while (i >= 0 && tempChaves[i] > chave) {
        tempChaves[i + 1] = tempChaves[i];
        tempPonteiros[i + 2] = tempPonteiros[i + 1];
        i--;
    }

    tempChaves[i + 1] = chave;
    tempPonteiros[i + 1] = dir;

    int meio = ORDEM / 2;
    int chaveMeio = tempChaves[meio];

    no->n = 0;
    for (i = 0; i < meio; i++) {
        no->chave[i] = tempChaves[i];
        no->ponteiros[i] = tempPonteiros[i];
        no->n++;
    }
    no->ponteiros[i] = tempPonteiros[i];

    NoBMais novoNo;
    novoNo.folha = 0;
    novoNo.n = 0;
    for (i = meio + 1; i < ORDEM; i++) {
        novoNo.chave[novoNo.n] = tempChaves[i];
        novoNo.ponteiros[novoNo.n] = tempPonteiros[i];
        novoNo.n++;
    }
    novoNo.ponteiros[novoNo.n] = tempPonteiros[i];

    long novoNoPos = escreverNo(&novoNo, arqIndices);
    atualizarNo(posNo, no, arqIndices);

    inserirInterno(raiz, chaveMeio, posNo, novoNoPos, arqIndices);
}
void buscar(long raiz, int chave, FILE *arqIndices, FILE *arqDados) {
    NoBMais no;
    long posFolha = buscarNoFolha(raiz, chave, &no, arqIndices);
    if (posFolha == -1) {
        printf("Chave nao encontrada.\n");
        return;
    }

    for (int i = 0; i < no.n; i++) {
        if (no.chave[i] == chave) {
            TPessoa p;
            fseek(arqDados, no.ponteiros[i], SEEK_SET);
            fread(&p, sizeof(TPessoa), 1, arqDados);
            printf("ID: %d, Nome: %s, CPF: %s, Nota: %d\n", p.id, p.nome, p.cpf, p.nota);
            return;
        }
    }

    printf("Chave nao encontrada.\n");
}

void menu() {
    printf("\nMenu:\n");
    printf("1 - Inserir\n2 - Buscar\n3 - Remover\n4 - Sair\n");
}

int main() {
    FILE* arqDados = fopen("BmaisDados.bin", "rb+");
    FILE* arqIndices = fopen("BmaisIndices.bin", "rb+");

    if (!arqDados) arqDados = fopen("BmaisDados.bin", "wb+");
    if (!arqIndices) arqIndices = fopen("BmaisIndices.bin", "wb+");
    if (!arqDados || !arqIndices) return 1;

    long raiz = carregarRaiz();
    if (raiz == -1) {
        NoBMais raizInicial = {1, 0, {0}, {0}, -1};
        raiz = escreverNo(&raizInicial, arqIndices);
        salvarRaiz(raiz);
    }

    FILE* arqImporta = fopen("listaalunos.bin", "rb");
    if (arqImporta) {
        TPessoa aluno;
        while (fread(&aluno, sizeof(TPessoa), 1, arqImporta) == 1) {
            inserir(&raiz, aluno.id, &aluno, arqDados, arqIndices);
        }
        fclose(arqImporta);
        printf("Arquivo listaalunos.bin carregado com sucesso na árvore B+.\n");
    } else {
        printf("Arquivo listaalunos.bin não encontrado para importação.\n");
    }

    int op = 0;
    do {
        menu();
        printf("Escolha uma opcao: ");
        scanf("%d", &op);
        switch (op) {
            case 1: {
                TPessoa novo;
                printf("Digite o ID: "); scanf("%d", &novo.id);
                printf("Digite o Nome: "); scanf("%49s", novo.nome);
                printf("Digite o CPF: "); scanf("%11s", novo.cpf);
                printf("Digite a Nota: "); scanf("%d", &novo.nota);
                inserir(&raiz, novo.id, &novo, arqDados, arqIndices);
                break;
            }
            case 2: {
                int chave;
                printf("Digite o ID a buscar: ");
                scanf("%d", &chave);
                buscar(raiz, chave, arqIndices, arqDados);
                break;
            }
            case 3: {
                int chave;
                printf("Digite o ID a remover: ");
                scanf("%d", &chave);
                remover(raiz, chave, arqIndices, arqDados);
                break;
            }
            case 4:
                printf("Saindo...\n");
                break;
            default:
                printf("Opcao invalida!\n");
        }
    } while (op != 4);

    fclose(arqDados);
    fclose(arqIndices);
    return 0;
}
