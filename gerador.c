#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#define PASTA_ENTRADA "entradatxt"
#define PASTA_SAIDA   "saida"
#define MAX_LINHA     1024
#define NUM_CAT       5

/* 1 = o rótulo do link final é o nome do arquivo (kruskal)
   0 = o rótulo é o campo 2 (Kruskal para árvore espalhada mínima) */
#define ROTULO_PELO_ARQUIVO 0

static const char *CAT_ROTULO[NUM_CAT] = {"Técnica", "Algoritmo", "Autores", "Data", "Conceitos"};
static const char *CAT_SLUG[NUM_CAT]   = {"tecnica", "algoritmo", "autores", "data", "conceitos"};
/* campos que aceitam vários valores separados por vírgula */
static const int   CAT_MULTI[NUM_CAT]  = {1, 0, 1, 0, 1};

static const char *CSS =
    "body{font-family:system-ui,sans-serif;max-width:720px;margin:40px auto;padding:0 16px;color:#222}"
    ".botoes{display:flex;flex-wrap:wrap;gap:12px}"
    ".botao{display:inline-block;padding:14px 24px;background:#2b6cb0;color:#fff;"
    "border-radius:8px;text-decoration:none;font-size:1.1rem}"
    ".botao:hover{background:#1e4e8c}"
    "details{margin:8px 0;border:1px solid #ccc;border-radius:8px;padding:8px 14px}"
    "summary{cursor:pointer;font-weight:600;font-size:1.05rem}"
    "ul{margin:8px 0 4px}li{margin:4px 0}.n{color:#777;font-weight:400}";

/* ---------- estruturas ---------- */

typedef struct {
    char base[256];                    /* nome do arquivo sem .txt */
    char rotulo[MAX_LINHA];            /* texto exibido no link final */
    char campo[NUM_CAT][MAX_LINHA];    /* campos 1..5 */
} Arquivo;

typedef struct {
    char texto[MAX_LINHA];             /* ex.: "Método guloso" */
    int *idx;                          /* arquivos que têm esse valor */
    int n, cap;
} Valor;

typedef struct {
    Valor *v;
    int n, cap;
} Categoria;

static Arquivo  *arq = NULL;
static int       n_arq = 0, cap_arq = 0;
static Categoria cat[NUM_CAT];

/* ---------- utilidades ---------- */

static char *trim(char *s) {
    while (isspace((unsigned char)*s)) s++;
    char *fim = s + strlen(s);
    while (fim > s && isspace((unsigned char)fim[-1])) *--fim = '\0';
    return s;
}

static int cmp_ci(const char *a, const char *b) {
    while (*a && *b) {
        int d = tolower((unsigned char)*a) - tolower((unsigned char)*b);
        if (d) return d;
        a++; b++;
    }
    return tolower((unsigned char)*a) - tolower((unsigned char)*b);
}

static void *checar(void *p) {
    if (!p) { fprintf(stderr, "Sem memória\n"); exit(1); }
    return p;
}

/* escreve texto escapando < > & " */
static void esc(FILE *o, const char *s) {
    for (; *s; s++) {
        switch (*s) {
            case '<': fputs("&lt;", o);   break;
            case '>': fputs("&gt;", o);   break;
            case '&': fputs("&amp;", o);  break;
            case '"': fputs("&quot;", o); break;
            default:  fputc(*s, o);
        }
    }
}

/* escreve um nome de arquivo de forma segura para usar em href */
static void href(FILE *o, const char *s) {
    for (; *s; s++) {
        unsigned char c = (unsigned char)*s;
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') fputc(c, o);
        else fprintf(o, "%%%02X", c);
    }
}

static int reservado(const char *base) {
    if (strcmp(base, "index") == 0) return 1;
    for (int c = 0; c < NUM_CAT; c++)
        if (strcmp(base, CAT_SLUG[c]) == 0) return 1;
    return 0;
}

/* ---------- leitura dos .txt ---------- */

static void ler_arquivo(const char *caminho, const char *nome) {
    char base[256];
    strncpy(base, nome, sizeof base - 1);
    base[sizeof base - 1] = '\0';
    char *ponto = strrchr(base, '.');
    if (ponto) *ponto = '\0';

    if (reservado(base)) {
        printf("[ignorado] %s: o nome '%s.html' conflita com uma página do índice\n", nome, base);
        return;
    }

    FILE *f = fopen(caminho, "r");
    if (!f) { perror(caminho); return; }

    if (n_arq == cap_arq) {
        cap_arq = cap_arq ? cap_arq * 2 : 32;
        arq = checar(realloc(arq, cap_arq * sizeof *arq));
    }
    Arquivo *a = &arq[n_arq];
    memset(a, 0, sizeof *a);
    strcpy(a->base, base);

    char linha[MAX_LINHA];
    int primeira = 1, achou = 0;
    while (fgets(linha, sizeof linha, f)) {
        char *s = linha;
        if (primeira) {                         /* pula BOM do UTF-8, se houver */
            if ((unsigned char)s[0] == 0xEF && (unsigned char)s[1] == 0xBB &&
                (unsigned char)s[2] == 0xBF) s += 3;
            primeira = 0;
        }
        while (isspace((unsigned char)*s)) s++;

        char *fim;
        long n = strtol(s, &fim, 10);
        if (fim == s || *fim != '.') break;     /* acabou o cabeçalho */
        char *dp = strchr(fim, ':');
        if (!dp) break;

        if (n >= 1 && n <= NUM_CAT) {           /* 6 (Problema) e outros são ignorados */
            strncpy(a->campo[n - 1], trim(dp + 1), MAX_LINHA - 1);
            achou = 1;
        }
    }
    fclose(f);

    if (!achou) {
        printf("[ignorado] %s: sem cabeçalho numerado\n", nome);
        return;
    }

    if (!ROTULO_PELO_ARQUIVO && a->campo[1][0]) strcpy(a->rotulo, a->campo[1]);
    else                                        strcpy(a->rotulo, a->base);
    n_arq++;
}

/* ---------- montagem das categorias ---------- */

static void adicionar(int c, const char *texto, int ia) {
    Categoria *C = &cat[c];
    Valor *val = NULL;

    for (int i = 0; i < C->n; i++)
        if (cmp_ci(C->v[i].texto, texto) == 0) { val = &C->v[i]; break; }

    if (!val) {
        if (C->n == C->cap) {
            C->cap = C->cap ? C->cap * 2 : 16;
            C->v = checar(realloc(C->v, C->cap * sizeof *C->v));
        }
        val = &C->v[C->n++];
        memset(val, 0, sizeof *val);
        strncpy(val->texto, texto, MAX_LINHA - 1);
    }

    for (int j = 0; j < val->n; j++)
        if (val->idx[j] == ia) return;          /* já registrado */

    if (val->n == val->cap) {
        val->cap = val->cap ? val->cap * 2 : 8;
        val->idx = checar(realloc(val->idx, val->cap * sizeof *val->idx));
    }
    val->idx[val->n++] = ia;
}

static void montar_categorias(void) {
    for (int i = 0; i < n_arq; i++) {
        for (int c = 0; c < NUM_CAT; c++) {
            if (!arq[i].campo[c][0]) continue;

            if (CAT_MULTI[c]) {
                char copia[MAX_LINHA];
                strcpy(copia, arq[i].campo[c]);
                for (char *tok = strtok(copia, ","); tok; tok = strtok(NULL, ",")) {
                    char *t = trim(tok);
                    if (*t) adicionar(c, t, i);
                }
            } else {
                adicionar(c, arq[i].campo[c], i);
            }
        }
    }
}

static int cmp_valor(const void *a, const void *b) {
    return cmp_ci(((const Valor *)a)->texto, ((const Valor *)b)->texto);
}
static int cmp_idx(const void *a, const void *b) {
    return cmp_ci(arq[*(const int *)a].rotulo, arq[*(const int *)b].rotulo);
}

/* ---------- geração dos HTMLs ---------- */

static void cabecalho(FILE *o, const char *titulo) {
    fputs("<!DOCTYPE html>\n<html lang=\"pt-BR\"><head><meta charset=\"utf-8\">\n"
          "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n<title>", o);
    esc(o, titulo);
    fprintf(o, "</title>\n<style>%s</style></head><body>\n", CSS);
}

static void gerar_index(void) {
    char caminho[1024];
    snprintf(caminho, sizeof caminho, "%s/index.html", PASTA_SAIDA);
    FILE *o = fopen(caminho, "w");
    if (!o) { perror(caminho); return; }

    cabecalho(o, "Catálogo de algoritmos");
    fputs("<h1>Catálogo de algoritmos</h1>\n<p>Navegue por:</p>\n<div class=\"botoes\">\n", o);
    for (int c = 0; c < NUM_CAT; c++)
        fprintf(o, "  <a class=\"botao\" href=\"%s.html\">%s</a>\n", CAT_SLUG[c], CAT_ROTULO[c]);
    fputs("</div>\n</body></html>\n", o);
    fclose(o);
}

static void gerar_categoria(int c) {
    char caminho[1024];
    snprintf(caminho, sizeof caminho, "%s/%s.html", PASTA_SAIDA, CAT_SLUG[c]);
    FILE *o = fopen(caminho, "w");
    if (!o) { perror(caminho); return; }

    Categoria *C = &cat[c];
    qsort(C->v, C->n, sizeof *C->v, cmp_valor);

    cabecalho(o, CAT_ROTULO[c]);
    fputs("<p><a href=\"index.html\">&larr; Início</a></p>\n", o);
    fprintf(o, "<h1>%s</h1>\n", CAT_ROTULO[c]);

    if (C->n == 0) fputs("<p>Nenhum item encontrado.</p>\n", o);

    for (int i = 0; i < C->n; i++) {
        Valor *v = &C->v[i];
        qsort(v->idx, v->n, sizeof *v->idx, cmp_idx);

        fputs("<details><summary>", o);
        esc(o, v->texto);
        fprintf(o, " <span class=\"n\">(%d)</span></summary>\n<ul>\n", v->n);
        for (int j = 0; j < v->n; j++) {
            Arquivo *a = &arq[v->idx[j]];
            fputs("  <li><a href=\"", o);
            href(o, a->base);
            fputs(".html\">", o);
            esc(o, a->rotulo);
            fputs("</a></li>\n", o);
        }
        fputs("</ul></details>\n", o);
    }
    fputs("</body></html>\n", o);
    fclose(o);
}

/* ---------- main ---------- */

int main(void) {
    DIR *d = opendir(PASTA_ENTRADA);
    if (!d) { perror("Erro ao abrir " PASTA_ENTRADA); return 1; }

    struct dirent *e;
    while ((e = readdir(d))) {
        size_t n = strlen(e->d_name);
        if (n < 5) continue;
        const char *ext = e->d_name + n - 4;
        if (ext[0] != '.' || tolower((unsigned char)ext[1]) != 't' ||
            tolower((unsigned char)ext[2]) != 'x' || tolower((unsigned char)ext[3]) != 't') continue;

        char caminho[1024];
        snprintf(caminho, sizeof caminho, "%s/%s", PASTA_ENTRADA, e->d_name);
        ler_arquivo(caminho, e->d_name);
    }
    closedir(d);

    montar_categorias();
    gerar_index();
    for (int c = 0; c < NUM_CAT; c++) gerar_categoria(c);

    printf("\n%d arquivo(s) lido(s). HTMLs gerados em '%s/'.\n", n_arq, PASTA_SAIDA);
    for (int c = 0; c < NUM_CAT; c++)
        printf("  %-10s %d valor(es)\n", CAT_ROTULO[c], cat[c].n);
    return 0;
}