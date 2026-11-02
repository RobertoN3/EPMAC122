#include <stdio.h>
#include <stdlib.h>

/* Intercala v[ini..meio] e v[meio+1..fim], ambos já ordenados */
void intercala(int v[], int ini, int meio, int fim) {
  int i = ini, j = meio + 1, k = 0;
  int *aux = malloc((fim - ini + 1) * sizeof(int));
  while (i <= meio && j <= fim) {
    if (v[i] <= v[j]) aux[k++] = v[i++];  /* <= mantém a estabilidade */
    else              aux[k++] = v[j++];
  }
  while (i <= meio) aux[k++] = v[i++];
  while (j <= fim)  aux[k++] = v[j++];
  for (k = 0; k < fim - ini + 1; k++)
    v[ini + k] = aux[k];
  free(aux);
}

/* Divisão e conquista: dividir, conquistar, combinar */
void mergesort(int v[], int ini, int fim) {
  if (ini < fim) {                    // caso base: 0 ou 1 elemento
    int meio = (ini + fim) / 2;       // dividir
    mergesort(v, ini, meio);          // conquistar a 1a metade
    mergesort(v, meio + 1, fim);      // conquistar a 2a metade
    intercala(v, ini, meio, fim);     // combinar
  }
}

int main(void) {
  int v[] = {38, 27, 43, 3, 9, 82, 10};
  int n = sizeof(v) / sizeof(v[0]), i;
  mergesort(v, 0, n - 1);
  for (i = 0; i < n; i++)
    printf("%d ", v[i]);
  printf("\n");
  return 0;
}
