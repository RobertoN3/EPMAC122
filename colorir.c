/* Compilar: gcc -Wall -Wextra -pedantic -std=c99 colorir.c -o colorir

   Filtro que transforma código C em HTML colorido, separando por cores os
   itens léxicos: pragmas (#include, #define, ...), tipos (int, float, ...),
   comandos (if, for, ...), comentários (de linha e de bloco) e cadeias de caracteres
   ("texto\n"). Também colore números, operadores e constantes como NULL.

   As cores são as do estilo "default" do Pygments, o realce usado nos
   exemplos de código da Wikipédia (https://en.wikipedia.org/wiki/Syntax_highlighting).

   Uso:
     colorir                         lê os .txt de entradatxt e grava, para
                                     cada TXT com código, saida/<nome>_codigo.html
     colorir pasta_entrada pasta_saida
                                     idem, com outras pastas
     colorir -f < prog.c > prog.html modo filtro: código C na entrada
                                     padrão, página HTML na saída padrão

   No TXT, o código fica entre as linhas INICIO_CODIGO_C e FIM_CODIGO_C
   (documentacao/formato_txt.md). Um TXT pode ter mais de um bloco. */
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

/* ---------- categorias léxicas e cores ---------- */

typedef struct { const char *classe, *nome, *css, *exemplo; } Categoria;

static const Categoria CATS[] = {
    {"pp", "pragma (pré-processador)",  "color:#9C6500",                     "#include"},
    {"pf", "arquivo incluído",          "color:#3D7B7B;font-style:italic",   "<stdio.h>"},
    {"tp", "tipo",                      "color:#B00040",                     "int"},
    {"kw", "comando ou palavra reservada", "color:#008000;font-weight:bold", "for"},
    {"cm", "comentário",                "color:#3D7B7B;font-style:italic",   "/* ... */"},
    {"st", "cadeia de caracteres",      "color:#BA2121",                     "\"texto\""},
    {"es", "sequência de escape",       "color:#AA5D1F;font-weight:bold",    "\\n"},
    {"nu", "número",                    "color:#666666",                     "3.14"},
    {"ct", "constante da biblioteca",   "color:#008000",                     "NULL"},
    {"op", "operador",                  "color:#666666",                     "+="},
};
#define N_CATS ((int)(sizeof CATS / sizeof CATS[0]))

static const char *const TIPOS[] = {
    "void", "char", "short", "int", "long", "float", "double", "signed",
    "unsigned", "_Bool", "_Complex", "struct", "union", "enum",
    "size_t", "ssize_t", "ptrdiff_t", "wchar_t", "FILE", "DIR", "bool",
    "clock_t", "time_t", "va_list", "intptr_t", "uintptr_t",
    "int8_t", "int16_t", "int32_t", "int64_t",
    "uint8_t", "uint16_t", "uint32_t", "uint64_t", NULL
};
static const char *const COMANDOS[] = {
    "if", "else", "for", "while", "do", "switch", "case", "default",
    "break", "continue", "return", "goto", "sizeof", NULL
};
static const char *const RESERVADAS[] = {
    "const", "static", "extern", "register", "volatile", "typedef", "auto",
    "inline", "restrict", "_Noreturn", "_Static_assert", "_Alignas",
    "_Alignof", "_Thread_local", "_Generic", NULL
};
static const char *const CONSTANTES[] = {
    "NULL", "EOF", "true", "false", "stdin", "stdout", "stderr",
    "EXIT_SUCCESS", "EXIT_FAILURE", "INT_MAX", "INT_MIN", "LLONG_MAX",
    "LLONG_MIN", "RAND_MAX", NULL
};

static const char *OPERADORES = "+-*/%=<>!&|^~?:";

/* ---------- utilidades ---------- */

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

static int identificador(const char *s) {
    if (!*s) return 0;
    for (; *s; s++)
        if (!((*s >= 'a' && *s <= 'z') || (*s >= '0' && *s <= '9') ||
              *s == '_' || *s == '-')) return 0;
    return 1;
}

static char *caminho(const char *pasta, const char *base, const char *ext) {
    size_t n = strlen(pasta) + strlen(base) + strlen(ext) + 2;
    char *s = checar(malloc(n));
    snprintf(s, n, "%s/%s%s", pasta, base, ext);
    return s;
}

/* escreve n caracteres escapando o que o HTML interpretaria */
static void esc(FILE *o, const char *s, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        switch (s[i]) {
            case '&': fputs("&amp;", o); break;
            case '<': fputs("&lt;", o);  break;
            case '>': fputs("&gt;", o);  break;
            default:  fputc((unsigned char)s[i], o);
        }
    }
}

static void span(FILE *o, const char *classe, const char *s, size_t n) {
    fprintf(o, "<span class=\"%s\">", classe);
    esc(o, s, n);
    fputs("</span>", o);
}

static int na_lista(const char *const lista[], const char *p, size_t n) {
    for (; *lista; lista++)
        if (strlen(*lista) == n && !memcmp(*lista, p, n)) return 1;
    return 0;
}

/* ---------- análise léxica ---------- */

/* comentário de bloco a partir de s[i] == '/' e s[i+1] == '*'; pode ocupar
   várias linhas. Devolve a posição logo depois do fechamento. */
static size_t comentario_bloco(FILE *o, const char *s, size_t i, size_t n) {
    const char *fim = strstr(s + i + 2, "*/");
    size_t j = fim ? (size_t)(fim - s) + 2 : n;   /* sem fechamento: até o fim */
    span(o, "cm", s + i, j - i);
    return j;
}

/* comentário de linha: de s[i] até o fim da linha (sem o '\n') */
static size_t comentario_linha(FILE *o, const char *s, size_t i, size_t n) {
    size_t j = i;
    while (j < n && s[j] != '\n') j++;
    span(o, "cm", s + i, j - i);
    return j;
}

/* cadeia "..." ou caractere '...', com as sequências de escape destacadas */
static size_t literal(FILE *o, const char *s, size_t i, size_t n) {
    char aspa = s[i];
    fputs("<span class=\"st\">", o);
    esc(o, s + i, 1);
    i++;
    while (i < n && s[i] != '\n') {
        if (s[i] == '\\' && i + 1 < n && s[i + 1] != '\n') {
            size_t j = i + 2;
            if (s[i + 1] == 'x')                      /* \x41 */
                while (j < n && isxdigit((unsigned char)s[j])) j++;
            else if (s[i + 1] >= '0' && s[i + 1] <= '7')   /* \0, \101 */
                while (j < n && j < i + 4 && s[j] >= '0' && s[j] <= '7') j++;
            span(o, "es", s + i, j - i);
            i = j;
            continue;
        }
        esc(o, s + i, 1);
        if (s[i++] == aspa) break;
    }
    fputs("</span>", o);
    return i;
}

/* número: inteiro, real, hexadecimal, com expoente e sufixos (10L, 7.5f) */
static size_t fim_numero(const char *s, size_t i, size_t n) {
    int hex = s[i] == '0' && i + 1 < n && (s[i + 1] == 'x' || s[i + 1] == 'X');
    size_t j = i;
    while (j < n) {
        char c = s[j];
        if (isalnum((unsigned char)c) || c == '.') j++;
        else if ((c == '+' || c == '-') && j > i &&
                 (hex ? (s[j - 1] == 'p' || s[j - 1] == 'P')
                      : (s[j - 1] == 'e' || s[j - 1] == 'E'))) j++;
        else break;
    }
    return j;
}

/* diretiva do pré-processador: de '#' até o fim da linha lógica (uma linha
   terminada em '\' continua na seguinte). O nome da diretiva e o resto da
   linha ficam com a cor de pragma; o arquivo do #include e os comentários,
   com as cores próprias. */
static size_t diretiva(FILE *o, const char *s, size_t i, size_t n) {
    size_t j = i + 1, k;
    int include;

    while (j < n && (s[j] == ' ' || s[j] == '\t')) j++;
    k = j;
    while (j < n && (isalnum((unsigned char)s[j]) || s[j] == '_')) j++;
    include = j - k == 7 && !memcmp(s + k, "include", 7);
    span(o, "pp", s + i, j - i);
    i = j;

    if (include) {
        while (i < n && (s[i] == ' ' || s[i] == '\t')) fputc(s[i++], o);
        if (i < n && (s[i] == '<' || s[i] == '"')) {
            char fecha = s[i] == '<' ? '>' : '"';
            j = i + 1;
            while (j < n && s[j] != fecha && s[j] != '\n') j++;
            if (j < n && s[j] == fecha) j++;
            span(o, "pf", s + i, j - i);
            i = j;
        }
    }

    fputs("<span class=\"pp\">", o);
    while (i < n) {
        if (s[i] == '\n') {
            if (i > 0 && s[i - 1] == '\\') { fputc('\n', o); i++; continue; }
            break;
        }
        if (s[i] == '/' && i + 1 < n && s[i + 1] == '/') {
            i = comentario_linha(o, s, i, n);
            break;
        }
        if (s[i] == '/' && i + 1 < n && s[i + 1] == '*') {
            i = comentario_bloco(o, s, i, n);
            continue;
        }
        esc(o, s + i, 1);
        i++;
    }
    fputs("</span>", o);
    return i;
}

/* colore o código s inteiro */
static void colorir(FILE *o, const char *s) {
    size_t i = 0, n = strlen(s);
    int inicio_linha = 1;          /* só espaços desde o último '\n' */

    while (i < n) {
        char c = s[i];
        size_t j;

        if (c == '\n') { fputc('\n', o); i++; inicio_linha = 1; continue; }
        if (c == ' ' || c == '\t') { fputc(c, o); i++; continue; }

        if (c == '#' && inicio_linha) {
            i = diretiva(o, s, i, n);
            continue;
        }
        inicio_linha = 0;

        if (c == '/' && i + 1 < n && s[i + 1] == '/') {
            i = comentario_linha(o, s, i, n);
        } else if (c == '/' && i + 1 < n && s[i + 1] == '*') {
            i = comentario_bloco(o, s, i, n);
        } else if (c == '"' || c == '\'') {
            i = literal(o, s, i, n);
        } else if (isdigit((unsigned char)c) ||
                   (c == '.' && i + 1 < n && isdigit((unsigned char)s[i + 1]))) {
            j = fim_numero(s, i, n);
            span(o, "nu", s + i, j - i);
            i = j;
        } else if (isalpha((unsigned char)c) || c == '_') {
            const char *classe = NULL;
            j = i;
            while (j < n && (isalnum((unsigned char)s[j]) || s[j] == '_')) j++;
            if (na_lista(TIPOS, s + i, j - i))            classe = "tp";
            else if (na_lista(COMANDOS, s + i, j - i) ||
                     na_lista(RESERVADAS, s + i, j - i))  classe = "kw";
            else if (na_lista(CONSTANTES, s + i, j - i))  classe = "ct";
            if (classe) span(o, classe, s + i, j - i);
            else esc(o, s + i, j - i);
            i = j;
        } else if (strchr(OPERADORES, c)) {
            j = i;
            while (j < n && strchr(OPERADORES, s[j]) &&
                   !(s[j] == '/' && j + 1 < n && (s[j + 1] == '/' || s[j + 1] == '*')))
                j++;
            if (j == i) j++;     /* nunca deveria acontecer: avança mesmo assim */
            span(o, "op", s + i, j - i);
            i = j;
        } else {                 /* pontuação: ( ) [ ] { } ; , . */
            esc(o, s + i, 1);
            i++;
        }
    }
}

/* ---------- página HTML ---------- */

/* tira linhas em branco do fim e devolve o número de linhas */
static int preparar(Texto *t) {
    int linhas = 1;
    size_t i;
    while (t->n && (t->s[t->n - 1] == '\n' || t->s[t->n - 1] == ' ' ||
                    t->s[t->n - 1] == '\t' || t->s[t->n - 1] == '\r'))
        t->s[--t->n] = '\0';
    for (i = 0; i < t->n; i++) if (t->s[i] == '\n') linhas++;
    return linhas;
}

static void bloco(FILE *o, Texto *t) {
    int i, linhas;
    if (!t->s) anexar(t, "", 0);
    linhas = preparar(t);
    fputs("<div class=\"codigo\"><pre class=\"nl\" aria-hidden=\"true\">", o);
    for (i = 1; i <= linhas; i++) fprintf(o, "%d%s", i, i < linhas ? "\n" : "");
    fputs("</pre><pre class=\"c\"><code>", o);
    colorir(o, t->s);
    fputs("</code></pre></div>\n", o);
}

/* base == NULL no modo filtro: sem links para as outras páginas */
static void pagina(FILE *o, const char *titulo, const char *base, Texto *blocos, int nb) {
    int c, b;
    fputs("<!DOCTYPE html>\n<html lang=\"pt-BR\"><head><meta charset=\"utf-8\">\n"
          "<meta name=\"viewport\" content=\"width=device-width,initial-scale=1\">\n<title>Código: ", o);
    esc(o, titulo, strlen(titulo));
    fputs("</title>\n<style>"
          "body{font-family:system-ui,sans-serif;max-width:960px;margin:40px auto;"
          "padding:0 16px;color:#222;line-height:1.6}"
          "a{color:#2b6cb0}nav a{margin-right:20px}"
          ".codigo{display:flex;background:#f8f8f8;border:1px solid #ddd;"
          "border-radius:8px;overflow-x:auto;margin:16px 0}"
          ".codigo pre{margin:0;padding:12px 14px;font:14px/1.45 ui-monospace,"
          "SFMono-Regular,Consolas,monospace;tab-size:4}"
          ".nl{color:#999;text-align:right;border-right:1px solid #ddd;user-select:none}"
          ".legenda{list-style:none;padding:0;display:flex;flex-wrap:wrap;"
          "gap:4px 18px;font-size:.9rem;color:#555}"
          ".legenda code{font:13px ui-monospace,Consolas,monospace;background:#f8f8f8;"
          "border:1px solid #e5e5e5;padding:0 5px;border-radius:4px}", o);
    for (c = 0; c < N_CATS; c++) fprintf(o, ".%s{%s}", CATS[c].classe, CATS[c].css);
    fputs("</style></head><body>\n", o);

    if (base) {
        fputs("<nav><a href=\"index.html\">&larr; Início</a>", o);
        fprintf(o, "<a href=\"%s.html\">&larr; Explicação: ", base);
        esc(o, titulo, strlen(titulo));
        fputs("</a></nav>\n", o);
    }
    fputs("<main><h1>Código em C: ", o);
    esc(o, titulo, strlen(titulo));
    fputs("</h1>\n<ul class=\"legenda\">\n", o);
    for (c = 0; c < N_CATS; c++) {
        fprintf(o, "<li><code class=\"%s\">", CATS[c].classe);
        esc(o, CATS[c].exemplo, strlen(CATS[c].exemplo));
        fprintf(o, "</code> %s</li>\n", CATS[c].nome);
    }
    fputs("</ul>\n", o);

    for (b = 0; b < nb; b++) {
        if (nb > 1) fprintf(o, "<h2>Programa %d</h2>\n", b + 1);
        bloco(o, &blocos[b]);
    }
    fputs("</main></body></html>\n", o);
}

/* ---------- modo filtro ---------- */

static int filtro(void) {
    Texto t = {0};
    char buf[4096];
    size_t n, i, k = 0;
    while ((n = fread(buf, 1, sizeof buf, stdin)) > 0) anexar(&t, buf, n);
    if (ferror(stdin)) { perror("entrada padrão"); return EXIT_FAILURE; }
    for (i = 0; i < t.n; i++)                  /* remove '\r' (Windows) */
        if (t.s[i] != '\r') t.s[k++] = t.s[i];
    if (t.s) { t.n = k; t.s[k] = '\0'; }
    pagina(stdout, "programa", NULL, &t, 1);
    free(t.s);
    return ferror(stdout) ? EXIT_FAILURE : EXIT_SUCCESS;
}

/* ---------- modo TXT ---------- */

/* devolve 1 se gerou a página, 0 se o TXT não tem código, -1 em erro */
static int gerar(const char *base) {
    char *in = caminho(entrada, base, ".txt"), *out, *linha, *titulo = NULL;
    FILE *f = fopen(in, "r"), *o;
    Texto *blocos = NULL;
    int nb = 0, dentro = 0, cabecalho = 1, primeira = 1, ok = 1, b;

    if (!f) { perror(in); free(in); return -1; }
    while ((linha = ler_linha(f))) {
        char *s = linha;
        if (primeira && !strncmp(s, "\xef\xbb\xbf", 3)) s += 3;   /* BOM */
        primeira = 0;

        if (cabecalho) {                       /* só o Titulo interessa aqui */
            char *t = trim(s), *fim;
            long num = strtol(t, &fim, 10);
            if (!*t || fim == t || *fim != '.' || !(fim = strchr(fim, ':'))) {
                cabecalho = 0;
            } else {
                if (num == 7) {
                    free(titulo);
                    titulo = checar(malloc(strlen(fim + 1) + 1));
                    strcpy(titulo, trim(fim + 1));
                }
                free(linha);
                continue;
            }
        }

        if (dentro) {
            if (eh_marca(s, "FIM_CODIGO_C") || eh_marca(s, "[/codigo]")) dentro = 0;
            else { anexar(&blocos[nb - 1], s, strlen(s)); anexar(&blocos[nb - 1], "\n", 1); }
        } else if (eh_marca(s, "INICIO_CODIGO_C") || eh_marca(s, "[codigo]")) {
            blocos = checar(realloc(blocos, (size_t)(nb + 1) * sizeof *blocos));
            memset(&blocos[nb++], 0, sizeof *blocos);
            dentro = 1;
        }
        free(linha);
    }
    fclose(f);

    if (dentro) {
        fprintf(stderr, "%s: bloco de código sem FIM_CODIGO_C\n", in);
        ok = 0;
    }
    if (ok && nb > 0) {
        out = caminho(saida, base, "_codigo.html");
        o = fopen(out, "w");
        if (!o) { perror(out); ok = 0; }
        else {
            pagina(o, titulo && *titulo ? titulo : base, base, blocos, nb);
            if (ferror(o)) ok = 0;
            if (fclose(o)) ok = 0;
            if (!ok) fprintf(stderr, "%s: erro de escrita\n", out);
        }
        free(out);
    }

    for (b = 0; b < nb; b++) free(blocos[b].s);
    free(blocos);
    free(titulo);
    free(in);
    if (!ok) return -1;
    return nb > 0;
}

int main(int argc, char **argv) {
    DIR *d;
    struct dirent *e;
    int total = 0, sem_codigo = 0;

    if (argc == 2 && !strcmp(argv[1], "-f")) return filtro();
    if (argc != 1 && argc != 3) {
        fprintf(stderr, "Uso: %s [pasta_entrada pasta_saida]\n"
                        "     %s -f < programa.c > programa.html\n", argv[0], argv[0]);
        return EXIT_FAILURE;
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
        int r;
        if (n < 5 || strcmp(e->d_name + n - 4, ".txt")) continue;
        base = checar(malloc(n - 3));
        memcpy(base, e->d_name, n - 4);
        base[n - 4] = '\0';
        if (!identificador(base)) {
            fprintf(stderr, "Nome inválido: %s\n", e->d_name);
            erros++;
        } else if ((r = gerar(base)) > 0) total++;
        else if (r == 0) sem_codigo++;
        else erros++;
        free(base);
    }
    closedir(d);

    printf("%d página(s) de código gerada(s) em '%s'", total, saida);
    if (sem_codigo) printf("; %d TXT sem código C", sem_codigo);
    printf(".\n");
    return erros ? EXIT_FAILURE : EXIT_SUCCESS;
}
