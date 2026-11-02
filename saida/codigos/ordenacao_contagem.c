#include <stdio.h>
#include <string.h>
#define U 6           /* universo: valores de 0 a U-1 */
#define N 8

/* ordena v[0..n-1], com valores em 0..U-1, por contagem (estável) */
void ordena_contagem(int v[], int n) {
  int freq[U + 1] = {0}, aux[N], i;
  for (i = 0; i < n; i++) freq[v[i] + 1]++;       /* 1) contar (índice + 1) */
  for (i = 1; i <= U; i++) freq[i] += freq[i - 1]; /* 2) acumular */
  for (i = 0; i < n; i++) aux[freq[v[i]]++] = v[i]; // 3) distribuir
  for (i = 0; i < n; i++) v[i] = aux[i];
}

/* bolha sobre um vetor de ordem: troca índices, não os nomes */
void bolha_ordem(const char *nome[], int ordem[], int n) {
  int fim, i, t;
  for (fim = n - 1; fim > 0; fim--)
    for (i = 0; i < fim; i++)
      if (strcmp(nome[ordem[i]], nome[ordem[i + 1]]) > 0) {
        t = ordem[i]; ordem[i] = ordem[i + 1]; ordem[i + 1] = t;
      }
}

int main(void) {
  int v[N] = {3, 1, 4, 1, 5, 2, 3, 0}, i;
  const char *nome[] = {"Pedro", "Ana", "Lucas", "Beatriz", "Joana"};
  int ordem[5] = {0, 1, 2, 3, 4};

  ordena_contagem(v, N);
  printf("contagem:");
  for (i = 0; i < N; i++) printf(" %d", v[i]);
  printf("\n\n");

  bolha_ordem(nome, ordem, 5);
  printf("vetor de ordem:");
  for (i = 0; i < 5; i++) printf(" %d", ordem[i]);
  printf("\nnomes em ordem:");
  for (i = 0; i < 5; i++) printf(" %s", nome[ordem[i]]);
  printf("\nvetor original:");
  for (i = 0; i < 5; i++) printf(" %s", nome[i]);
  printf("\n");
  return 0;
}
