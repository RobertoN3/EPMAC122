/* cabecalho.h: reconhece as linhas do cabeçalho dos TXT, comum ao gerador.c,
   ao gerador_conteudo.c e ao txt.h (colorir.c e exportar.c).

   Aceita os dois jeitos de escrever um campo:
       1. Tecnica: Método guloso        (numerado, como nos TXT do EP)
       Tecnica: Método guloso           (sem número, como no enunciado)
   O campo é reconhecido pelo NOME, sem diferenciar maiúsculas nem acentos
   ("Técnica", "TECNICA" e "tecnica" são o mesmo campo; "Autor" e
   "Conceito" também valem). O número só decide quando o nome não é
   nenhum dos conhecidos. */
#ifndef CABECALHO_H
#define CABECALHO_H

#include <string.h>
#include <ctype.h>

#define CAMPO_TECNICA    1
#define CAMPO_ALGORITMO  2
#define CAMPO_AUTORES    3
#define CAMPO_DATA       4
#define CAMPO_CONCEITOS  5
#define CAMPO_PROBLEMA   6
#define CAMPO_TITULO     7

/* letra sem acento e minúscula; *p avança 1 ou 2 bytes (UTF-8 de Latin-1) */
static int letra_simples(const unsigned char **p) {
    const unsigned char *s = *p;
    static const char MAPA[] =      /* bytes 0x80..0xBF depois de 0xC3 */
        "aaaaaaaceeeeiiiidnooooo/ouuuuyps"     /* 0x80..0x9F (maiúsculas) */
        "aaaaaaaceeeeiiiidnooooo/ouuuuypy";    /* 0xA0..0xBF (minúsculas) */
    if (s[0] == 0xC3 && s[1] >= 0x80 && s[1] <= 0xBF) {
        *p += 2;
        return MAPA[s[1] - 0x80];
    }
    *p += 1;
    return tolower(s[0]);
}

/* compara o rótulo (sem acento, sem espaços, minúsculo) com um nome ASCII */
static int rotulo_igual(const char *ini, const char *fim, const char *nome) {
    const unsigned char *p = (const unsigned char *)ini;
    while (p < (const unsigned char *)fim) {
        int c;
        if (isspace(*p)) { p++; continue; }
        c = letra_simples(&p);
        if (c != *nome) return 0;
        nome++;
    }
    return *nome == '\0';
}

static int campo_pelo_nome(const char *ini, const char *fim) {
    static const struct { const char *nome; int campo; } NOMES[] = {
        {"tecnica", CAMPO_TECNICA},     {"tecnicas", CAMPO_TECNICA},
        {"algoritmo", CAMPO_ALGORITMO}, {"algoritmos", CAMPO_ALGORITMO},
        {"autores", CAMPO_AUTORES},     {"autor", CAMPO_AUTORES},
        {"data", CAMPO_DATA},           {"datas", CAMPO_DATA},
        {"conceitos", CAMPO_CONCEITOS}, {"conceito", CAMPO_CONCEITOS},
        {"problema", CAMPO_PROBLEMA},   {"problemas", CAMPO_PROBLEMA},
        {"titulo", CAMPO_TITULO}
    };
    size_t k;
    for (k = 0; k < sizeof NOMES / sizeof NOMES[0]; k++)
        if (rotulo_igual(ini, fim, NOMES[k].nome)) return NOMES[k].campo;
    return 0;
}

/* Se a linha (já sem espaços nas pontas) é um campo do cabeçalho, devolve o
   número do campo (1 a 7) e aponta *valor para o texto depois do ':'.
   Devolve 0 se a linha não é campo do cabeçalho (fim do cabeçalho). */
static int campo_cabecalho(const char *linha, const char **valor) {
    const char *s = linha, *rot, *dp;
    int numero = 0, campo;

    if (isdigit((unsigned char)*s)) {           /* "1." opcional */
        while (isdigit((unsigned char)*s)) numero = numero * 10 + (*s++ - '0');
        if (*s != '.') return 0;
        s++;
    }
    while (*s == ' ' || *s == '\t') s++;
    rot = s;
    dp = strchr(s, ':');
    if (!dp || dp == rot) return 0;

    campo = campo_pelo_nome(rot, dp);
    if (!campo) {
        if (numero < 1 || numero > CAMPO_TITULO) return 0;   /* nome e número desconhecidos */
        campo = numero;                         /* nome estranho: vale o número */
    }
    *valor = dp + 1;
    while (**valor == ' ' || **valor == '\t') (*valor)++;
    return campo;
}

#endif
