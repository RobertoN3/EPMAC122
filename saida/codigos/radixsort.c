#include <stdio.h>
#include <stdlib.h>
#define BASE 10

void imprime(const char *rotulo, int v[], int n) {
  int i;
  printf("%-12s", rotulo);
  for (i = 0; i < n; i++) printf("%d ", v[i]);
  printf("\n");
}

/* Counting Sort estável pelo dígito de peso 'exp' (1, 10, 100, ...) */
void contagem(int v[], int aux[], int n, int exp) {
  int cont[BASE] = {0}, i, d;
  for (i = 0; i < n; i++) cont[(v[i] / exp) % BASE]++;     /* contar */
  for (d = 1; d < BASE; d++) cont[d] += cont[d - 1];       /* acumular */
  for (i = n - 1; i >= 0; i--) {         // de trás para a frente: estável
    d = (v[i] / exp) % BASE;
    aux[--cont[d]] = v[i];
  }
  for (i = 0; i < n; i++) v[i] = aux[i];
}

/* Radix Sort LSD para inteiros não negativos */
void radixsort(int v[], int n) {
  int maior = v[0], exp, i;
  int *aux = malloc(n * sizeof(int));
  if (aux == NULL) return;
  for (i = 1; i < n; i++) if (v[i] > maior) maior = v[i];
  for (exp = 1; maior / exp > 0; exp *= BASE) {
    contagem(v, aux, n, exp);
    printf("exp = %-6d", exp);
    imprime("", v, n);
  }
  free(aux);
}

int main(void) {
  int v[] = {170, 45, 75, 90, 802, 24, 2, 66};
  int n = 8;
  imprime("original:", v, n);
  radixsort(v, n);
  return 0;
}
