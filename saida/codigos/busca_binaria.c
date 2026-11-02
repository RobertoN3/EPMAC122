#include <stdio.h>

/* devolve o índice de x em v[0..n-1] (crescente), ou -1 */
int buscaBinaria(int v[], int n, int x) {
  int ini = 0, fim = n - 1, m;
  while (ini <= fim) {
    m = ini + (fim - ini) / 2;     /* evita estouro de ini+fim */
    if (v[m] == x) return m;
    if (x < v[m]) fim = m - 1;     // descarta a metade direita
    else          ini = m + 1;     // descarta a metade esquerda
  }
  return -1;
}

/* versão recursiva no intervalo fechado [ini, fim] */
int buscaRec(int v[], int ini, int fim, int x) {
  int m;
  if (ini > fim) return -1;        // trecho vazio: não está
  m = (ini + fim) / 2;
  if (v[m] == x) return m;
  if (x < v[m]) return buscaRec(v, ini, m - 1, x);
  return buscaRec(v, m + 1, fim, x);
}

int main(void) {
  int v[] = {2, 5, 8, 12, 16, 23, 38, 56, 72, 91};
  printf("23 esta na posicao %d\n", buscaBinaria(v, 10, 23));
  printf("40 esta na posicao %d\n", buscaRec(v, 0, 9, 40));
  return 0;
}
