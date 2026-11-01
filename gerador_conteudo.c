/* Compilar: gcc -Wall -Wextra -pedantic -std=c99 gerador_conteudo.c -o gerador_conteudo */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#ifdef _WIN32
#include <direct.h>
#endif

static const char *entrada = "entradatxt", *saida = "saida";
static int erros;

/* Leitura dinâmica: uma linha longa nunca vira dois parágrafos por acidente. */
static char *ler_linha(FILE *f) {
    size_t n = 0, cap = 128;
    char *s = malloc(cap);
    int c;
    if (!s) { perror("malloc"); exit(EXIT_FAILURE); }
    while ((c = fgetc(f)) != EOF && c != '\n') {
        if (n + 1 == cap) {
            char *novo;
            cap *= 2;
            novo = realloc(s, cap);
            if (!novo) { free(s); perror("realloc"); exit(EXIT_FAILURE); }
            s = novo;
        }
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

static void escapar(FILE *o, const char *s, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        switch (s[i]) {
            case '&': fputs("&amp;", o); break;
            case '<': fputs("&lt;", o); break;
            case '>': fputs("&gt;", o); break;
            case '"': fputs("&quot;", o); break;
            default: fputc((unsigned char)s[i], o);
        }
    }
}

static int identificador(const char *s) {
    if (!*s) return 0;
    for (; *s; s++)
        if (!((*s >= 'a' && *s <= 'z') || (*s >= '0' && *s <= '9') ||
              *s == '_' || *s == '-')) return 0;
    return 1;
}

static char *caminho(const char *pasta, const char *base, const char *ext) {
    size_t n = strlen(pasta) + strlen(base) + strlen(ext) + 2;
    char *s = malloc(n);
    if (!s) { perror("malloc"); exit(EXIT_FAILURE); }
    snprintf(s, n, "%s/%s%s", pasta, base, ext);
    return s;
}

static void texto(FILE *o, const char *s, const char *origem) {
    while (*s) {
        const char *ab = strstr(s, "[["), *fe, *barra;
        char *alvo, *p;
        FILE *f;
        size_t n;
        if (!ab || !(fe = strstr(ab + 2, "]]"))) {
            escapar(o, s, strlen(s)); return;
        }
        escapar(o, s, (size_t)(ab - s));
        barra = memchr(ab + 2, '|', (size_t)(fe - ab - 2));
        n = (size_t)((barra ? barra : fe) - (ab + 2));
        alvo = malloc(n + 1);
        if (!alvo) { perror("malloc"); exit(EXIT_FAILURE); }
        memcpy(alvo, ab + 2, n); alvo[n] = '\0';
        p = trim(alvo);
        if (identificador(p)) {
            char *nome = caminho(entrada, p, ".txt");
            f = fopen(nome, "r");
            if (!f) fprintf(stderr, "[aviso] %s: alvo ausente: %s\n", origem, nome);
            else fclose(f);
            free(nome);
            fprintf(o, "<a href=\"%s.html\">", p);
            if (barra) escapar(o, barra + 1, (size_t)(fe - barra - 1));
            else escapar(o, p, strlen(p));
            fputs("</a>", o);
        } else {
            fprintf(stderr, "[aviso] %s: alvo inválido\n", origem);
            escapar(o, ab, (size_t)(fe + 2 - ab));
        }
        free(alvo);
        s = fe + 2;
    }
}

typedef struct {
    int paragrafo, pre, codigo, blocos, nivel;
    struct { int recuo, ordenada; } lista[64];
} Estado;

static void fechar_listas(FILE *o, Estado *e, int manter) {
    while (e->nivel > manter) {
        e->nivel--;
        fprintf(o, "</li></%s>\n", e->lista[e->nivel].ordenada ? "ol" : "ul");
    }
}

static void fechar(FILE *o, Estado *e) {
    if (e->paragrafo) { fputs("</p>\n", o); e->paragrafo = 0; }
    fechar_listas(o, e, 0);
}

static void corpo(FILE *o, FILE *f, const char *nome, const char *base) {
    Estado e = {0};
    char *linha;
    while ((linha = ler_linha(f))) {
        char *s = linha;
        int recuo = 0, ordenada = 0, item = 0;
        if (e.codigo) {
            if (!strcmp(linha, "FIM_CODIGO_C") || !strcmp(linha, "[/codigo]")) e.codigo = 0;
            free(linha); continue;
        }
        if (!strcmp(linha, "```")) {
            if (e.pre) fputs("</pre>\n", o);
            else { fechar(o, &e); fputs("<pre>", o); }
            e.pre = !e.pre;
            free(linha); continue;
        }
        if (e.pre) {
            escapar(o, linha, strlen(linha)); fputc('\n', o);
            free(linha); continue;
        }
        while (*s == ' ' || *s == '\t') { recuo += *s == '\t' ? 4 : 1; s++; }
        s = trim(s);
        if (!strcmp(s, "INICIO_CODIGO_C") || !strcmp(s, "[codigo]")) {
            fechar(o, &e); e.codigo = 1; e.blocos++;
        } else if (!*s) fechar(o, &e);
        else if (!strcmp(s, "REFERENCIAS:") || !strcmp(s, "Referências:")) {
            fechar(o, &e); fputs("<h2>Referências</h2>\n", o);
        } else if (!strncmp(s, "## ", 3)) {
            fechar(o, &e); fputs("<h2>", o); texto(o, s + 3, nome); fputs("</h2>\n", o);
        } else {
            char *conteudo = s;
            if (!strncmp(s, "- ", 2)) { item = 1; conteudo = s + 2; }
            else if (isdigit((unsigned char)*s)) {
                char *p = s;
                while (isdigit((unsigned char)*p)) p++;
                if (*p == ')' && p[1] == ' ') { item = ordenada = 1; conteudo = p + 2; }
            }
            if (item) {
                if (e.paragrafo) { fputs("</p>\n", o); e.paragrafo = 0; }
                while (e.nivel && recuo < e.lista[e.nivel - 1].recuo)
                    fechar_listas(o, &e, e.nivel - 1);
                if (e.nivel && recuo == e.lista[e.nivel - 1].recuo &&
                    ordenada != e.lista[e.nivel - 1].ordenada)
                    fechar_listas(o, &e, e.nivel - 1);
                if (!e.nivel || recuo > e.lista[e.nivel - 1].recuo) {
                    if (e.nivel == 64) {
                        fprintf(stderr, "%s: listas profundas demais\n", nome); erros++;
                        free(linha); continue;
                    }
                    e.lista[e.nivel].recuo = recuo;
                    e.lista[e.nivel++].ordenada = ordenada;
                    fprintf(o, "<%s><li>", ordenada ? "ol" : "ul");
                } else fputs("</li>\n<li>", o);
                texto(o, conteudo, nome);
            } else if (e.nivel && recuo > e.lista[e.nivel - 1].recuo) {
                fputc(' ', o); texto(o, s, nome);
            } else {
                fechar_listas(o, &e, 0);
                if (!e.paragrafo) { fputs("<p>", o); e.paragrafo = 1; }
                else fputc(' ', o);
                texto(o, s, nome);
            }
        }
        free(linha);
    }
    fechar(o, &e);
    if (e.pre) fputs("</pre>\n", o);
    if (e.codigo || e.pre) {
        fprintf(stderr, "%s: bloco sem marcador de fechamento\n", nome); erros++;
    }
    if (e.blocos)
        fprintf(o, "<p class=\"codigo\"><a href=\"%s_codigo.html\">"
                "Ver código em C</a></p>\n", base);
}

static int gerar(const char *base) {
    char *in = caminho(entrada, base, ".txt"), *out = caminho(saida, base, ".html");
    char *linha, *titulo = NULL;
    FILE *f = fopen(in, "r"), *o;
    int primeira = 1, falha;
    if (!f) { perror(in); free(in); free(out); return 0; }
    while ((linha = ler_linha(f))) {
        char *s = linha, *fim;
        long numero;
        if (primeira && strlen(s) >= 3 && !memcmp(s, "\xef\xbb\xbf", 3)) s += 3;
        primeira = 0;
        s = trim(s);
        if (!*s) { free(linha); break; }
        numero = strtol(s, &fim, 10);
        if (fim == s || *fim != '.' || !(fim = strchr(fim, ':'))) {
            fprintf(stderr, "%s: cabeçalho inválido\n", in);
            free(linha); free(titulo); fclose(f); free(in); free(out); return 0;
        }
        if (numero == 7) {
            s = trim(fim + 1);
            free(titulo); titulo = malloc(strlen(s) + 1);
            if (!titulo) { perror("malloc"); exit(EXIT_FAILURE); }
            strcpy(titulo, s);
        }
        free(linha);
    }
    if (!titulo || !*titulo) {
        fprintf(stderr, "%s: Titulo obrigatório ausente\n", in);
        free(titulo); fclose(f); free(in); free(out); return 0;
    }
    o = fopen(out, "w");
    if (!o) { perror(out); free(titulo); fclose(f); free(in); free(out); return 0; }
    fputs("<!DOCTYPE html>\n<html lang=\"pt-BR\"><head><meta charset=\"utf-8\">\n"
          "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n<title>", o);
    escapar(o, titulo, strlen(titulo));
    fputs("</title><style>body{font-family:system-ui,sans-serif;max-width:720px;"
          "margin:40px auto;padding:0 16px;color:#222;line-height:1.6}"
          "pre{overflow:auto;background:#f3f4f6;padding:16px;line-height:1.4}"
          "a{color:#2b6cb0}</style></head><body>\n"
          "<nav><a href=\"index.html\">&larr; Início</a></nav>\n<main><h1>", o);
    escapar(o, titulo, strlen(titulo)); fputs("</h1>\n", o);
    corpo(o, f, in, base);
    fputs("</main></body></html>\n", o);
    falha = ferror(f) || ferror(o);
    if (fclose(o)) falha = 1;
    fclose(f); free(titulo); free(in); free(out);
    return !falha;
}

int main(int argc, char **argv) {
    DIR *d;
    struct dirent *e;
    int total = 0;
    if (argc != 1 && argc != 3) {
        fprintf(stderr, "Uso: %s [pasta_entrada pasta_saida]\n", argv[0]); return EXIT_FAILURE;
    }
    if (argc == 3) { entrada = argv[1]; saida = argv[2]; }
    d = opendir(entrada);
    if (!d) { perror(entrada); return EXIT_FAILURE; }
#ifdef _WIN32
    if (_mkdir(saida) && errno != EEXIST) {
#else
    if (mkdir(saida, 0777) && errno != EEXIST) {
#endif
        perror(saida); closedir(d); return EXIT_FAILURE;
    }
    while ((e = readdir(d))) {
        size_t n = strlen(e->d_name);
        char *base;
        if (n < 5 || strcmp(e->d_name + n - 4, ".txt")) continue;
        base = malloc(n - 3);
        if (!base) { perror("malloc"); exit(EXIT_FAILURE); }
        memcpy(base, e->d_name, n - 4); base[n - 4] = '\0';
        if (!identificador(base) || !strcmp(base, "index") || !strcmp(base, "tecnica") ||
            !strcmp(base, "algoritmo") || !strcmp(base, "autores") || !strcmp(base, "data") ||
            !strcmp(base, "conceitos") || !strcmp(base, "problema")) {
            fprintf(stderr, "Nome inválido ou reservado: %s\n", base); erros++;
        } else if (gerar(base)) total++;
        else erros++;
        free(base);
    }
    closedir(d);
    printf("%d página(s) de conteúdo gerada(s) em '%s'.\n", total, saida);
    return erros ? EXIT_FAILURE : EXIT_SUCCESS;
}
