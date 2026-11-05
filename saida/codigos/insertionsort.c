#include <stdio.h>

void imprime(int v[], int n) {
  int i;
  for (i = 0; i < n; i++) printf("%d ", v[i]);
  printf("\n");
}

/* Ordena v[0..n-1] por inserção direta */
void insercao(int v[], int n) {
  int i, j, x;
  for (i = 1; i < n; i++) {
    /* invariante: v[0..i-1] está em ordem crescente */
    x = v[i];
    for (j = i - 1; j >= 0 && v[j] > x; j--)
      v[j + 1] = v[j];            /* desloca os maiores para a direita */
    v[j + 1] = x;                 /* encaixa x no espaço aberto */
    printf("i = %d: ", i);
    imprime(v, n);
  }
}

int main(void) {
  int v[] = {12, 11, 13, 5, 6};
  insercao(v, 5);
  return 0;
}
