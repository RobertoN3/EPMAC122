#include <stdio.h>
#include <string.h>

typedef struct {
  char nome[50];
  int nusp;
  float nota;
} Aluno;

/* recebe ponteiro: altera o registro original */
void corrige(Aluno *p, float novaNota) {
  p->nota = novaNota;            /* o mesmo que (*p).nota */
}

/* inserção direta pela nota, decrescente e estável */
void ordenaPorNota(Aluno t[], int n) {
  int i, j;
  Aluno x;
  for (i = 1; i < n; i++) {
    x = t[i];                    /* copia o registro inteiro */
    for (j = i - 1; j >= 0 && t[j].nota < x.nota; j--)
      t[j + 1] = t[j];
    t[j + 1] = x;
  }
}

int main(void) {
  Aluno turma[3] = {
    {"Ana", 1111, 7.5f},
    {"Bruno", 2222, 9.0f},
    {"Carla", 3333, 7.5f}
  };
  int i;
  corrige(&turma[0], 8.0f);
  ordenaPorNota(turma, 3);
  for (i = 0; i < 3; i++)
    printf("%-6s %d %.1f\n", turma[i].nome, turma[i].nusp, turma[i].nota);
  return 0;
}
