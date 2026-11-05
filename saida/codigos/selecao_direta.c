#include <stdio.h>

/* Ordena v[0..n-1] por seleção direta */
void selecao(int v[], int n) {
  int i, j, menor, aux;
  for (i = 0; i < n - 1; i++) {
    /* invariante: v[0..i-1] ordenado e <= v[i..n-1] */
    menor = i;
    for (j = i + 1; j < n; j++)
      if (v[j] < v[menor]) menor = j;   /* escolha gulosa: o menor restante */
    aux = v[i]; v[i] = v[menor]; v[menor] = aux;
  }
}

int main(void) {
  int v[] = {29, 10, 14, 37, 13, 5};
  int n = 6, i;
  selecao(v, n);
  for (i = 0; i < n; i++) printf("%d ", v[i]);
  printf("\n");
  return 0;
}
