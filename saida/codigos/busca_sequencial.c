#include <stdio.h>
#define MAX 20

/* contadores de comparações, para comparar os dois métodos */
long cmp_x, cmp_fim;

/* devolve o índice da primeira ocorrência de x em v[0..n-1], ou -1 */
int busca(int v[], int n, int x) {
  int j = 0;
  for (;;) {               /* invariante: x não está em v[0..j-1] */
    cmp_fim++;
    if (j >= n) return -1;
    cmp_x++;
    if (v[j] == x) return j;
    j++;
  }
}

/* v precisa ter espaço para n+1 elementos: v[n] recebe o sentinela */
int busca_sentinela(int v[], int n, int x) {
  int j = 0;
  v[n] = x;                /* sentinela: garante que o laço para */
  for (;;) {
    cmp_x++;
    if (v[j] == x) break;
    j++;
  }
  cmp_fim++;
  return j < n ? j : -1;   // parou no sentinela: x não estava no vetor
}

void testa(int v[], int n, int x) {
  int p;
  cmp_x = cmp_fim = 0;
  p = busca(v, n, x);
  printf("x = %2d  simples:   posição %2d, %2ld comparações\n", x, p, cmp_x + cmp_fim);
  cmp_x = cmp_fim = 0;
  p = busca_sentinela(v, n, x);
  printf("        sentinela: posição %2d, %2ld comparações\n", p, cmp_x + cmp_fim);
}

int main(void) {
  int v[MAX + 1] = {31, 7, 18, 42, 5, 26, 13, 9};   /* uma posição a mais */
  int n = 8;
  testa(v, n, 26);
  testa(v, n, 31);
  testa(v, n, 99);
  return 0;
}
