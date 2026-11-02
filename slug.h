/* slug.h: âncora (id) de um valor nos índices.

   gerador.c cria as âncoras nas páginas indexadoras (por exemplo,
   <details id="método-guloso"> em tecnica.html) e gerador_conteudo.c aponta
   para elas na ficha de cada página (tecnica.html#m%C3%A9todo-guloso). Os
   dois incluem este arquivo para produzir exatamente o mesmo texto. */
#ifndef SLUG_H
#define SLUG_H

#include <stdio.h>
#include <ctype.h>

/* Letras ASCII viram minúsculas; dígitos e bytes de caracteres acentuados
   (UTF-8) ficam; qualquer sequência de outros caracteres vira um único '-',
   sem '-' nas pontas. Com url != 0, os bytes UTF-8 saem codificados
   (%C3%A9), como devem aparecer em um href. */
static void escrever_slug(FILE *o, const char *s, int url) {
    int traco = 0, algo = 0;
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (c >= 0x80 || isalnum(c)) {
            if (traco && algo) fputc('-', o);
            traco = 0;
            algo = 1;
            if (c < 0x80)  fputc(tolower(c), o);
            else if (url)  fprintf(o, "%%%02X", c);
            else           fputc(c, o);
        } else {
            traco = 1;
        }
    }
}

#endif
