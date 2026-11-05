#include <stdio.h>
#include <stdlib.h>
#define MAXN 20

int col[MAXN];   /* col[lin] = coluna da rainha da linha lin */
int total = 0;

/* devolve 1 se a rainha em (lin, c) não é atacada pelas anteriores */
int pode(int lin, int c) {
  int i;
  for (i = 0; i < lin; i++) {
    if (col[i] == c) return 0;                 /* mesma coluna */
    if (abs(col[i] - c) == lin - i) return 0;  /* mesma diagonal */
  }
  return 1;
}

/* tentativa e erro: tenta todas as colunas da linha lin */
void rainhas(int lin, int n) {
  int c, i;
  if (lin == n) {                /* solução completa encontrada */
    total++;
    if (total == 1) {
      printf("Primeira solucao:\n");
      for (i = 0; i < n; i++) printf("linha %d -> coluna %d\n", i, col[i]);
    }
    return;
  }
  for (c = 0; c < n; c++) {
    if (pode(lin, c)) {
      col[lin] = c;              /* tenta */
      rainhas(lin + 1, n);       /* avança; ao retornar, volta atrás */
    }
  }
}

int main(void) {
  int n = 8;
  rainhas(0, n);
  printf("Total de solucoes para N=%d: %d\n", n, total);
  return 0;
}
