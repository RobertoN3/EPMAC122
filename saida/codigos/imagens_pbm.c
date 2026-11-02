#include <stdio.h>
#define LIN 5
#define COL 11
#define TAM 8          /* lado do degradê */

/* "MAC" do material da disciplina: 1 = preto, 0 = branco */
int mac[LIN][COL] = {
  {1, 0, 1, 0, 1, 1, 1, 0, 1, 1, 1},
  {1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0},
  {1, 1, 1, 0, 1, 1, 1, 0, 1, 0, 0},
  {1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0},
  {1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1}
};

/* grava uma matriz TAM x TAM de cinzas no formato PGM em texto (P2) */
int grava_pgm(const char *nome, int img[TAM][TAM], int max) {
  FILE *f = fopen(nome, "w");
  int i, j;
  if (f == NULL) { perror(nome); return 0; }
  fprintf(f, "P2\n# gerado pelo EP de MAC0122\n%d %d\n%d\n", TAM, TAM, max); /* largura, altura */
  for (i = 0; i < TAM; i++) {
    for (j = 0; j < TAM; j++) fprintf(f, "%4d", img[i][j]);
    fprintf(f, "\n");
  }
  fclose(f);
  return 1;
}

int main(void) {
  int deg[TAM][TAM], neg[TAM][TAM], i, j;
  FILE *f;

  /* PBM: a palavra MAC; o cabeçalho traz COLUNAS antes de LINHAS */
  f = fopen("mac.pbm", "w");
  if (f == NULL) { perror("mac.pbm"); return 1; }
  fprintf(f, "P1\n%d %d\n", COL, LIN);
  for (i = 0; i < LIN; i++) {
    for (j = 0; j < COL; j++) fprintf(f, "%d ", mac[i][j]);
    fprintf(f, "\n");
  }
  fclose(f);

  /* PGM: degradê da esquerda (preto) para a direita (branco) e o negativo */
  for (i = 0; i < TAM; i++)
    for (j = 0; j < TAM; j++) {
      deg[i][j] = j * 255 / (TAM - 1);
      neg[i][j] = 255 - deg[i][j];       // negativo: max - valor
    }
  if (!grava_pgm("degrade.pgm", deg, 255) || !grava_pgm("negativo.pgm", neg, 255)) return 1;

  /* PPM: 2 x 3 pixels coloridos, três números (R G B) por pixel */
  f = fopen("cores.ppm", "w");
  if (f == NULL) { perror("cores.ppm"); return 1; }
  fprintf(f, "P3\n3 2\n255\n");
  fprintf(f, "255 0 0   0 255 0   0 0 255\n");     /* vermelho, verde, azul */
  fprintf(f, "255 255 0   0 0 0   255 255 255\n"); /* amarelo, preto, branco */
  fclose(f);

  printf("Gravados: mac.pbm (%dx%d), degrade.pgm e negativo.pgm (%dx%d), cores.ppm (3x2)\n",
         COL, LIN, TAM, TAM);
  printf("Linha do meio do degradê:");
  for (j = 0; j < TAM; j++) printf(" %d", deg[TAM / 2][j]);
  printf("\n");
  return 0;
}
