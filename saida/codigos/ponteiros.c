#include <stdio.h>
#include <stdlib.h>

/* passagem por referência: recebe os endereços */
void troca(int *a, int *b) {
  int aux = *a;
  *a = *b;
  *b = aux;
}

int main(void) {
  int x = 1, y = 2, n = 5, i;
  int *p = &x;          /* p guarda o endereço de x */
  int *v;

  *p = 10;              /* altera x através de p */
  printf("x = %d, endereco de x = %p\n", x, (void *)p);

  troca(&x, &y);
  printf("depois da troca: x = %d, y = %d\n", x, y);

  v = malloc(n * sizeof(int));   /* vetor com tamanho dado na execução */
  if (v == NULL) return 1;
  for (i = 0; i < n; i++)
    *(v + i) = i * i;            /* o mesmo que v[i] = i * i */
  for (i = 0; i < n; i++) printf("%d ", v[i]);
  printf("\n");
  free(v);
  return 0;
}
