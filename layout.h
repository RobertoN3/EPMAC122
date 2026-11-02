/* layout.h: o que as páginas dos três programas têm em comum.

   gerador.c, gerador_conteudo.c e colorir.c incluem este arquivo para
   escrever o mesmo <head>, a mesma barra de navegação (com os seis índices)
   e o mesmo rodapé. O visual fica em estilo.css, que cada programa copia
   para a pasta de saída. */
#ifndef LAYOUT_H
#define LAYOUT_H

#include <stdio.h>
#include <string.h>

#define ESTILO "estilo.css"

/* os seis índices, na ordem dos campos 1 a 6 do cabeçalho dos TXT */
static const char *const LAYOUT_FACETA[6] =
    {"tecnica", "algoritmo", "autores", "data", "conceitos", "problema"};
static const char *const LAYOUT_ROTULO[6] =
    {"Técnica", "Algoritmo", "Autores", "Data", "Conceitos", "Problema"};

static void layout_esc(FILE *o, const char *s) {
    for (; *s; s++) {
        switch (*s) {
            case '&': fputs("&amp;", o);  break;
            case '<': fputs("&lt;", o);   break;
            case '>': fputs("&gt;", o);   break;
            case '"': fputs("&quot;", o); break;
            default:  fputc((unsigned char)*s, o);
        }
    }
}

/* Copia estilo.css (da pasta onde o programa roda) para a pasta de saída.
   Devolve 0 se deu certo. Sem o arquivo, as páginas funcionam, mas sem o
   visual: o programa só avisa. */
static int copiar_estilo(const char *pasta) {
    char destino[1024], buf[4096];
    FILE *in, *out;
    size_t n;
    int ok = 1;

    if (snprintf(destino, sizeof destino, "%s/%s", pasta, ESTILO) >= (int)sizeof destino)
        return -1;
    in = fopen(ESTILO, "rb");
    if (!in) {
        fprintf(stderr, "[aviso] %s não encontrado: as páginas ficarão sem o visual\n", ESTILO);
        return -1;
    }
    out = fopen(destino, "wb");
    if (!out) { perror(destino); fclose(in); return -1; }
    while ((n = fread(buf, 1, sizeof buf, in)) > 0)
        if (fwrite(buf, 1, n, out) != n) { ok = 0; break; }
    if (ferror(in)) ok = 0;
    fclose(in);
    if (fclose(out)) ok = 0;
    if (!ok) fprintf(stderr, "[aviso] erro ao copiar %s\n", ESTILO);
    return ok ? 0 : -1;
}

/* Abre a página até o início do conteúdo.
   titulo:   texto da aba do navegador
   classe:   classe do <body> (tipo de página e cor do índice, se houver)
   faceta:   0 a 5 marca o índice atual na barra; -1 para nenhum
   avulsa:   1 para uma página fora do site (modo filtro do colorir): o
             estilo vai embutido e não há barra de navegação
   css_extra: regras CSS próprias da página, ou NULL */
static void abrir_pagina(FILE *o, const char *titulo, const char *classe,
                         int faceta, int avulsa, const char *css_extra) {
    int k;
    fputs("<!DOCTYPE html>\n<html lang=\"pt-BR\"><head><meta charset=\"utf-8\">\n"
          "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n<title>", o);
    layout_esc(o, titulo);
    fputs(" | MAC0122</title>\n"
          "<link rel=\"preconnect\" href=\"https://fonts.googleapis.com\">\n"
          "<link rel=\"preconnect\" href=\"https://fonts.gstatic.com\" crossorigin>\n"
          "<link rel=\"stylesheet\" href=\"https://fonts.googleapis.com/css2?"
          "family=Bricolage+Grotesque:opsz,wght@12..96,400..800&amp;"
          "family=JetBrains+Mono:wght@400;700&amp;"
          "family=Source+Serif+4:ital,opsz,wght@0,8..60,400..700;1,8..60,400..700"
          "&amp;display=swap\">\n", o);
    if (avulsa) {                       /* sem pasta de site: estilo embutido */
        FILE *f = fopen(ESTILO, "rb");
        if (f) {
            int c;
            fputs("<style>\n", o);
            while ((c = fgetc(f)) != EOF) fputc(c, o);
            fputs("</style>\n", o);
            fclose(f);
        }
    } else {
        fputs("<link rel=\"stylesheet\" href=\"" ESTILO "\">\n", o);
    }
    if (css_extra) fprintf(o, "<style>%s</style>\n", css_extra);
    fprintf(o, "</head>\n<body class=\"%s\">\n", classe);

    if (!avulsa) {
        fputs("<header class=\"barra\">\n<a class=\"marca\" href=\"index.html\">"
              "<span class=\"marca-no\" aria-hidden=\"true\"></span>MAC0122</a>\n"
              "<nav class=\"facetas\" aria-label=\"Índices\">", o);
        for (k = 0; k < 6; k++)
            fprintf(o, "<a class=\"f-%s\" href=\"%s.html\"%s>%s</a>",
                    LAYOUT_FACETA[k], LAYOUT_FACETA[k],
                    k == faceta ? " aria-current=\"page\"" : "", LAYOUT_ROTULO[k]);
        fputs("</nav>\n</header>\n", o);
    }
}

/* Fecha a página; script: código JavaScript da página, ou NULL */
static void fechar_pagina(FILE *o, int avulsa, const char *script) {
    if (!avulsa)
        fputs("<footer class=\"rodape\">Páginas geradas a partir dos arquivos TXT "
              "do EP de MAC0122.</footer>\n", o);
    if (script) fprintf(o, "<script>%s</script>\n", script);
    fputs("</body></html>\n", o);
}

#endif
