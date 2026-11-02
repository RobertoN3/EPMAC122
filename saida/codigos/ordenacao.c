#include <stdio.h>

typedef struct {
  int primaria, secundaria;
} Par;

/* inserção direta por uma das chaves: é estável, pois só desloca
   elementos ESTRITAMENTE maiores que o elemento sendo inserido */
void insercao(Par v[], int n, int pelaPrimaria) {
  int i, j;
  Par x;
  for (i = 1; i < n; i++) {
    x = v[i];
    j = i - 1;
    while (j >= 0 && (pelaPrimaria ? v[j].primaria > x.primaria
                                   : v[j].secundaria > x.secundaria)) {
      v[j + 1] = v[j];
      j--;
    }
    v[j + 1] = x;
  }
}

void imprime(const char *rotulo, Par v[], int n) {
  int i;
  printf("%s: ", rotulo);
  for (i = 0; i < n; i++) printf("(%d,%d) ", v[i].primaria, v[i].secundaria);
  printf("\n");
}

int main(void) {
  Par v[] = {{3,6}, {7,5}, {3,5}, {6,2}, {9,1}};
  int n = 5;
  imprime("L1", v, n);
  insercao(v, n, 0);   // primeiro pela chave secundária
  imprime("L2", v, n);
  insercao(v, n, 1);   // depois, de forma estável, pela primária
  imprime("L3", v, n);
  return 0;
}
