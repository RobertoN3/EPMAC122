#include <stdio.h>

typedef struct {
    char nome[32];
    int nota;
} Aluno;

/* Estavel: registros com nota igual nao sao deslocados pela chave. */
void insertion_sort(Aluno vetor[], size_t n)
{
    size_t i;

    for (i = 1; i < n; i++) {
        Aluno chave = vetor[i];
        size_t j = i;

        while (j > 0 && vetor[j - 1].nota > chave.nota) {
            vetor[j] = vetor[j - 1];
            j--;
        }
        vetor[j] = chave;
    }
}

/* Instavel: a troca distante pode inverter registros de notas iguais.
   Mesmo selecionar o primeiro minimo nao impede essa inversao. */
void selecao_direta(Aluno vetor[], size_t n)
{
    size_t i, j;

    for (i = 0; i < n; i++) {
        size_t menor = i;

        for (j = i + 1; j < n; j++) {
            if (vetor[j].nota < vetor[menor].nota) {
                menor = j;
            }
        }
        if (menor != i) {
            Aluno auxiliar = vetor[i];
            vetor[i] = vetor[menor];
            vetor[menor] = auxiliar;
        }
    }
}

void imprimir(const char *titulo, const Aluno vetor[], size_t n)
{
    size_t i;

    printf("%s\n", titulo);
    for (i = 0; i < n; i++) {
        printf("  %s: nota %d\n", vetor[i].nome, vetor[i].nota);
    }
    printf("\n");
}

int main(void)
{
    const Aluno original[] = {
        {"Ana", 8},
        {"Bruno", 8},
        {"Carla", 6}
    };
    Aluno estavel[sizeof original / sizeof original[0]];
    Aluno instavel[sizeof original / sizeof original[0]];
    const size_t n = sizeof original / sizeof original[0];
    size_t i;

    /* As duas ordenacoes recebem a mesma sequencia inicial. */
    for (i = 0; i < n; i++) {
        estavel[i] = original[i];
        instavel[i] = original[i];
    }

    insertion_sort(estavel, n);
    selecao_direta(instavel, n);

    imprimir("Entrada original:", original, n);
    imprimir("Insertion Sort estavel, por nota crescente:", estavel, n);
    imprimir("Selecao Direta instavel, por nota crescente:", instavel, n);
    printf("Ana e Bruno empatam na nota. Observe a ordem dos nomes.\n");
    return 0;
}
