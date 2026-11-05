#include <stdio.h>
#define MAXN 10
#define MAXC 100

/* Fibonacci de baixo para cima: cada valor é calculado uma vez */
long fib(int n) {
  long f[90];
  int i;
  f[0] = 0; f[1] = 1;
  for (i = 2; i <= n; i++)
    f[i] = f[i - 1] + f[i - 2];
  return f[n];
}

/* Mochila 0/1: M[i][c] = melhor valor com os i primeiros objetos */
int mochila(int n, int C, int peso[], int valor[]) {
  static int M[MAXN + 1][MAXC + 1];
  int i, c;
  for (c = 0; c <= C; c++) M[0][c] = 0;
  for (i = 1; i <= n; i++) {
    for (c = 0; c <= C; c++) {
      M[i][c] = M[i - 1][c];                         /* não leva o objeto i */
      if (peso[i] <= c && valor[i] + M[i - 1][c - peso[i]] > M[i][c])
        M[i][c] = valor[i] + M[i - 1][c - peso[i]];  /* leva o objeto i */
    }
  }
  return M[n][C];
}

int main(void) {
  /* índice 0 não é usado: objetos numerados de 1 a n */
  int peso[]  = {0, 1, 3, 4, 5};
  int valor[] = {0, 1, 4, 5, 7};
  printf("fib(40) = %ld\n", fib(40));
  printf("Melhor valor (C=7): %d\n", mochila(4, 7, peso, valor));
  return 0;
}
