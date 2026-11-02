#include <stdio.h>
#include <stdlib.h>

typedef struct celula {
  int conteudo;
  struct celula *prox;
} Celula;

/* insere x logo depois da célula p */
void insere(int x, Celula *p) {
  Celula *nova = malloc(sizeof(Celula));
  nova->conteudo = x;
  nova->prox = p->prox;
  p->prox = nova;
}

/* devolve a célula ANTERIOR à que contém x (ou NULL), em lista com cabeça */
Celula *buscaAnterior(int x, Celula *cabeca) {
  Celula *p = cabeca;
  while (p->prox != NULL && p->prox->conteudo != x)
    p = p->prox;
  return p->prox == NULL ? NULL : p;
}

/* remove a primeira célula com conteúdo x */
void removeX(int x, Celula *cabeca) {
  Celula *ant = buscaAnterior(x, cabeca), *lixo;
  if (ant == NULL) return;          // x não está na lista
  lixo = ant->prox;
  ant->prox = lixo->prox;
  free(lixo);
}

/* impressão recursiva: lista = célula seguida de lista */
void imprime(Celula *lst) {
  if (lst == NULL) { printf("\n"); return; }
  printf("%d -> ", lst->conteudo);
  imprime(lst->prox);
}

int main(void) {
  Celula cabeca;                    /* cabeça: conteúdo ignorado */
  int i;
  cabeca.prox = NULL;
  for (i = 5; i >= 1; i--) insere(i * 10, &cabeca);
  imprime(cabeca.prox);
  removeX(30, &cabeca);
  imprime(cabeca.prox);
  while (cabeca.prox != NULL) removeX(cabeca.prox->conteudo, &cabeca);
  return 0;
}
