#include <stdio.h>
#include <stdlib.h>
#define MAXV 10

typedef struct {
  int u, v, peso;
} Aresta;

int pai[MAXV];

/* union-find com compressão de caminhos */
int find(int x) {
  if (pai[x] != x) pai[x] = find(pai[x]);
  return pai[x];
}

int compara(const void *a, const void *b) {
  return ((Aresta *)a)->peso - ((Aresta *)b)->peso;
}

/* devolve o peso total da árvore espalhada mínima */
int kruskal(int nv, Aresta e[], int na) {
  int i, escolhidas = 0, total = 0;
  for (i = 0; i < nv; i++) pai[i] = i;       /* cada vértice isolado */
  qsort(e, na, sizeof(Aresta), compara);     /* ordena por peso */
  for (i = 0; i < na && escolhidas < nv - 1; i++) {
    int ru = find(e[i].u), rv = find(e[i].v);
    if (ru != rv) {                          // não forma ciclo
      pai[ru] = rv;                          // union
      total += e[i].peso;
      escolhidas++;
      printf("aresta %d-%d (peso %d)\n", e[i].u, e[i].v, e[i].peso);
    }
  }
  return total;
}

int main(void) {
  Aresta e[] = {{0,1,4}, {0,2,3}, {1,2,1}, {1,3,2}, {2,3,4}, {3,4,2}, {4,5,6}, {3,5,7}};
  int na = sizeof(e) / sizeof(e[0]);
  printf("Peso total: %d\n", kruskal(6, e, na));
  return 0;
}
