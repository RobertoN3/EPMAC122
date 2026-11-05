#include <stdio.h>

#define MAX_VERTICES 8
#define MAX_ARESTAS 16

typedef struct {
    int origem;
    int destino;
} Aresta;

typedef struct No {
    int destino;
    struct No *proximo;
} No;

typedef struct {
    int n;
    No *lista[MAX_VERTICES];
    No nos[MAX_ARESTAS];
} Grafo;

/* Os nos das listas pertencem ao proprio grafo: nao copiar a estrutura. */
int construir(Grafo *g, int n, const Aresta arestas[], size_t m)
{
    int u, v;
    size_t i;

    if (n < 0 || n > MAX_VERTICES || m > MAX_ARESTAS) {
        return 0;
    }
    g->n = n;
    for (u = 0; u < n; u++) {
        g->lista[u] = NULL;
    }
    for (i = 0; i < m; i++) {
        u = arestas[i].origem;
        v = arestas[i].destino;
        if (u < 0 || u >= n || v < 0 || v >= n) {
            return 0;
        }
        g->nos[i].destino = v;
        g->nos[i].proximo = g->lista[u];
        g->lista[u] = &g->nos[i];
    }
    return 1;
}

/* Algoritmo de Kahn: devolve quantos vertices foram processados.
   Ha ciclo se esse total for menor que g->n. */
int ordenar(const Grafo *g, int ordem[])
{
    int grau[MAX_VERTICES] = {0};
    int fila[MAX_VERTICES];
    int inicio = 0;
    int fim = 0;
    int quantidade = 0;
    int u, v;
    const No *p;

    for (u = 0; u < g->n; u++) {
        for (p = g->lista[u]; p != NULL; p = p->proximo) {
            grau[p->destino]++;
        }
    }
    for (u = 0; u < g->n; u++) {
        if (grau[u] == 0) {
            fila[fim++] = u;              /* fontes iniciais */
        }
    }
    while (inicio < fim) {
        u = fila[inicio++];
        ordem[quantidade++] = u;
        for (p = g->lista[u]; p != NULL; p = p->proximo) {
            v = p->destino;
            grau[v]--;                    /* "remove" a aresta u->v */
            if (grau[v] == 0) {
                fila[fim++] = v;
            }
        }
    }
    return quantidade;
}

/* Confere unicidade dos vertices e precedencia em todas as arestas. */
int ordem_valida(const Grafo *g, const int ordem[])
{
    int posicao[MAX_VERTICES];
    int u, i;
    const No *p;

    for (u = 0; u < g->n; u++) {
        posicao[u] = -1;
    }
    for (i = 0; i < g->n; i++) {
        u = ordem[i];
        if (u < 0 || u >= g->n || posicao[u] != -1) {
            return 0;
        }
        posicao[u] = i;
    }
    for (u = 0; u < g->n; u++) {
        for (p = g->lista[u]; p != NULL; p = p->proximo) {
            if (posicao[u] >= posicao[p->destino]) {
                return 0;
            }
        }
    }
    return 1;
}

int testar(const char *nome, const Aresta arestas[], size_t m,
           int espera_ciclo)
{
    Grafo g;
    int ordem[MAX_VERTICES];
    int quantidade, tem_ciclo, i;

    if (!construir(&g, 4, arestas, m)) {
        fprintf(stderr, "Erro ao construir o grafo.\n");
        return 0;
    }
    quantidade = ordenar(&g, ordem);
    tem_ciclo = quantidade < g.n;

    printf("%s\n", nome);
    printf("%s", tem_ciclo ? "Prefixo processado:" : "Ordem topologica:");
    for (i = 0; i < quantidade; i++) {
        printf(" %d", ordem[i] + 1);
    }
    printf("\n");
    if (tem_ciclo) {
        printf("Ciclo detectado: %d de %d vertices processados.\n",
               quantidade, g.n);
    } else if (!ordem_valida(&g, ordem)) {
        fprintf(stderr, "Erro: ordem invalida.\n");
        return 0;
    }
    if (tem_ciclo != espera_ciclo) {
        fprintf(stderr, "Erro: deteccao de ciclo diferente da esperada.\n");
        return 0;
    }
    printf("Teste OK.\n\n");
    return 1;
}

int main(void)
{
    /* Grafo G2 do material, convertido de rotulos 1..4 para indices 0..3. */
    const Aresta aciclico[] = {
        {0, 1}, {0, 3}, {1, 2}, {1, 3}, {2, 3}
    };
    const Aresta ciclico[] = {
        {0, 1}, {0, 3}, {1, 2}, {1, 3}, {2, 3},
        {3, 1} /* Acrescenta 4->2: ciclo 2->3->4->2. */
    };
    int falhas = 0;
    falhas += !testar("Grafo aciclico:", aciclico,
                      sizeof aciclico / sizeof aciclico[0], 0);
    falhas += !testar("Grafo com ciclo:", ciclico,
                      sizeof ciclico / sizeof ciclico[0], 1);
    return falhas != 0;
}
