#include <stdio.h>

/* Ordenação bolha com parada antecipada.
   Devolve o número de trocas (= número de inversões do vetor). */
int bolha(int v[], int n) {
  int fim, i, aux, trocou, trocas = 0;
  for (fim = n - 1; fim > 0; fim--) {
    trocou = 0;
    for (i = 0; i < fim; i++) {
      if (v[i] > v[i + 1]) {      /* par de vizinhos fora de ordem */
        aux = v[i]; v[i] = v[i + 1]; v[i + 1] = aux;
        trocou = 1;
        trocas++;
      }
    }
    if (!trocou) break;           // passada sem trocas: está ordenado
  }
  return trocas;
}

void imprime(int v[], int n) {
  int i;
  for (i = 0; i < n; i++) printf("%d ", v[i]);
  printf("\n");
}

int main(void) {
  int v[] = {5, 1, 4, 2, 8};
  int n = 5, t;
  t = bolha(v, n);
  imprime(v, n);
  printf("Trocas (inversoes): %d\n", t);
  return 0;
}
