#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include "slug.h"
#include "layout.h"

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

/* palavras para os resumos: "4 técnicas", "1 autor" */
static const char *const SINGULAR[NUM_CAT] = {"técnica", "algoritmo", "autor", "ano", "conceito", "problema"};
static const char *const PLURAL[NUM_CAT]   = {"técnicas", "algoritmos", "autores", "anos", "conceitos", "problemas"};

/* posição de cada índice no grafo da página inicial (viewBox 800 x 480);
   o nó central fica em (400, 240) */
static const int NO_X[NUM_CAT] = {400, 660, 660, 400, 140, 140};
static const int NO_Y[NUM_CAT] = { 72, 156, 324, 408, 324, 156};

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

/* quantos arquivos têm o campo c preenchido */
static int paginas_com(int c) {
    int n = 0;
    for (int i = 0; i < n_arq; i++) if (arq[i].campo[c][0]) n++;
    return n;
}

/* Página inicial: os seis índices desenhados como um grafo (uma "roda": o
   nó central MAC0122 ligado aos seis, que formam um anel). Cada nó é um
   link. No celular, o grafo dá lugar a uma lista com os mesmos links. */
static void gerar_index(void) {
    char caminho[1024];
    snprintf(caminho, sizeof caminho, "%s/index.html", PASTA_SAIDA);
    FILE *o = fopen(caminho, "w");
    if (!o) { perror(caminho); return; }

    abrir_pagina(o, "Algoritmos de MAC0122", "pagina-inicio", -1, 0, NULL);
    fprintf(o, "<main class=\"inicio\">\n<div class=\"heroi\">\n<div class=\"heroi-texto\">"
               "<h1>Algoritmos de MAC0122</h1>\n"
               "<p class=\"intro\">%d páginas sobre técnicas de projeto, algoritmos e "
               "estruturas de dados da disciplina, cada uma com explicação, referências "
               "e código em C. Elas estão ligadas entre si como um grafo: escolha por "
               "onde entrar.</p>\n</div>\n", n_arq);

    /* o grafo */
    fputs("<svg class=\"grafo\" viewBox=\"0 0 800 480\" role=\"group\" "
          "aria-label=\"Os seis índices do site\">\n", o);
    for (int c = 0; c < NUM_CAT; c++) {          /* anel entre índices vizinhos */
        int d = (c + 1) % NUM_CAT;
        fprintf(o, "<line class=\"anel\" x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\"/>\n",
                NO_X[c], NO_Y[c], NO_X[d], NO_Y[d]);
    }
    for (int c = 0; c < NUM_CAT; c++) {
        int n = cat[c].n;
        fprintf(o, "<a class=\"no f-%s\" href=\"%s.html\" style=\"--i:%d\" "
                   "aria-label=\"%s: %d %s\">\n", CAT_SLUG[c], CAT_SLUG[c], c,
                CAT_ROTULO[c], n, n == 1 ? SINGULAR[c] : PLURAL[c]);
        fprintf(o, "<line class=\"aresta\" pathLength=\"1\" x1=\"400\" y1=\"240\" "
                   "x2=\"%d\" y2=\"%d\"/>\n", NO_X[c], NO_Y[c]);
        fprintf(o, "<circle cx=\"%d\" cy=\"%d\" r=\"60\"/>\n", NO_X[c], NO_Y[c]);
        fprintf(o, "<text class=\"num\" x=\"%d\" y=\"%d\">%d</text>\n", NO_X[c], NO_Y[c] + 4, n);
        fprintf(o, "<text class=\"rot\" x=\"%d\" y=\"%d\">%s</text>\n</a>\n",
                NO_X[c], NO_Y[c] + 30, CAT_ROTULO[c]);
    }
    fprintf(o, "<g class=\"centro\"><circle cx=\"400\" cy=\"240\" r=\"74\"/>"
               "<text x=\"400\" y=\"238\">MAC0122</text>"
               "<text class=\"sub\" x=\"400\" y=\"262\">%d páginas</text></g>\n</svg>\n</div>\n",
            n_arq);

    /* a mesma navegação em lista, para telas estreitas */
    fputs("<ul class=\"lista-facetas\">\n", o);
    for (int c = 0; c < NUM_CAT; c++) {
        int n = cat[c].n;
        fprintf(o, "<li><a class=\"f-%s\" href=\"%s.html\"><span class=\"qtd\">%d</span>"
                   "<span class=\"nome\">%s</span><span class=\"desc\">%d %s em %d páginas"
                   "</span></a></li>\n", CAT_SLUG[c], CAT_SLUG[c], n, CAT_ROTULO[c],
                n, n == 1 ? SINGULAR[c] : PLURAL[c], paginas_com(c));
    }
    fputs("</ul>\n", o);

    /* atalhos: as técnicas da disciplina */
    if (cat[C_TECNICA].n > 0) {
        Categoria *T = &cat[C_TECNICA];
        qsort(T->v, T->n, sizeof *T->v, cmp_valor);
        fputs("<section class=\"atalhos\">\n<h2>As técnicas de projeto da disciplina</h2>\n"
              "<ul class=\"chips\">\n", o);
        for (int i = 0; i < T->n; i++) {
            fputs("<li><a class=\"chip f-tecnica\" href=\"tecnica.html#", o);
            escrever_slug(o, T->v[i].texto, 1);
            fputs("\">", o);
            esc(o, T->v[i].texto);
            fputs("</a></li>\n", o);
        }
        fputs("</ul>\n</section>\n", o);
    }
    fputs("</main>\n", o);
    fechar_pagina(o, 0, NULL);
    fclose(o);
}

static void gerar_categoria(int c) {
    char caminho[1024];
    snprintf(caminho, sizeof caminho, "%s/%s.html", PASTA_SAIDA, CAT_SLUG[c]);
    FILE *o = fopen(caminho, "w");
    if (!o) { perror(caminho); return; }

    Categoria *C = &cat[c];
    qsort(C->v, C->n, sizeof *C->v, cmp_valor);

    char classe[64];
    snprintf(classe, sizeof classe, "pagina-indice f-%s", CAT_SLUG[c]);
    abrir_pagina(o, CAT_ROTULO[c], classe, c, 0, NULL);

    int np = paginas_com(c);
    fprintf(o, "<main class=\"indice\">\n<header class=\"cab-indice\">"
               "<span class=\"no-cat\" aria-hidden=\"true\"></span>"
               "<h1>%s</h1>\n<p class=\"resumo\">%d %s em %d %s. ",
            CAT_ROTULO[c], C->n, C->n == 1 ? SINGULAR[c] : PLURAL[c],
            np, np == 1 ? "página" : "páginas");
    fputs(c == C_CONCEITOS
          ? "Abra um conceito para ver a página que o explica e as que o usam."
          : "Abra um item para ver as páginas.", o);
    fputs("</p>\n</header>\n", o);

    if (C->n == 0) {
        fputs("<p class=\"vazio\">Nenhum TXT preenche este campo ainda. Acrescente-o no "
              "cabeçalho de um arquivo em entradatxt e gere as páginas de novo.</p>\n", o);
    } else {
        fputs("<div class=\"acoes\"><button class=\"acao\" type=\"button\" data-acao=\"abrir\">"
              "Abrir todos</button><button class=\"acao\" type=\"button\" data-acao=\"fechar\">"
              "Fechar todos</button></div>\n<div class=\"grupos\">\n", o);
    }

    cat_atual = c;
    for (int i = 0; i < C->n; i++) {
        Valor *v = &C->v[i];
        qsort(v->idx, v->n, sizeof *v->idx, cmp_idx);

        /* id: âncora usada pela ficha das páginas de conteúdo (slug.h) */
        fputs("<details class=\"grupo\" id=\"", o);
        escrever_slug(o, v->texto, 0);
        fputs("\"><summary><span class=\"valor\">", o);
        if (c == C_PROBLEMA) esc_links(o, v->texto);   /* [[alvo|texto]] vira link */
        else                 esc(o, v->texto);
        fprintf(o, "</span><span class=\"qtd\" title=\"%d %s\">%d</span></summary>\n",
                v->n, v->n == 1 ? "página" : "páginas", v->n);

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

        fputs("<ul class=\"paginas\">\n", o);
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
    if (C->n > 0) fputs("</div>\n", o);
    fputs("</main>\n", o);

    /* Chegando por um link com #âncora, abre e mostra o grupo correspondente;
       os botões abrem ou fecham todos os grupos. */
    fechar_pagina(o, 0,
        "function abrir(){var e=location.hash&&document.getElementById("
        "decodeURIComponent(location.hash.slice(1)));if(e&&e.tagName==='DETAILS'){"
        "e.open=true;e.scrollIntoView({block:'center'});}}"
        "abrir();addEventListener('hashchange',abrir);"
        "document.querySelectorAll('[data-acao]').forEach(function(b){"
        "b.addEventListener('click',function(){var a=b.dataset.acao==='abrir';"
        "document.querySelectorAll('details.grupo').forEach(function(d){d.open=a;});});});");
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
    copiar_estilo(PASTA_SAIDA);
    gerar_index();
    for (int c = 0; c < NUM_CAT; c++) gerar_categoria(c);

    printf("\n%d arquivo(s) lido(s). HTMLs gerados em '%s/'.\n", n_arq, PASTA_SAIDA);
    for (int c = 0; c < NUM_CAT; c++)
        printf("  %-10s %d valor(es)\n", CAT_ROTULO[c], cat[c].n);
    return 0;
}