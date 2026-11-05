/* txt.h: leitura dos blocos de código dos TXT, comum ao colorir.c e ao
   exportar.c.

   Um TXT pode ter um ou mais blocos de código C, cada um entre as linhas
   INICIO_CODIGO_C e FIM_CODIGO_C (documentacao/formato_txt.md). Além dos
   blocos, ler_txt guarda o título da página (marcador 7) e o texto da seção
   "## Sobre o código", usado no COMO_COMPILAR.txt. */
#ifndef TXT_H
#define TXT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void *checar(void *p) {
    if (!p) { perror("memória"); exit(EXIT_FAILURE); }
    return p;
}

/* texto que cresce conforme a necessidade */
typedef struct { char *s; size_t n, cap; } Texto;

static void anexar(Texto *t, const char *s, size_t n) {
    if (t->n + n + 1 > t->cap) {
        size_t cap = t->cap ? t->cap : 256;
        while (cap < t->n + n + 1) cap *= 2;
        t->s = checar(realloc(t->s, cap));
        t->cap = cap;
    }
    memcpy(t->s + t->n, s, n);
    t->n += n;
    t->s[t->n] = '\0';
}

/* lê uma linha de qualquer tamanho, sem o '\n' (e sem o '\r' do Windows) */
static char *ler_linha(FILE *f) {
    size_t n = 0, cap = 128;
    char *s = checar(malloc(cap));
    int c;
    while ((c = fgetc(f)) != EOF && c != '\n') {
        if (n + 1 == cap) s = checar(realloc(s, cap *= 2));
        s[n++] = (char)c;
    }
    if (!n && c == EOF) { free(s); return NULL; }
    if (n && s[n - 1] == '\r') n--;
    s[n] = '\0';
    return s;
}

static char *trim(char *s) {
    char *fim;
    while (isspace((unsigned char)*s)) s++;
    fim = s + strlen(s);
    while (fim > s && isspace((unsigned char)fim[-1])) *--fim = '\0';
    return s;
}

/* a linha, sem os espaços das pontas, é exatamente a marca? */
static int eh_marca(const char *linha, const char *marca) {
    size_t n = strlen(marca);
    while (*linha == ' ' || *linha == '\t') linha++;
    if (strncmp(linha, marca, n)) return 0;
    for (linha += n; *linha; linha++)
        if (!isspace((unsigned char)*linha)) return 0;
    return 1;
}

/* nome-base aceito para um TXT: letras minúsculas, dígitos, _ e - */
static int identificador(const char *s) {
    if (!*s) return 0;
    for (; *s; s++)
        if (!((*s >= 'a' && *s <= 'z') || (*s >= '0' && *s <= '9') ||
              *s == '_' || *s == '-')) return 0;
    return 1;
}

/* devolve "pasta/base+ext" alocado */
static char *caminho(const char *pasta, const char *base, const char *ext) {
    size_t n = strlen(pasta) + strlen(base) + strlen(ext) + 2;
    char *s = checar(malloc(n));
    sprintf(s, "%s/%s%s", pasta, base, ext);  /* n bytes bastam */
    return s;
}

/* nome do arquivo .c do bloco: kruskal.c, ou kruskal_2.c a partir do 2º.
   Se não couber em tam bytes, dest fica vazio. */
static void nome_arquivo(char *dest, size_t tam, const char *base, int numero) {
    if (strlen(base) + 16 > tam) { if (tam) dest[0] = '\0'; return; }
    if (numero == 1) sprintf(dest, "%s.c", base);
    else             sprintf(dest, "%s_%d.c", base, numero);
}

/* o que interessa de um TXT para os códigos */
typedef struct {
    char *titulo;        /* marcador 7 (NULL se não houver) */
    Texto sobre;         /* texto da seção "## Sobre o código", numa linha só */
    Texto *blocos;       /* um Texto por bloco de código */
    int nb;
} CodigoTxt;

/* Lê o TXT do caminho dado. Devolve 1 se deu certo e 0 em erro (arquivo
   que não abre ou bloco sem FIM_CODIGO_C); a mensagem já sai em stderr. */
static int ler_txt(const char *arq, CodigoTxt *r) {
    FILE *f = fopen(arq, "r");
    char *linha;
    int dentro = 0, cabecalho = 1, primeira = 1, em_sobre = 0;

    memset(r, 0, sizeof *r);
    if (!f) { perror(arq); return 0; }
    while ((linha = ler_linha(f))) {
        char *s = linha;
        if (primeira && !strncmp(s, "\xef\xbb\xbf", 3)) s += 3;   /* BOM */
        primeira = 0;

        if (cabecalho) {                       /* do cabeçalho, só o Titulo */
            char *t = trim(s), *fim;
            long num = strtol(t, &fim, 10);
            if (!*t || fim == t || *fim != '.' || !(fim = strchr(fim, ':'))) {
                cabecalho = 0;
            } else {
                if (num == 7) {
                    free(r->titulo);
                    r->titulo = checar(malloc(strlen(fim + 1) + 1));
                    strcpy(r->titulo, trim(fim + 1));
                }
                free(linha);
                continue;
            }
        }

        if (!dentro) {                         /* seção "## Sobre o código" */
            if (eh_marca(s, "## Sobre o código")) { em_sobre = 1; free(linha); continue; }
            if (em_sobre && (!strncmp(s, "## ", 3) || eh_marca(s, "REFERENCIAS:") ||
                             eh_marca(s, "INICIO_CODIGO_C") || eh_marca(s, "[codigo]")))
                em_sobre = 0;
            if (em_sobre) {
                char *t = trim(s);
                if (*t) {
                    if (r->sobre.n) anexar(&r->sobre, " ", 1);
                    anexar(&r->sobre, t, strlen(t));
                }
                free(linha);
                continue;
            }
        }

        if (dentro) {
            if (eh_marca(s, "FIM_CODIGO_C") || eh_marca(s, "[/codigo]")) dentro = 0;
            else {
                anexar(&r->blocos[r->nb - 1], s, strlen(s));
                anexar(&r->blocos[r->nb - 1], "\n", 1);
            }
        } else if (eh_marca(s, "INICIO_CODIGO_C") || eh_marca(s, "[codigo]")) {
            r->blocos = checar(realloc(r->blocos, (size_t)(r->nb + 1) * sizeof *r->blocos));
            memset(&r->blocos[r->nb++], 0, sizeof *r->blocos);
            dentro = 1;
        }
        free(linha);
    }
    fclose(f);
    if (dentro) {
        fprintf(stderr, "%s: bloco de código sem FIM_CODIGO_C\n", arq);
        return 0;
    }
    return 1;
}

static void liberar_txt(CodigoTxt *r) {
    int b;
    for (b = 0; b < r->nb; b++) free(r->blocos[b].s);
    free(r->blocos);
    free(r->sobre.s);
    free(r->titulo);
    memset(r, 0, sizeof *r);
}

#endif
