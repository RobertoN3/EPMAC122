#include <stdio.h>
#include <limits.h>
#define NV 5
#define INF INT_MAX

/* peso[u][v] = 0 indica que não existe aresta u-v */
void dijkstra(int peso[NV][NV], int s, int dist[], int pai[]) {
  int fixado[NV], i, k, u, v;
  for (v = 0; v < NV; v++) {
    dist[v] = INF; pai[v] = -1; fixado[v] = 0;
  }
  dist[s] = 0;
  for (k = 0; k < NV; k++) {
    u = -1;                                    /* escolha gulosa */
    for (i = 0; i < NV; i++)
      if (!fixado[i] && dist[i] != INF && (u == -1 || dist[i] < dist[u]))
        u = i;
    if (u == -1) break;                        /* restantes inalcançáveis */
    fixado[u] = 1;
    for (v = 0; v < NV; v++)                   /* relaxa arestas de u */
      if (peso[u][v] && !fixado[v] && dist[u] + peso[u][v] < dist[v]) {
        dist[v] = dist[u] + peso[u][v];
        pai[v] = u;
      }
  }
}

int main(void) {
  int peso[NV][NV] = {
    {0, 10, 3, 0, 0},
    {10, 0, 1, 2, 0},
    {3, 1, 0, 8, 2},
    {0, 2, 8, 0, 7},
    {0, 0, 2, 7, 0}
  };
  int dist[NV], pai[NV], v;
  dijkstra(peso, 0, dist, pai);
  for (v = 0; v < NV; v++)
    printf("dist(0, %d) = %d   pai = %d\n", v, dist[v], pai[v]);
  return 0;
}
