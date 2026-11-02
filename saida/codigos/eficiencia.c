#include <stdio.h>
#include <math.h>

/* Cosseno pela série de Taylor, versão eficiente:
   cada termo é obtido do anterior, sem calcular potência e fatorial */
double cosseno(double x) {
  double termo = 1.0, soma = 1.0;
  int i;
  for (i = 0; i < 40; i += 2) {
    termo = -termo * x * x / ((i + 1) * (i + 2));
    soma += termo;
  }
  return soma;
}

int main(void) {
  double x = 0.1;
  printf("cosseno(%.2f) = %.15f\n", x, cosseno(x));
  printf("cos(%.2f)     = %.15f  (math.h)\n", x, cos(x));
  return 0;
}
