#include <stdio.h>
#define MAX 10

/* dimensões: a matriz Ai tem p[i-1] linhas e p[i] colunas */
long m[MAX][MAX];   /* m[i][j]: menor custo para calcular Ai...Aj */
int  s[MAX][MAX];   /* s[i][j]: onde fica o último produto nesse ótimo */
long chamadas = 0;  /* chamadas da versão recursiva ingênua */

/* recursão direta da definição: tempo exponencial */
long custo_rec(const int p[], int i, int j) {
  long melhor = -1, c;
  int k;
  chamadas++;
  if (i == j) return 0;
  for (k = i; k < j; k++) {
    c = custo_rec(p, i, k) + custo_rec(p, k + 1, j) + (long)p[i - 1] * p[k] * p[j];
    if (melhor < 0 || c < melhor) melhor = c;
  }
  return melhor;
}

/* programação dinâmica de baixo para cima, por tamanho do trecho: O(n^3) */
void cadeia(const int p[], int n) {
  int i, j, k, tam;
  long c;
  for (i = 1; i <= n; i++) m[i][i] = 0;
  for (tam = 2; tam <= n; tam++)
    for (i = 1; i + tam - 1 <= n; i++) {
      j = i + tam - 1;
      m[i][j] = -1;
      for (k = i; k < j; k++) {                    /* último produto: (Ai..Ak)(Ak+1..Aj) */
        c = m[i][k] + m[k + 1][j] + (long)p[i - 1] * p[k] * p[j];
        if (m[i][j] < 0 || c < m[i][j]) { m[i][j] = c; s[i][j] = k; }
      }
    }
}

/* reconstrói a parentização ótima a partir de s */
void imprime(int i, int j) {
  if (i == j) { printf("A%d", i); return; }
  printf("(");
  imprime(i, s[i][j]);
  imprime(s[i][j] + 1, j);
  printf(")");
}

int main(void) {
  int p3[] = {10, 100, 5, 50};
  int p6[] = {30, 35, 15, 5, 10, 20, 25};
  int i, j;
  long ingenuo;

  cadeia(p3, 3);
  printf("A1(10x100) A2(100x5) A3(5x50)\n");
  printf("  (A1 A2) A3 custa %ld, A1 (A2 A3) custa %ld; ótimo = %ld: ",
         10L * 100 * 5 + 10L * 5 * 50, 100L * 5 * 50 + 10L * 100 * 50, m[1][3]);
  imprime(1, 3);
  printf("\n\n");

  cadeia(p6, 6);
  printf("Seis matrizes, dimensões 30 35 15 5 10 20 25. Tabela m[i][j]:\n");
  for (i = 1; i <= 6; i++) {
    for (j = 1; j <= 6; j++)
      if (j < i) printf("%7s", ""); else printf("%7ld", m[i][j]);
    printf("\n");
  }
  printf("Custo mínimo: %ld multiplicações. Ordem: ", m[1][6]);
  imprime(1, 6);
  ingenuo = custo_rec(p6, 1, 6);          /* calcula antes: a ordem dos argumentos de printf não é garantida */
  printf("\nA recursão ingênua chega ao mesmo valor (%ld) com %ld chamadas.\n", ingenuo, chamadas);
  return 0;
}
