#include <stdio.h>
#define N 5

typedef struct {
  int v[N];
  int ini, fim, tam;   /* tam = número de elementos */
} Fila;

void cria(Fila *f) { f->ini = f->fim = f->tam = 0; }
int vazia(Fila *f) { return f->tam == 0; }
int cheia(Fila *f) { return f->tam == N; }

int insere(Fila *f, int x) {
  if (cheia(f)) return 0;
  f->v[f->fim] = x;
  f->fim = (f->fim + 1) % N;     /* avança circularmente */
  f->tam++;
  return 1;
}

int removeFila(Fila *f) {        /* supõe fila não vazia */
  int x = f->v[f->ini];
  f->ini = (f->ini + 1) % N;
  f->tam--;
  return x;
}

int main(void) {
  Fila f;
  int i;
  cria(&f);
  for (i = 1; i <= 4; i++) insere(&f, i * 10);
  printf("saiu %d\n", removeFila(&f));
  printf("saiu %d\n", removeFila(&f));
  insere(&f, 50); insere(&f, 60); insere(&f, 70);   /* dá a volta no vetor */
  printf("insere 80 com fila cheia: %s\n", insere(&f, 80) ? "ok" : "recusado");
  while (!vazia(&f)) printf("%d ", removeFila(&f));
  printf("\n");
  return 0;
}
