#include <stdio.h>
#include <stdlib.h>
#define MAXV 8

/* Representação 1: matriz de adjacência */
int adj[MAXV][MAXV];

/* Representação 2: vetor de listas de adjacência */
typedef struct no {
  int vizinho;
  struct no *prox;
} No;

No *lista[MAXV];
int nv = 5;                     /* número de vértices */

/* insere v no início da lista de u: custo O(1) */
void insere_lista(int u, int v) {
  No *novo = malloc(sizeof(No));
  if (novo == NULL) exit(1);
  novo->vizinho = v;
  novo->prox = lista[u];
  lista[u] = novo;
}

/* grafo não dirigido: a aresta u-v vale nos dois sentidos */
void aresta(int u, int v) {
  adj[u][v] = adj[v][u] = 1;
  insere_lista(u, v);
  insere_lista(v, u);
}

int main(void) {
  int u, v;
  No *p, *q;
  aresta(0, 1); aresta(0, 2); aresta(1, 3);
  aresta(2, 3); aresta(3, 4);

  printf("Matriz de adjacencia:\n");
  for (u = 0; u < nv; u++) {
    for (v = 0; v < nv; v++) printf("%d ", adj[u][v]);
    printf("\n");
  }

  printf("Listas de adjacencia:\n");
  for (u = 0; u < nv; u++) {
    printf("%d:", u);
    for (p = lista[u]; p != NULL; p = p->prox)   // percorre os vizinhos
      printf(" %d", p->vizinho);
    printf("\n");
  }

  for (u = 0; u < nv; u++)                       /* libera as listas */
    for (p = lista[u]; p != NULL; p = q) {
      q = p->prox;
      free(p);
    }
  return 0;
}
