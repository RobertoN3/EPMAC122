#include <stdio.h>
#define N 7

/* mesmo grafo da página da busca em largura: 0-1, 0-2, 1-3, 2-3, 3-4, 4-5; 6 isolado */
int adj[N][N];
int visitado[N], pai[N], entrada[N], saida[N], relogio = 0;

void aresta(int u, int v) { adj[u][v] = adj[v][u] = 1; }

/* versão recursiva: a pilha de execução guarda o caminho atual */
void dfs(int u) {
  int v;
  visitado[u] = 1;
  entrada[u] = ++relogio;            /* momento em que u é descoberto */
  printf(" %d", u);
  for (v = 0; v < N; v++)
    if (adj[u][v] && !visitado[v]) {
      pai[v] = u;
      dfs(v);
    }
  saida[u] = ++relogio;              /* todos os descendentes de u terminaram */
}

/* versão iterativa com pilha explícita (marca ao desempilhar) */
void dfs_pilha(int s) {
  int pilha[N * N], topo = 0, u, v, marcado[N] = {0};
  pilha[topo++] = s;
  while (topo > 0) {
    u = pilha[--topo];
    if (marcado[u]) continue;        /* já foi visitado por outro caminho */
    marcado[u] = 1;
    printf(" %d", u);
    for (v = N - 1; v >= 0; v--)     /* empilha em ordem inversa: sai o menor primeiro */
      if (adj[u][v] && !marcado[v]) pilha[topo++] = v;
  }
}

int main(void) {
  int u, comp = 0;
  aresta(0, 1); aresta(0, 2); aresta(1, 3); aresta(2, 3); aresta(3, 4); aresta(4, 5);
  for (u = 0; u < N; u++) pai[u] = -1;

  printf("DFS recursiva a partir de 0:");
  dfs(0);
  printf("\nDFS com pilha a partir de 0:");
  dfs_pilha(0);
  printf("\n\nvértice  pai  entrada/saída\n");
  for (u = 0; u < N; u++)
    if (visitado[u]) printf("%5d %5d %7d/%d\n", u, pai[u], entrada[u], saida[u]);
    else             printf("%5d     (não alcançado a partir de 0)\n", u);

  /* componentes conexas: uma DFS nova a partir de cada vértice ainda não visitado */
  for (u = 0; u < N; u++) visitado[u] = 0;
  relogio = 0;
  printf("\nComponentes conexas:\n");
  for (u = 0; u < N; u++)
    if (!visitado[u]) {
      printf("  componente %d:", ++comp);
      dfs(u);
      printf("\n");
    }
  return 0;
}
