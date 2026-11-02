#include <stdio.h>

typedef struct No {
    int valor;
    struct No *esquerdo;
    struct No *direito;
} No;

size_t contar_nos(const No *raiz)
{
    if (raiz == NULL) {
        return 0; /* Caso base: arvore vazia. */
    }
    return 1 + contar_nos(raiz->esquerdo) + contar_nos(raiz->direito);
}

size_t contar_folhas(const No *raiz)
{
    if (raiz == NULL) {
        return 0; /* Arvore vazia nao tem folhas. */
    }
    if (raiz->esquerdo == NULL && raiz->direito == NULL) {
        return 1; /* Uma folha conta a si mesma. */
    }
    return contar_folhas(raiz->esquerdo) + contar_folhas(raiz->direito);
}

int altura(const No *raiz)
{
    if (raiz == NULL) {
        return -1; /* Faz a altura de uma folha ser zero. */
    }
    int esquerda = altura(raiz->esquerdo);
    int direita = altura(raiz->direito);
    return 1 + (esquerda > direita ? esquerda : direita);
}

/* Imprime os resultados e retorna 1 quando todos estao corretos. */
int conferir(const char *nome, const No *raiz,
             size_t nos_esperados, size_t folhas_esperadas,
             int altura_esperada)
{
    size_t nos = contar_nos(raiz);
    size_t folhas = contar_folhas(raiz);
    int h = altura(raiz);
    int correto = nos == nos_esperados && folhas == folhas_esperadas &&
                  h == altura_esperada;

    printf("%s: nos = %zu, folhas = %zu, altura = %d [%s]\n",
           nome, nos, folhas, h, correto ? "OK" : "ERRO");
    if (!correto) {
        fprintf(stderr, "Esperado: nos = %zu, folhas = %zu, altura = %d\n",
                nos_esperados, folhas_esperadas, altura_esperada);
    }
    return correto;
}

int main(void)
{
    No nos[] = {
        {1, NULL, NULL}, {2, NULL, NULL}, {3, NULL, NULL},
        {4, NULL, NULL}, {5, NULL, NULL}, {6, NULL, NULL}
    };
    No unico = {1, NULL, NULL};
    int falhas = 0;

    nos[0].esquerdo = &nos[1];
    nos[0].direito = &nos[2];
    nos[1].esquerdo = &nos[3];
    nos[2].esquerdo = &nos[4];
    nos[2].direito = &nos[5];

    // Todos os testes sao executados, mesmo se algum falhar.
    falhas += !conferir("Arvore de seis nos", &nos[0], 6, 3, 2);
    falhas += !conferir("Arvore vazia", NULL, 0, 0, -1);
    falhas += !conferir("Arvore com um no", &unico, 1, 1, 0);

    if (falhas != 0) {
        return 1;
    }
    printf("Todos os resultados conferem.\n");
    return 0;
}
