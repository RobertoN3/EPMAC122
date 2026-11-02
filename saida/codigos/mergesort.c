#include <stdio.h>
#include <stdlib.h>

/* intercala v[ini..meio] e v[meio+1..fim] usando aux */
void intercala(int v[], int aux[], int ini, int meio, int fim) {
  int i = ini, j = meio + 1, k;
  for (k = ini; k <= fim; k++) {
    if (j > fim || (i <= meio && v[i] <= v[j]))   /* <= : estável */
      aux[k] = v[i++];
    else
      aux[k] = v[j++];
  }
  for (k = ini; k <= fim; k++) v[k] = aux[k];
}

/* Merge Sort de baixo para cima: blocos de 1, 2, 4, 8, ... */
void mergesortIterativo(int v[], int n) {
  int largura, ini, meio, fim;
  int *aux = malloc(n * sizeof(int));   // alocado uma única vez
  if (aux == NULL) return;
  for (largura = 1; largura < n; largura *= 2) {
    for (ini = 0; ini + largura < n; ini += 2 * largura) {
      meio = ini + largura - 1;
      fim = ini + 2 * largura - 1;
      if (fim > n - 1) fim = n - 1;     // último bloco pode ser menor
      intercala(v, aux, ini, meio, fim);
    }
  }
  free(aux);
}

int main(void) {
  int v[] = {38, 27, 43, 3, 9, 82, 10};
  int n = 7, i;
  mergesortIterativo(v, n);
  for (i = 0; i < n; i++) printf("%d ", v[i]);
  printf("\n");
  return 0;
}
