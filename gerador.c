#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#define PASTA_ENTRADA "entradatxt"
#define PASTA_SAIDA   "saida"
#define MAX_LINHA     4096
#define NUM_CAT       6
#define CAMPO_TITULO  7      /* "7. Titulo:" nomeia a página; não é categoria */

/* Regras do formato: documentacao/formato_txt.md */

/* 1 = o rótulo do link final é sempre o nome do arquivo (kruskal)
   0 = o rótulo é o campo 7 (Titulo); sem título, o campo 2 (Algoritmo);
       sem os dois, o nome do arquivo */
#define ROTULO_PELO_ARQUIVO 0

enum { C_TECNICA, C_ALGORITMO, C_AUTORES, C_DATA, C_CONCEITOS, C_PROBLEMA };

static const char *CAT_ROTULO[NUM_CAT] = {"Técnica", "Algoritmo", "Autores", "Data", "Conceitos", "Problema"};
static const char *CAT_SLUG[NUM_CAT]   = {"tecnica", "algoritmo", "autores", "data", "conceitos", "problema"};
/* campos que aceitam vários valores, separados por vírgula */
static const int   CAT_MULTI[NUM_CAT]  = {1, 0, 1, 1, 1, 0};

/* Vocabulário fechado de Conceitos e a página que explica cada um
   (NULL = sem página explicativa). Mantido igual à tabela do formato_txt.md. */
static const struct { const char *conceito, *pagina; } VOCABULARIO[] = {
    {"Árvores", "arvores"},                 {"Hash", "hash"},
    {"Árvore espalhada mínima", "kruskal"}, {"Invariante", NULL},
    {"Busca binária", "busca_binaria"},     {"Listas encadeadas", "listas_encadeadas"},
    {"Caminho mínimo", "dijkstra"},         {"Memorização", "programacao_dinamica"},
    {"Conjuntos disjuntos", "kruskal"},     {"Ordenação", "ordenacao"},
    {"Eficiência de algoritmos", "eficiencia"}, {"Pilhas", "pilhas"},
    {"Estabilidade", "estabilidade"},          {"Ponteiros", "ponteiros"},
    {"Filas", "filas"},                     {"Registros", "registros"},
    {"Fila de prioridade", "heapsort"},     {"Recursão", "recursao"},
    {"Grafos", "grafos"},                   {"Subestrutura ótima", "programacao_dinamica"},
    {"Vetores", NULL},
};
#define N_VOCAB ((int)(sizeof VOCABULARIO / sizeof VOCABULARIO[0]))

static const char *CSS =
    "body{font-family:system-ui,sans-serif;max-width:720px;margin:40px auto;padding:0 16px;color:#222}"
    ".botoes{display:flex;flex-wrap:wrap;gap:12px}"
    ".botao{display:inline-block;padding:14px 24px;background:#2b6cb0;color:#fff;"
    "border-radius:8px;text-decoration:none;font-size:1.1rem}"
    ".botao:hover{background:#1e4e8c}"
    "details{margin:8px 0;border:1px solid #ccc;border-radius:8px;padding:8px 14px}"
    "summary{cursor:pointer;font-weight:600;font-size:1.05rem}"
    "ul{margin:8px 0 4px}li{margin:4px 0}.n{color:#777;font-weight:400}"
    ".exp{margin:8px 0 0;font-size:.95rem;color:#555}";

/* ---------- estruturas ---------- */

typedef struct {
    char base[256];                    /* nome do arquivo sem .txt */
    char rotulo[MAX_LINHA];            /* texto exibido no link final */
    char campo[NUM_CAT][MAX_LINHA];    /* campos 1..6 */
    char titulo[MAX_LINHA];            /* campo 7 (pode ficar vazio) */
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

/* escreve texto escapado, transformando [[alvo]] e [[alvo|texto]]
   em <a href="alvo.html">...</a> (usado no campo Problema) */
static void esc_links(FILE *o, const char *s) {
    char buf[MAX_LINHA];
    while (*s) {
        const char *ab = strstr(s, "[[");
        const char *fe = ab ? strstr(ab + 2, "]]") : NULL;
        if (!ab || !fe) { esc(o, s); return; }      /* sem link: resto literal */

        size_t n = (size_t)(ab - s);                 /* texto antes do link */
        memcpy(buf, s, n); buf[n] = '\0';
        esc(o, buf);

        n = (size_t)(fe - (ab + 2));                 /* conteúdo entre [[ e ]] */
        memcpy(buf, ab + 2, n); buf[n] = '\0';
        char *barra = strchr(buf, '|');
        char *texto = buf;
        if (barra) { *barra = '\0'; texto = barra + 1; }

        fputs("<a href=\"", o);
        href(o, trim(buf));
        fputs(".html\">", o);
        esc(o, trim(texto));
        fputs("</a>", o);
        s = fe + 2;
    }
}

static int existe_arquivo(const char *base);

/* avisa sobre [[alvo]] cujo alvo.txt não existe */
static void checar_links(const char *s, const char *nome) {
    char alvo[256];
    while ((s = strstr(s, "[["))) {
        const char *fe = strstr(s + 2, "]]");
        if (!fe) return;
        size_t n = strcspn(s + 2, "|]");
        if (n >= sizeof alvo) n = sizeof alvo - 1;
        memcpy(alvo, s + 2, n); alvo[n] = '\0';
        if (!existe_arquivo(alvo))
            printf("[aviso] %s: link [[%s]] sem %s.txt\n", nome, alvo, alvo);
        s = fe + 2;
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

        if (n >= 1 && n <= NUM_CAT) {           /* campos 1..6: categorias */
            strncpy(a->campo[n - 1], trim(dp + 1), MAX_LINHA - 1);
            achou = 1;
        } else if (n == CAMPO_TITULO) {         /* campo 7: título da página */
            strncpy(a->titulo, trim(dp + 1), MAX_LINHA - 1);
        }                                       /* outros números são ignorados */
    }
    fclose(f);

    if (!achou) {
        printf("[ignorado] %s: sem cabeçalho numerado\n", nome);
        return;
    }

    if      (ROTULO_PELO_ARQUIVO) strcpy(a->rotulo, a->base);
    else if (a->titulo[0])        strcpy(a->rotulo, a->titulo);
    else if (a->campo[1][0])      strcpy(a->rotulo, a->campo[1]);
    else                          strcpy(a->rotulo, a->base);
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

static int existe_arquivo(const char *base) {
    for (int i = 0; i < n_arq; i++)
        if (strcmp(arq[i].base, base) == 0) return 1;
    return 0;
}

/* posição do conceito no vocabulário, ou -1 */
static int no_vocabulario(const char *conceito) {
    for (int k = 0; k < N_VOCAB; k++)
        if (strcmp(VOCABULARIO[k].conceito, conceito) == 0) return k;
    return -1;
}

static void montar_categorias(void) {
    for (int i = 0; i < n_arq; i++) {
        char nome[300];
        snprintf(nome, sizeof nome, "%s.txt", arq[i].base);
        checar_links(arq[i].campo[C_PROBLEMA], nome);

        for (int c = 0; c < NUM_CAT; c++) {
            if (!arq[i].campo[c][0]) continue;

            if (CAT_MULTI[c]) {
                /* separa na vírgula e remove os espaços das pontas */
                char copia[MAX_LINHA];
                strcpy(copia, arq[i].campo[c]);
                for (char *tok = strtok(copia, ","); tok; tok = strtok(NULL, ",")) {
                    char *t = trim(tok);
                    if (!*t) continue;
                    if (c == C_CONCEITOS && no_vocabulario(t) < 0)
                        printf("[aviso] %s: conceito fora do vocabulário: %s\n", nome, t);
                    adicionar(c, t, i);
                }
            } else {
                adicionar(c, arq[i].campo[c], i);
            }
        }
    }
}

/* Texto do link de um arquivo dentro de uma categoria. Em Autores e Data o
   link mostra o Algoritmo (a obra datada e atribuída); sem Algoritmo, o
   título. Nas demais categorias, sempre o título. */
static const char *rotulo_em(int c, const Arquivo *a) {
    if ((c == C_AUTORES || c == C_DATA) && a->campo[C_ALGORITMO][0] && !ROTULO_PELO_ARQUIVO)
        return a->campo[C_ALGORITMO];
    return a->rotulo;
}

static int cat_atual;                   /* categoria usada por cmp_idx */

static int cmp_valor(const void *a, const void *b) {
    return cmp_ci(((const Valor *)a)->texto, ((const Valor *)b)->texto);
}
static int cmp_idx(const void *a, const void *b) {
    return cmp_ci(rotulo_em(cat_atual, &arq[*(const int *)a]),
                  rotulo_em(cat_atual, &arq[*(const int *)b]));
}

/* índice do arquivo com esse nome-base, ou -1 */
static int achar_arquivo(const char *base) {
    for (int i = 0; i < n_arq; i++)
        if (strcmp(arq[i].base, base) == 0) return i;
    return -1;
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

    cat_atual = c;
    for (int i = 0; i < C->n; i++) {
        Valor *v = &C->v[i];
        qsort(v->idx, v->n, sizeof *v->idx, cmp_idx);

        fputs("<details><summary>", o);
        if (c == C_PROBLEMA) esc_links(o, v->texto);   /* [[alvo|texto]] vira link */
        else                 esc(o, v->texto);
        fprintf(o, " <span class=\"n\">(%d)</span></summary>\n", v->n);

        /* Conceitos: liga o nome do conceito à página que o explica */
        if (c == C_CONCEITOS) {
            int k = no_vocabulario(v->texto);
            int p = (k >= 0 && VOCABULARIO[k].pagina) ? achar_arquivo(VOCABULARIO[k].pagina) : -1;
            if (p >= 0) {
                fputs("<p class=\"exp\">Explicação: <a href=\"", o);
                href(o, arq[p].base);
                fputs(".html\">", o);
                esc(o, arq[p].rotulo);
                fputs("</a></p>\n", o);
            }
        }

        fputs("<ul>\n", o);
        for (int j = 0; j < v->n; j++) {
            Arquivo *a = &arq[v->idx[j]];
            fputs("  <li><a href=\"", o);
            href(o, a->base);
            fputs(".html\">", o);
            esc(o, rotulo_em(c, a));
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