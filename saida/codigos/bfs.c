#include <stdio.h>
#define MAXV 10

int adj[MAXV][MAXV];   /* matriz de adjacência */
int nv = 7;            /* número de vértices */

void aresta(int u, int v) {
  adj[u][v] = adj[v][u] = 1;   /* grafo não dirigido */
}

/* Busca em largura a partir de s.
   dist[v] = -1 se v não é alcançável; pai[v] = vértice anterior no caminho */
void bfs(int s, int dist[], int pai[]) {
  int fila[MAXV], ini = 0, fim = 0, u, v;
  for (v = 0; v < nv; v++) {
    dist[v] = -1;
    pai[v] = -1;
  }
  dist[s] = 0;
  fila[fim++] = s;                       // insere a origem
  while (ini < fim) {
    u = fila[ini++];                     // remove o primeiro da fila
    for (v = 0; v < nv; v++)
      if (adj[u][v] && dist[v] == -1) {  // vizinho ainda não visitado
        dist[v] = dist[u] + 1;
        pai[v] = u;
        fila[fim++] = v;                 // marca ao entrar na fila
      }
  }
}

/* imprime o caminho de s até v seguindo o vetor pai (recursivo) */
void imprime_caminho(int pai[], int v) {
  if (pai[v] != -1) {
    imprime_caminho(pai, pai[v]);
    printf("-");
  }
  printf("%d", v);
}

int main(void) {
  int dist[MAXV], pai[MAXV], v;
  aresta(0, 1); aresta(0, 2); aresta(1, 3);
  aresta(2, 3); aresta(3, 4); aresta(4, 5);   /* vértice 6 fica isolado */
  bfs(0, dist, pai);
  for (v = 0; v < nv; v++) {
    printf("vertice %d: distancia %2d", v, dist[v]);
    if (dist[v] != -1) {
      printf("  caminho ");
      imprime_caminho(pai, v);
    }
    printf("\n");
  }
  return 0;
}
