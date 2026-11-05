#include <stdio.h>
#include <limits.h>
#define MAXV 10
#define INF INT_MAX

int peso[MAXV][MAXV];   /* 0 indica que não há aresta */

void aresta(int u, int v, int p) {
  peso[u][v] = peso[v][u] = p;          /* grafo não dirigido */
}

/* Prim a partir de s, versão O(n²); devolve o peso total */
int prim(int nv, int s) {
  int custo[MAXV], pai[MAXV], na_arvore[MAXV];
  int i, u, v, total = 0;

  for (v = 0; v < nv; v++) {
    custo[v] = INF;
    pai[v] = -1;
    na_arvore[v] = 0;
  }
  custo[s] = 0;

  for (i = 0; i < nv; i++) {
    u = -1;                             /* vértice fora da árvore de menor custo */
    for (v = 0; v < nv; v++)
      if (!na_arvore[v] && (u == -1 || custo[v] < custo[u]))
        u = v;
    na_arvore[u] = 1;
    if (pai[u] != -1) {
      printf("aresta %d-%d (peso %d)\n", pai[u], u, peso[pai[u]][u]);
      total += peso[pai[u]][u];
    }
    for (v = 0; v < nv; v++)            /* atualiza os vizinhos de u */
      if (peso[u][v] && !na_arvore[v] && peso[u][v] < custo[v]) {
        custo[v] = peso[u][v];
        pai[v] = u;
      }
  }
  return total;
}

int main(void) {
  aresta(0, 1, 4); aresta(0, 2, 3); aresta(1, 2, 1); aresta(1, 3, 2);
  aresta(2, 3, 4); aresta(3, 4, 2); aresta(4, 5, 6); aresta(3, 5, 7);
  printf("Peso total: %d\n", prim(6, 0));
  return 0;
}
