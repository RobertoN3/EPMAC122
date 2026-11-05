#include <stdio.h>
#include <stdlib.h>

typedef struct no {
  int chave;
  struct no *esq, *dir;
} No;

/* insere x na ABB de raiz r e devolve a (nova) raiz */
No *insere(No *r, int x) {
  if (r == NULL) {                     /* lugar vazio: cria o nó */
    No *novo = malloc(sizeof(No));
    if (novo == NULL) exit(1);
    novo->chave = x;
    novo->esq = novo->dir = NULL;
    return novo;
  }
  if (x < r->chave) r->esq = insere(r->esq, x);
  else if (x > r->chave) r->dir = insere(r->dir, x);
  return r;                            /* chave repetida: nada a fazer */
}

/* devolve o nó com chave x, ou NULL */
No *busca(No *r, int x) {
  while (r != NULL && r->chave != x)
    r = (x < r->chave) ? r->esq : r->dir;
  return r;
}

No *minimo(No *r) {
  while (r->esq != NULL) r = r->esq;
  return r;
}

/* remove x da ABB de raiz r e devolve a (nova) raiz */
No *remove_chave(No *r, int x) {
  No *filho;
  if (r == NULL) return NULL;          /* x não está na árvore */
  if (x < r->chave) r->esq = remove_chave(r->esq, x);
  else if (x > r->chave) r->dir = remove_chave(r->dir, x);
  else if (r->esq != NULL && r->dir != NULL) {
    r->chave = minimo(r->dir)->chave;  /* copia o sucessor */
    r->dir = remove_chave(r->dir, r->chave);
  } else {                             /* zero ou um filho */
    filho = (r->esq != NULL) ? r->esq : r->dir;
    free(r);
    return filho;
  }
  return r;
}

void inordem(No *r) {                  /* lista em ordem crescente */
  if (r == NULL) return;
  inordem(r->esq);
  printf("%d ", r->chave);
  inordem(r->dir);
}

void libera(No *r) {
  if (r == NULL) return;
  libera(r->esq);
  libera(r->dir);
  free(r);
}

int main(void) {
  int v[] = {50, 30, 70, 20, 40, 60, 80}, i;
  No *raiz = NULL;
  for (i = 0; i < 7; i++) raiz = insere(raiz, v[i]);
  inordem(raiz);
  printf("\n40 %s na arvore\n", busca(raiz, 40) ? "esta" : "nao esta");
  printf("45 %s na arvore\n", busca(raiz, 45) ? "esta" : "nao esta");
  raiz = remove_chave(raiz, 50);       /* dois filhos */
  raiz = remove_chave(raiz, 20);       /* folha */
  printf("raiz agora: %d\n", raiz->chave);
  inordem(raiz);
  printf("\n");
  libera(raiz);
  return 0;
}
