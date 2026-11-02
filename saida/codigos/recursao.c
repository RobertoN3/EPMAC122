#include <stdio.h>

/* fatorial recursivo */
long fat(int n) {
  if (n == 0) return 1;      /* base da recursão */
  return n * fat(n - 1);     /* passo recursivo */
}

/* Torres de Hanói: move n discos de 'origem' para 'destino' */
void hanoi(int n, char origem, char destino, char aux) {
  if (n == 1) {
    printf("mover disco 1 de %c para %c\n", origem, destino);
    return;
  }
  hanoi(n - 1, origem, aux, destino);   // libera o maior disco
  printf("mover disco %d de %c para %c\n", n, origem, destino);
  hanoi(n - 1, aux, destino, origem);   // recoloca os n-1 discos
}

int main(void) {
  printf("10! = %ld\n", fat(10));
  hanoi(3, 'A', 'C', 'B');
  return 0;
}
