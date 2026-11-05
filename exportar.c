/* Compilar: gcc -Wall -Wextra -pedantic -std=c99 exportar.c -o exportar

   Exporta os códigos C dos TXT para arquivos próprios, prontos para
   compilar, e escreve o COMO_COMPILAR.txt, que diz como compilar e rodar
   cada um. Para cada TXT de entradatxt/ com código:
   - grava cada bloco (entre INICIO_CODIGO_C e FIM_CODIGO_C) em
     saida/codigos/<nome>.c, ou <nome>_2.c, <nome>_3.c... se houver mais de
     um bloco; o texto é exatamente o do TXT;
   - anota o título da página, o texto da seção "## Sobre o código" e se o
     código usa math.h (para pôr -lm no comando de compilação).
   No fim, junta tudo em saida/codigos/COMO_COMPILAR.txt, em ordem
   alfabética de arquivo.

   As páginas de código coloridas são feitas pelo colorir.c, que tem um
   botão "Baixar" apontando para estes arquivos. A leitura dos TXT é a
   mesma nos dois programas (txt.h).

   Uso:
     exportar                          lê entradatxt/ e escreve em saida/
     exportar pasta_entrada pasta_saida  idem, com outras pastas */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif
#include "txt.h"

#define PASTA_CODIGOS "codigos"

static const char *entrada = "entradatxt", *saida = "saida";
static int erros;

/* o que o COMO_COMPILAR.txt diz de cada TXT com código */
typedef struct {
    char *base, *titulo, *sobre;   /* sobre: texto da seção "Sobre o código" */
    int nb, usa_math;
} Programa;

static Programa *progs;
static int nprogs, cap_progs;

static char *copia(const char *s) {
    char *c = checar(malloc(strlen(s) + 1));
    strcpy(c, s);
    return c;
}

static int criar_pasta(const char *p) {
#ifdef _WIN32
    if (_mkdir(p) && errno != EEXIST) { perror(p); return -1; }
#else
    if (mkdir(p, 0777) && errno != EEXIST) { perror(p); return -1; }
#endif
    return 0;
}

/* grava o texto do bloco em saida/codigos/<arquivo> */
static int gravar_c(const char *arquivo, const Texto *t) {
    char *pasta = caminho(saida, PASTA_CODIGOS, ""), *nome = caminho(pasta, arquivo, "");
    FILE *o = fopen(nome, "w");
    int ok = 1;
    if (!o) { perror(nome); ok = 0; }
    else {
        size_t n = t->n;                             /* sem as linhas em branco do fim */
        while (n && (t->s[n - 1] == '\n' || t->s[n - 1] == ' ' ||
                     t->s[n - 1] == '\t' || t->s[n - 1] == '\r')) n--;
        if (n) fwrite(t->s, 1, n, o);
        fputc('\n', o);
        if (ferror(o)) ok = 0;
        if (fclose(o)) ok = 0;
        if (!ok) fprintf(stderr, "%s: erro de escrita\n", nome);
    }
    free(nome);
    free(pasta);
    return ok;
}

/* troca [[alvo|texto]] por texto e [[alvo]] por alvo (texto puro, sem links) */
static void tirar_links(char *s) {
    char *o = s;
    while (*s) {
        char *fe;
        if (s[0] == '[' && s[1] == '[' && (fe = strstr(s + 2, "]]"))) {
            char *ini = s + 2, *barra = memchr(ini, '|', (size_t)(fe - ini));
            if (barra) ini = barra + 1;
            memmove(o, ini, (size_t)(fe - ini));
            o += fe - ini;
            s = fe + 2;
        } else {
            *o++ = *s++;
        }
    }
    *o = '\0';
}

/* escreve texto quebrado em linhas de até 76 colunas, com recuo */
static void quebrar(FILE *o, const char *s, const char *recuo) {
    int col = 0;
    while (*s) {
        size_t n;
        while (*s == ' ') s++;
        n = strcspn(s, " ");
        if (!n) break;
        if (col && col + 1 + (int)n > 76) { fputc('\n', o); col = 0; }
        if (!col) { fputs(recuo, o); col = (int)strlen(recuo); }
        else { fputc(' ', o); col++; }
        fwrite(s, 1, n, o);
        col += (int)n;
        s += n;
    }
    if (col) fputc('\n', o);
}

static int cmp_prog(const void *a, const void *b) {
    return strcmp(((const Programa *)a)->base, ((const Programa *)b)->base);
}

static void como_compilar(void) {
    char *pasta = caminho(saida, PASTA_CODIGOS, ""), *nome = caminho(pasta, "COMO_COMPILAR.txt", "");
    FILE *o = fopen(nome, "w");
    int i, b, arquivos = 0;
    char titulo[120];
    if (!o) { perror(nome); erros++; free(nome); free(pasta); return; }
    qsort(progs, (size_t)nprogs, sizeof *progs, cmp_prog);

    fputs("COMO COMPILAR E RODAR OS CÓDIGOS C\n"
          "==================================\n\n", o);
    quebrar(o, "Estes arquivos são gerados pelo exportar.c a partir dos TXT de entradatxt/: "
               "cada código C fica entre as linhas INICIO_CODIGO_C e FIM_CODIGO_C do TXT, "
               "e o arquivo .c tem exatamente o mesmo texto. Cada programa é um único "
               "arquivo .c e não lê arquivos auxiliares nem a entrada padrão: os dados de "
               "exemplo estão no próprio código, na função main.", "");
    fputs("\n", o);
    quebrar(o, "Todos compilam sem avisos com as opções abaixo (gcc em Linux, macOS ou "
               "Windows com MinGW). No Windows, o programa gerado se chama nome.exe e "
               "roda com nome.exe em vez de ./nome.", "");
    fputs("\nExemplo, para o algoritmo de Kruskal (na pasta onde está kruskal.c):\n\n"
          "    gcc -Wall -Wextra -pedantic -std=c99 -o kruskal kruskal.c\n"
          "    ./kruskal\n\n", o);
    fputs("OS QUATRO PROGRAMAS DO EP (geram o site)\n"
          "----------------------------------------\n", o);
    quebrar(o, "Na pasta dos fontes dos programas, junto com slug.h, layout.h, txt.h, "
               "estilo.css e indice.js (incluídos ou copiados pelos programas) e com a "
               "pasta entradatxt/ dos TXT:", "");
    fputs("\n    gcc -Wall -Wextra -pedantic -std=c99 -o gerador gerador.c\n"
          "    gcc -Wall -Wextra -pedantic -std=c99 -o gerador_conteudo gerador_conteudo.c\n"
          "    gcc -Wall -Wextra -pedantic -std=c99 -o colorir colorir.c\n"
          "    gcc -Wall -Wextra -pedantic -std=c99 -o exportar exportar.c\n"
          "    ./gerador\n    ./gerador_conteudo\n    ./colorir\n    ./exportar\n\n", o);
    quebrar(o, "gerador gera index.html e as seis páginas indexadoras; gerador_conteudo, "
               "uma página de explicação por TXT; colorir, o filtro de cores, uma página "
               "com o código C colorido por TXT; exportar, os arquivos .c e este "
               "COMO_COMPILAR.txt em saida/codigos/ (os botões \"Baixar\" das páginas de "
               "código apontam para eles). Todos escrevem em saida/; o site começa em "
               "saida/index.html. O colorir também funciona como filtro para qualquer "
               "arquivo C: ./colorir -f < programa.c > programa.html", "");
    for (i = 0; i < nprogs; i++) arquivos += progs[i].nb;
    snprintf(titulo, sizeof titulo, "OS CÓDIGOS DE EXEMPLO DAS PÁGINAS (%d programas em %d páginas)",
             arquivos, nprogs);
    fprintf(o, "\n\n%s\n", titulo);
    for (i = (int)strlen(titulo) - 2; i > 0; i--) fputc('-', o);   /* o "Ó" e o "Á" têm 2 bytes */
    fputc('\n', o);

    for (i = 0; i < nprogs; i++) {
        Programa *p = &progs[i];
        fputs("\n------------------------------------------------------------------------\n", o);
        for (b = 1; b <= p->nb; b++) {
            char arquivo[300], exe[300];
            nome_arquivo(arquivo, sizeof arquivo, p->base, b);
            snprintf(exe, sizeof exe, "%s", arquivo);
            exe[strlen(exe) - 2] = '\0';                  /* tira o ".c" */
            if (b == 1) fprintf(o, "%s\n", p->titulo);
            fprintf(o, "  Arquivo:  %s\n", arquivo);
            fprintf(o, "  Compilar: gcc -Wall -Wextra -pedantic -std=c99 -o %s %s%s\n",
                    exe, arquivo, p->usa_math ? " -lm" : "");
            fprintf(o, "  Rodar:    ./%s\n", exe);
        }
        if (p->usa_math)
            quebrar(o, "Usa math.h: no Linux, o -lm no fim do comando liga a biblioteca "
                       "matemática.", "  ");
        if (p->sobre && *p->sobre) {
            fputs("  O que faz:\n", o);
            quebrar(o, p->sobre, "    ");
        }
        fprintf(o, "  Explicação no site: %s.html\n", p->base);
    }
    if (ferror(o) | fclose(o)) { fprintf(stderr, "%s: erro de escrita\n", nome); erros++; }
    free(nome);
    free(pasta);
}

/* grava os .c de um TXT e anota o programa; 1 se tinha código, 0 se não, -1 em erro */
static int exportar(const char *base) {
    char *in = caminho(entrada, base, ".txt");
    CodigoTxt t;
    int ok = ler_txt(in, &t), b, usa_math = 0, r;
    Programa *p;

    if (ok && t.nb > 0) {
        for (b = 0; b < t.nb; b++) {
            char arquivo[300];
            nome_arquivo(arquivo, sizeof arquivo, base, b + 1);
            if (!gravar_c(arquivo, &t.blocos[b])) ok = 0;
            if (t.blocos[b].s && strstr(t.blocos[b].s, "<math.h>")) usa_math = 1;
        }
        if (nprogs == cap_progs) {
            cap_progs = cap_progs ? cap_progs * 2 : 32;
            progs = checar(realloc(progs, (size_t)cap_progs * sizeof *progs));
        }
        p = &progs[nprogs++];
        p->base = copia(base);
        p->titulo = copia(t.titulo && *t.titulo ? t.titulo : base);
        p->sobre = copia(t.sobre.s ? t.sobre.s : "");
        tirar_links(p->sobre);
        p->nb = t.nb;
        p->usa_math = usa_math;
    }
    r = !ok ? -1 : t.nb > 0;
    liberar_txt(&t);
    free(in);
    return r;
}

int main(int argc, char **argv) {
    DIR *d;
    struct dirent *e;
    int i, arquivos = 0;
    char *pasta;

    if (argc != 1 && argc != 3) {
        fprintf(stderr, "Uso: %s [pasta_entrada pasta_saida]\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (argc == 3) { entrada = argv[1]; saida = argv[2]; }

    d = opendir(entrada);
    if (!d) { perror(entrada); return EXIT_FAILURE; }
    pasta = caminho(saida, PASTA_CODIGOS, "");
    if (criar_pasta(saida) || criar_pasta(pasta)) { free(pasta); closedir(d); return EXIT_FAILURE; }
    free(pasta);

    while ((e = readdir(d))) {
        size_t n = strlen(e->d_name);
        char *base;
        if (n < 5 || strcmp(e->d_name + n - 4, ".txt")) continue;
        base = checar(malloc(n - 3));
        memcpy(base, e->d_name, n - 4);
        base[n - 4] = '\0';
        if (!identificador(base)) {
            fprintf(stderr, "Nome inválido: %s\n", e->d_name);
            erros++;
        } else if (exportar(base) < 0) erros++;
        free(base);
    }
    closedir(d);
    como_compilar();

    for (i = 0; i < nprogs; i++) arquivos += progs[i].nb;
    printf("%d arquivo(s) .c de %d TXT e COMO_COMPILAR.txt em '%s/%s'.\n",
           arquivos, nprogs, saida, PASTA_CODIGOS);
    for (i = 0; i < nprogs; i++) { free(progs[i].base); free(progs[i].titulo); free(progs[i].sobre); }
    free(progs);
    return erros ? EXIT_FAILURE : EXIT_SUCCESS;
}
