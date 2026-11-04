#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#define MAX 100

/* precedência dos operadores: * e / antes de + e - */
int prec(char op) { return (op == '*' || op == '/') ? 2 : (op == '+' || op == '-') ? 1 : 0; }

/* infixa -> posfixa (algoritmo da estação de manobras, com pilha de operadores);
   operandos de um dígito, operadores + - * / e parênteses */
void posfixa(const char *in, char *out) {
  char pilha[MAX];
  int topo = 0, k = 0;
  for (; *in; in++) {
    char c = *in;
    if (isdigit((unsigned char)c)) out[k++] = c;           /* operando: vai direto */
    else if (c == '(') pilha[topo++] = c;
    else if (c == ')') {                                   /* desempilha até o '(' */
      while (topo > 0 && pilha[topo - 1] != '(') out[k++] = pilha[--topo];
      topo--;                                              /* descarta o '(' */
    } else if (prec(c)) {                                  /* operador */
      while (topo > 0 && prec(pilha[topo - 1]) >= prec(c)) out[k++] = pilha[--topo];
      pilha[topo++] = c;
    }                                                      /* espaços são ignorados */
  }
  while (topo > 0) out[k++] = pilha[--topo];
  out[k] = '\0';
}

int aplica(char op, int a, int b) {
  switch (op) { case '+': return a + b; case '-': return a - b;
                case '*': return a * b; default:  return a / b; }
}

/* avalia a posfixa com uma pilha de valores */
int avalia(const char *pos) {
  int pilha[MAX], topo = 0, a, b;
  for (; *pos; pos++)
    if (isdigit((unsigned char)*pos)) pilha[topo++] = *pos - '0';
    else { b = pilha[--topo]; a = pilha[--topo]; pilha[topo++] = aplica(*pos, a, b); }
  return pilha[0];
}

/* árvore de expressão: folhas são operandos, nós internos são operadores */
typedef struct no { char c; struct no *esq, *dir; } No;

No *novo(char c, No *e, No *d) {
  No *n = malloc(sizeof(No));
  if (n == NULL) { perror("malloc"); exit(1); }
  n->c = c; n->esq = e; n->dir = d;
  return n;
}

/* monta a árvore lendo a posfixa com uma pilha de ponteiros para nós */
No *arvore(const char *pos) {
  No *pilha[MAX], *e, *d;
  int topo = 0;
  for (; *pos; pos++)
    if (isdigit((unsigned char)*pos)) pilha[topo++] = novo(*pos, NULL, NULL);
    else { d = pilha[--topo]; e = pilha[--topo]; pilha[topo++] = novo(*pos, e, d); }
  return pilha[0];
}

void pre_ordem(No *r) { if (r) { printf("%c ", r->c); pre_ordem(r->esq); pre_ordem(r->dir); } }
void pos_ordem(No *r) { if (r) { pos_ordem(r->esq); pos_ordem(r->dir); printf("%c ", r->c); } }
void in_ordem(No *r) {                       /* in-ordem com parênteses em volta de cada operador */
  if (!r) return;
  if (r->esq) printf("(");
  in_ordem(r->esq); printf("%c", r->c); in_ordem(r->dir);
  if (r->esq) printf(")");
}
int altura(No *r) {                    /* em arestas: uma folha tem altura 0 */
  int he, hd;
  if (!r) return -1;
  he = altura(r->esq); hd = altura(r->dir);
  return 1 + (he > hd ? he : hd);
}
int valor(No *r) {                     /* avaliação = percurso em pós-ordem */
  if (!r->esq) return r->c - '0';
  return aplica(r->c, valor(r->esq), valor(r->dir));
}
void libera(No *r) { if (r) { libera(r->esq); libera(r->dir); free(r); } }

int main(void) {
  const char *testes[] = {"(5 + 3) * 2 - 8 / 4", "2 + 3 * 4", "(2 + 3) * 4", "9 - 4 - 3"};
  char p[MAX];
  int t;
  for (t = 0; t < 4; t++) {
    No *r;
    posfixa(testes[t], p);
    r = arvore(p);
    printf("infixa: %-22s posfixa: %-10s valor: %d\n", testes[t], p, avalia(p));
    printf("  árvore -> pré: "); pre_ordem(r);
    printf(" pós: "); pos_ordem(r);
    printf(" in: "); in_ordem(r);
    printf("\n  altura %d, valor pela árvore %d\n", altura(r), valor(r));
    libera(r);
  }
  return 0;
}
