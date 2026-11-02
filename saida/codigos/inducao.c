#include <stdio.h>

/* soma 0 + 1 + ... + n por um laço */
long soma_laco(int n) {
  long s = 0;
  int i;
  for (i = 1; i <= n; i++) s += i;
  return s;
}

/* a mesma soma pela definição recursiva: S(0) = 0, S(n) = n + S(n-1) */
long soma_rec(int n) {
  if (n == 0) return 0;
  return n + soma_rec(n - 1);
}

/* conta os movimentos da solução recursiva das Torres de Hanói */
long hanoi(int n) {
  if (n == 0) return 0;
  return hanoi(n - 1) + 1 + hanoi(n - 1);
}

int main(void) {
  int n, erros = 0;
  printf("  n   laço  recursiva  n(n+1)/2   Hanói  2^n - 1\n");
  for (n = 0; n <= 20; n++) {
    long f = (long)n * (n + 1) / 2, h = hanoi(n), p = (1L << n) - 1;
    if (soma_laco(n) != f || soma_rec(n) != f || h != p) erros++;
    if (n <= 5 || n % 5 == 0)
      printf("%3d %6ld %10ld %9ld %7ld %8ld\n", n, soma_laco(n), soma_rec(n), f, h, p);
  }
  printf("%s\n", erros ? "Alguma fórmula falhou!" : "As fórmulas conferem para n = 0..20.");
  /* conferir não é provar: a prova é a indução descrita na página */
  return 0;
}
