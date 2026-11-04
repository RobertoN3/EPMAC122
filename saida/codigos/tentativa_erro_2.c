#include <stdio.h>

long tentativas;

void troca(int *a, int *b) { int t = *a; *a = *b; *b = t; }

/* tenta cada elemento restante na posição k; se poda != 0, descarta logo
   um candidato menor que v[k-1], pois nenhuma continuação ficaria ordenada */
int ordena_te(int v[], int n, int k, int poda) {
  int i;
  if (k == n) {                                  /* todas as posições preenchidas */
    for (i = 1; i < n; i++) if (v[i] < v[i - 1]) return 0;
    return 1;
  }
  for (i = k; i < n; i++) {
    if (poda && k > 0 && v[i] < v[k - 1]) continue;
    tentativas++;
    troca(&v[k], &v[i]);                         /* tenta v[i] na posição k */
    if (ordena_te(v, n, k + 1, poda)) return 1;
    troca(&v[k], &v[i]);                         /* não deu: desfaz (volta atrás) */
  }
  return 0;
}

void teste(const int orig[], int n, int poda) {
  int v[16], i;
  for (i = 0; i < n; i++) v[i] = orig[i];
  tentativas = 0;
  ordena_te(v, n, 0, poda);
  printf("  %s: %9ld tentativas ->", poda ? "com poda" : "sem poda", tentativas);
  for (i = 0; i < n; i++) printf(" %d", v[i]);
  printf("\n");
}

int main(void) {
  int a[] = {3, 1, 4, 2};
  int b[] = {19, 2, 3, 7, 10, 8, 5, 4, 1, 15};
  printf("n = 4\n");  teste(a, 4, 0);  teste(a, 4, 1);
  printf("n = 10\n"); teste(b, 10, 0); teste(b, 10, 1);
  return 0;
}
