#include <stdio.h>
#include <ctype.h>

enum { INICIO, SINAL, INTEIRO, PONTO, REAL, IDENT, ERRO, N_ESTADOS };
enum { C_DIGITO, C_LETRA, C_SINAL, C_PONTO, C_OUTRO, N_CLASSES };

const char *nome_estado[N_ESTADOS] =
  {"INICIO", "SINAL", "INTEIRO", "PONTO", "REAL", "IDENT", "ERRO"};

/* função de transição d: delta[estado][classe do caractere] */
const int delta[N_ESTADOS][N_CLASSES] = {
  /*            DIGITO   LETRA  SINAL  PONTO  OUTRO */
  /* INICIO  */ {INTEIRO, IDENT, SINAL, ERRO,  ERRO},
  /* SINAL   */ {INTEIRO, ERRO,  ERRO,  ERRO,  ERRO},
  /* INTEIRO */ {INTEIRO, ERRO,  ERRO,  PONTO, ERRO},
  /* PONTO   */ {REAL,    ERRO,  ERRO,  ERRO,  ERRO},
  /* REAL    */ {REAL,    ERRO,  ERRO,  ERRO,  ERRO},
  /* IDENT   */ {IDENT,   IDENT, ERRO,  ERRO,  ERRO},
  /* ERRO    */ {ERRO,    ERRO,  ERRO,  ERRO,  ERRO}
};

int classe(char c) {
  if (isdigit((unsigned char)c)) return C_DIGITO;
  if (isalpha((unsigned char)c) || c == '_') return C_LETRA;
  if (c == '+' || c == '-') return C_SINAL;
  if (c == '.') return C_PONTO;
  return C_OUTRO;
}

/* roda o autômato; se mostrar != 0, imprime os estados percorridos */
int reconhece(const char *s, int mostrar) {
  int q = INICIO;
  if (mostrar) printf("%s", nome_estado[q]);
  for (; *s; s++) {
    q = delta[q][classe(*s)];        /* uma consulta por caractere: O(1) */
    if (mostrar) printf(" -(%c)-> %s", *s, nome_estado[q]);
  }
  if (mostrar) printf("\n");
  return q;
}

int main(void) {
  const char *testes[] = {"42", "-17", "3.14", "+2.", "x1", "_soma",
                          "2x", "--3", "", "a.b", ".5"};
  int i, q;
  reconhece("-3.14", 1);
  printf("\n");
  for (i = 0; i < 11; i++) {
    q = reconhece(testes[i], 0);
    printf("%-8s %s\n", testes[i][0] ? testes[i] : "(vazia)",
           q == INTEIRO ? "inteiro" : q == REAL ? "real" :
           q == IDENT ? "identificador" : "rejeitada");
  }
  return 0;
}
