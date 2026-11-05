#include <stdio.h>

void troca(int v[], int i, int j) {
  int t = v[i];
  v[i] = v[j];
  v[j] = t;
}

/* desce v[i] no heap v[0..n-1]; as subárvores de v[i] já são heaps */
void desce(int v[], int n, int i) {
  int f;                               /* maior filho de i */
  while (2 * i + 1 < n) {
    f = 2 * i + 1;                     /* filho esquerdo */
    if (f + 1 < n && v[f + 1] > v[f]) f++;
    if (v[i] >= v[f]) break;           /* já é heap */
    troca(v, i, f);
    i = f;
  }
}

void heapsort(int v[], int n) {
  int i, fim;
  for (i = n / 2 - 1; i >= 0; i--)     /* construção: O(n) */
    desce(v, n, i);
  for (fim = n - 1; fim > 0; fim--) {
    troca(v, 0, fim);                  /* o maior vai para o fim */
    desce(v, fim, 0);
  }
}

void imprime(const char *rotulo, int v[], int n) {
  int i;
  printf("%s", rotulo);
  for (i = 0; i < n; i++) printf(" %d", v[i]);
  printf("\n");
}

int main(void) {
  int v[] = {29, 10, 14, 37, 13, 5};
  int n = sizeof(v) / sizeof(v[0]), i;

  for (i = n / 2 - 1; i >= 0; i--) desce(v, n, i);
  imprime("heap:     ", v, n);

  heapsort(v, n);                      /* um heap também pode ser ordenado */
  imprime("ordenado: ", v, n);
  return 0;
}
