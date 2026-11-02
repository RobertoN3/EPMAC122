#include <stdio.h>
#define U 64                  /* universo: 0..U-1 */
#define BYTES ((U + 7) / 8)   /* 8 elementos por byte */

/* ---------- método linear ---------- */
typedef struct { int v[U]; int n; } ConjLinear;

int lin_pertence(const ConjLinear *c, int x) {
  int i;
  for (i = 0; i < c->n; i++) if (c->v[i] == x) return 1;   /* O(n) */
  return 0;
}
void lin_insere(ConjLinear *c, int x) {
  if (!lin_pertence(c, x)) c->v[c->n++] = x;
}
void lin_remove(ConjLinear *c, int x) {
  int i;
  for (i = 0; i < c->n; i++)
    if (c->v[i] == x) { c->v[i] = c->v[--c->n]; return; }  /* põe o último no lugar */
}

/* ---------- vetor de bits ---------- */
typedef struct { unsigned char b[BYTES]; } ConjBits;

int bit_pertence(const ConjBits *c, int x) { return (c->b[x / 8] >> (x % 8)) & 1; }
void bit_insere(ConjBits *c, int x) { c->b[x / 8] |= (unsigned char)(1u << (x % 8)); }
void bit_remove(ConjBits *c, int x) { c->b[x / 8] &= (unsigned char)~(1u << (x % 8)); }
void bit_esvazia(ConjBits *c) { int i; for (i = 0; i < BYTES; i++) c->b[i] = 0; } // O(U)

void bit_imprime(const char *nome, const ConjBits *c) {
  int x;
  printf("%s = {", nome);
  for (x = 0; x < U; x++) if (bit_pertence(c, x)) printf(" %d", x);
  printf(" }\n");
}

int main(void) {
  ConjLinear L = {{0}, 0};
  ConjBits A, B, uniao, inter;
  int dados[] = {5, 12, 40, 12, 7, 63}, i, x;

  bit_esvazia(&A);
  for (i = 0; i < 6; i++) { lin_insere(&L, dados[i]); bit_insere(&A, dados[i]); }
  lin_remove(&L, 40);
  bit_remove(&A, 40);
  printf("linear: n = %d, pertence(12) = %d, pertence(40) = %d\n",
         L.n, lin_pertence(&L, 12), lin_pertence(&L, 40));
  bit_imprime("bits A", &A);

  bit_esvazia(&B);
  for (x = 0; x < 16; x += 3) bit_insere(&B, x);    /* múltiplos de 3 até 15 */
  bit_imprime("bits B", &B);
  for (i = 0; i < BYTES; i++) {                       /* 8 elementos por operação */
    uniao.b[i] = A.b[i] | B.b[i];
    inter.b[i] = A.b[i] & B.b[i];
  }
  bit_imprime("A uniao B", &uniao);
  bit_imprime("A inter B", &inter);

  /* crivo de Eratóstenes: os compostos são marcados no vetor de bits */
  {
    unsigned char comp[(101 + 7) / 8] = {0};
    int p, m;
    printf("primos até 100:");
    for (p = 2; p <= 100; p++) {
      if ((comp[p / 8] >> (p % 8)) & 1) continue;
      printf(" %d", p);
      for (m = p * p; m <= 100; m += p) comp[m / 8] |= (unsigned char)(1u << (m % 8));
    }
    printf("\n");
  }
  return 0;
}
