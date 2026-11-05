#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define M 13   /* tamanho da tabela: um número primo */

typedef struct celula {
  char *chave;
  int valor;
  struct celula *prox;
} Celula;

Celula *tabela[M];   /* cada posição aponta para uma lista encadeada */

/* espalhamento de cadeias pelo método de Horner, base 31 */
unsigned hash(const char *s) {
  unsigned h = 0;
  while (*s) h = (h * 31 + (unsigned char)*s++) % M;
  return h;
}

Celula *busca(const char *chave) {
  Celula *p;
  for (p = tabela[hash(chave)]; p != NULL; p = p->prox)
    if (strcmp(p->chave, chave) == 0) return p;
  return NULL;
}

void insere(const char *chave, int valor) {
  Celula *p = busca(chave);
  unsigned h;
  if (p != NULL) { p->valor = valor; return; }   /* já existe: atualiza */
  h = hash(chave);
  p = malloc(sizeof(Celula));
  p->chave = malloc(strlen(chave) + 1);
  strcpy(p->chave, chave);
  p->valor = valor;
  p->prox = tabela[h];                           /* insere no início da lista */
  tabela[h] = p;
}

int main(void) {
  Celula *p;
  insere("Kruskal", 1956);
  insere("Dijkstra", 1956);
  insere("Hoare", 1959);
  p = busca("Hoare");
  printf("Hoare -> %d (posicao %u)\n", p ? p->valor : -1, hash("Hoare"));
  printf("Euler %s na tabela\n", busca("Euler") ? "esta" : "nao esta");
  return 0;
}
