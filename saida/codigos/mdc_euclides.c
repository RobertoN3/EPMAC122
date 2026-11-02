#include <stdio.h>

long passos;               /* contador de iterações */

/* testa os candidatos de min(a,b) para baixo (a, b > 0) */
int mdc_ingenuo(int a, int b) {
  int d = a < b ? a : b;
  for (;;) {               /* invariante: nenhum inteiro > d divide a e b */
    passos++;
    if (a % d == 0 && b % d == 0) return d;
    d--;
  }
}

/* algoritmo de Euclides; se mostrar != 0, imprime cada divisão */
int mdc_euclides(int a, int b, int mostrar) {
  while (b != 0) {         /* invariante: mdc(a,b) = mdc dos valores iniciais */
    int r = a % b;
    passos++;
    if (mostrar) printf("  %4d = %d x %3d + %3d\n", a, a / b, b, r);
    a = b;
    b = r;
  }
  return a;
}

int mdc_rec(int a, int b) {
  return b == 0 ? a : mdc_rec(b, a % b);   // recursão de cauda
}

void compara(int a, int b) {
  int g1, g2;
  passos = 0; g1 = mdc_ingenuo(a, b);
  printf("mdc(%d, %d): ingênuo = %d em %ld passos; ", a, b, g1, passos);
  passos = 0; g2 = mdc_euclides(a, b, 0);
  printf("Euclides = %d em %ld passos\n", g2, passos);
}

int main(void) {
  printf("Divisões de mdc(1071, 462):\n");
  printf("  resultado: %d\n\n", mdc_euclides(1071, 462, 1));
  compara(1071, 462);
  compara(55, 34);          /* números de Fibonacci: pior caso */
  compara(1000000, 999999);
  printf("\nrecursivo: mdc(1071, 462) = %d\n", mdc_rec(1071, 462));
  printf("mmc(12, 18) = %d\n", 12 / mdc_rec(12, 18) * 18);
  return 0;
}
