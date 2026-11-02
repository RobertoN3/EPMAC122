#include <stdio.h>

void troca(int v[], int a, int b) {
  int aux = v[a]; v[a] = v[b]; v[b] = aux;
}

/* Partição de Lomuto: o pivô (elemento do meio) é levado para v[fim].
   Devolve p tal que v[ini..p-1] <= v[p] < v[p+1..fim]. */
int particiona(int v[], int ini, int fim) {
  int x, i = ini - 1, j;
  troca(v, (ini + fim) / 2, fim);    /* pivô do meio: evita O(n²) em vetor ordenado */
  x = v[fim];
  for (j = ini; j < fim; j++) {
    if (v[j] <= x) {
      i++;
      troca(v, i, j);
    }
  }
  troca(v, i + 1, fim);              // pivô na posição definitiva
  return i + 1;
}

void quicksort(int v[], int ini, int fim) {
  int p;
  if (ini >= fim) return;            // 0 ou 1 elemento
  p = particiona(v, ini, fim);       // dividir (todo o trabalho está aqui)
  quicksort(v, ini, p - 1);
  quicksort(v, p + 1, fim);          // combinar: nada a fazer
}

int main(void) {
  int v[] = {7, 2, 1, 6, 8, 5, 3, 4};
  int n = 8, i;
  quicksort(v, 0, n - 1);
  for (i = 0; i < n; i++) printf("%d ", v[i]);
  printf("\n");
  return 0;
}
