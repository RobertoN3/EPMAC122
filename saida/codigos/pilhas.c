#include <stdio.h>
#include <string.h>
#define MAX 100

char pilha[MAX];
int t = 0;                     /* número de elementos na pilha */

void empilha(char c) { pilha[t++] = c; }
char desempilha(void) { return pilha[--t]; }
int  vazia(void)      { return t == 0; }

/* devolve 1 se os símbolos ()[]{} de s estão balanceados */
int balanceada(const char *s) {
  int i;
  char c, a;
  t = 0;
  for (i = 0; s[i] != '\0'; i++) {
    c = s[i];
    if (c == '(' || c == '[' || c == '{') {
      if (t == MAX) return 0;            /* pilha cheia */
      empilha(c);
    } else if (c == ')' || c == ']' || c == '}') {
      if (vazia()) return 0;             /* fecha sem ter aberto */
      a = desempilha();
      if ((c == ')' && a != '(') || (c == ']' && a != '[') ||
          (c == '}' && a != '{')) return 0;
    }
  }
  return vazia();                        /* sobrou algo aberto? */
}

int main(void) {
  const char *testes[] = {"{a[i] = (x + y) * 2;}", "(a + b]", "((a)"};
  int i;
  for (i = 0; i < 3; i++)
    printf("\"%s\" -> %s\n", testes[i],
           balanceada(testes[i]) ? "bem formada" : "mal formada");
  return 0;
}
