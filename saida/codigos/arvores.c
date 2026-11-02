#include <stdio.h>
#include <stdlib.h>

/* nó de uma árvore de expressão: operador (+, -, *) ou número */
typedef struct no {
  char op;                      /* ' ' quando o nó é um número */
  int valor;
  struct no *esq, *dir;
} No;

No *cria(char op, int valor, No *esq, No *dir) {
  No *novo = malloc(sizeof(No));
  if (novo == NULL) exit(1);
  novo->op = op;
  novo->valor = valor;
  novo->esq = esq;
  novo->dir = dir;
  return novo;
}

No *numero(int x) { return cria(' ', x, NULL, NULL); }

void visita(No *r) {
  if (r->op == ' ') printf("%d ", r->valor);
  else printf("%c ", r->op);
}

void preordem(No *r) {          /* raiz, esquerda, direita */
  if (r == NULL) return;
  visita(r); preordem(r->esq); preordem(r->dir);
}

void inordem(No *r) {           /* esquerda, raiz, direita */
  if (r == NULL) return;
  inordem(r->esq); visita(r); inordem(r->dir);
}

void posordem(No *r) {          /* esquerda, direita, raiz */
  if (r == NULL) return;
  posordem(r->esq); posordem(r->dir); visita(r);
}

/* altura: -1 para a árvore vazia, 0 para uma folha */
int altura(No *r) {
  int he, hd;
  if (r == NULL) return -1;
  he = altura(r->esq);
  hd = altura(r->dir);
  return 1 + (he > hd ? he : hd);
}

int conta(No *r) {
  if (r == NULL) return 0;
  return 1 + conta(r->esq) + conta(r->dir);
}

/* valor da expressão: percurso em pós-ordem */
int avalia(No *r) {
  int a, b;
  if (r->op == ' ') return r->valor;
  a = avalia(r->esq);
  b = avalia(r->dir);
  switch (r->op) {
    case '+': return a + b;
    case '-': return a - b;
    default:  return a * b;
  }
}

void libera(No *r) {            // pós-ordem: filhos antes do pai
  if (r == NULL) return;
  libera(r->esq);
  libera(r->dir);
  free(r);
}

int main(void) {
  /* (2 + 3) * 4 */
  No *raiz = cria('*', 0, cria('+', 0, numero(2), numero(3)), numero(4));
  printf("pre-ordem: "); preordem(raiz); printf("\n");
  printf("in-ordem:  "); inordem(raiz);  printf("\n");
  printf("pos-ordem: "); posordem(raiz); printf("\n");
  printf("altura %d, %d nos, valor %d\n", altura(raiz), conta(raiz), avalia(raiz));
  libera(raiz);
  return 0;
}
