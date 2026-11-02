#include <stdio.h>
#define N 8

/* Ordenação bolha como método guloso:
   a cada passada, o maior elemento restante "sobe" até v[fim]
   e essa posição nunca mais é alterada (escolha definitiva). */
void bolha(int v[], int n) {
  int fim, i, aux, trocou;
  for (fim = n - 1; fim > 0; fim--) {
    trocou = 0;
    for (i = 0; i < fim; i++) {
      if (v[i] > v[i + 1]) {   /* vizinhos fora de ordem */
        aux = v[i];
        v[i] = v[i + 1];
        v[i + 1] = aux;
        trocou = 1;
      }
    }
    if (!trocou) break;        // nenhuma troca: já está ordenado
  }
}

int main(void) {
  int v[N] = {5, 2, 9, 1, 7, 3, 8, 4};
  int i;
  bolha(v, N);
  printf("Vetor ordenado: ");
  for (i = 0; i < N; i++)
    printf("%d ", v[i]);
  printf("\n");
  return 0;
}
