# Formato dos arquivos TXT

Regras que os 38 TXT de `entradatxt/` seguem e que o gerador deve implementar.

## Estrutura do arquivo

```
1. Tecnica: Método guloso
2. Algoritmo: Kruskal para árvore espalhada mínima
3. Autores: Joseph Kruskal
4. Data: 1956
5. Conceitos: Grafos, Árvores, Árvore espalhada mínima, Conjuntos disjuntos, Ordenação
6. Problema: Dado um [[grafos|grafo]] não dirigido e conexo com pesos nas arestas, determinar sua árvore espalhada mínima
7. Titulo: Algoritmo de Kruskal

## Primeira seção
Texto...

REFERENCIAS:
- KRUSKAL, J. B. ...

INICIO_CODIGO_C
#include <stdio.h>
...
FIM_CODIGO_C
```

- Codificação UTF-8. O nome do arquivo (minúsculas, sem acento, sem espaço) é o identificador da página: `kruskal.txt` gera `kruskal.html`.
- Para escrever estas regras usamos `` ``` `` dentro deste documento, mas nos TXT ele também marca bloco pré-formatado (veja abaixo).

## Cabeçalho

- Linhas `N. Rotulo: valor`, com N de 1 a 7, na ordem. Cada campo ocupa uma única linha.
- O cabeçalho termina na primeira linha em branco.
- Campo vazio ou desconhecido: a linha é omitida. Não usar `?`, "Desconhecido", "Tradição".
- Tecnica, Autores, Data e Conceitos aceitam vários valores separados por vírgula. O gerador separa na vírgula e remove os espaços das pontas (`trim`).
- **Tecnica**: só as quatro do enunciado: Método guloso, Divisão e conquista, Tentativa e erro, Programação dinâmica. As páginas de técnica (`metodo_guloso`, `divisao_conquista`, `tentativa_erro`, `programacao_dinamica`) se listam na própria técnica.
- **Autores e Data** se referem ao que está em Algoritmo, ou ao Titulo quando Algoritmo está vazio. No índice: "Joseph Kruskal (Kruskal para árvore espalhada mínima)" e "1956: Kruskal para árvore espalhada mínima".
- **Problema** começa com "Dado/Dada/Dados" e existe só onde há um problema concreto. Páginas de técnica e de fundamento (eficiência, ponteiros, registros) não têm Problema.
- **Titulo** é obrigatório. Ele dá nome à página e aos links nos índices.

## Vocabulário fechado de Conceitos

Só estes nomes, escritos exatamente assim. A coluna da direita é a página que explica o conceito. Ela serve para o índice de Conceitos ligar o nome à explicação, além de listar as páginas que usam o conceito.

| Conceito | Página explicativa | Conceito | Página explicativa |
|---|---|---|---|
| Árvores | arvores | Hash | hash |
| Árvore espalhada mínima | kruskal | Invariante | mdc_euclides |
| Busca binária | busca_binaria | Listas encadeadas | listas_encadeadas |
| Caminho mínimo | dijkstra | Memorização | programacao_dinamica |
| Conjuntos disjuntos | kruskal | Ordenação | ordenacao |
| Eficiência de algoritmos | eficiencia | Pilhas | pilhas |
| Estabilidade | estabilidade | Ponteiros | ponteiros |
| Filas | filas | Registros | registros |
| Fila de prioridade | heapsort | Recursão | recursao |
| Grafos | grafos | Subestrutura ótima | programacao_dinamica |
| Vetores | (sem página) | Matrizes | imagens_pbm |
| Busca sequencial | busca_sequencial | Indução | inducao |
| Representação de conjuntos | conjuntos | Autômatos finitos | automatos |

## Corpo

O texto vem quebrado em linhas de ~75 colunas, então o gerador **não** pode transformar cada quebra em `<br>`. Regras, como máquina de estados:

| No TXT | No HTML |
|---|---|
| `## Título` | `<h2>Título</h2>` |
| linha em branco | fecha o parágrafo, a lista ou o item atual |
| linhas comuns seguidas | juntam-se num só `<p>` |
| linha começando com `- ` | novo `<li>` de uma `<ul>` |
| linha começando com `1) `, `2) `... | novo `<li>` de uma `<ol>` |
| linha recuada logo depois de um item | continuação desse `<li>` (inclusive sub-itens `- ` recuados) |
| linha `` ``` `` | abre/fecha `<pre>`; dentro, as linhas saem como estão |
| `[[pagina]]` / `[[pagina\|texto]]` | `<a href="pagina.html">pagina</a>` / `<a href="pagina.html">texto</a>` |

- Todo `[[alvo]]` precisa de `alvo.txt`. Os links também aparecem no campo Problema.
- O gerador escapa `<`, `>` e `&` em todo o texto e no código (`m < n²`, `#include <stdio.h>`).

## Referências e código

- `REFERENCIAS:` sozinha numa linha abre a seção; cada referência é uma linha `- ...`. Fica na mesma página.
- O código C fica entre as linhas `INICIO_CODIGO_C` e `FIM_CODIGO_C`, depois das referências. Ele vai para uma página HTML separada e colorida. Um TXT pode ter mais de um bloco.
- Todo código compila sem avisos com `gcc -Wall -Wextra -pedantic -std=c99` e roda.

## Páginas (38)

- **Técnicas:** metodo_guloso, divisao_conquista, tentativa_erro, programacao_dinamica
- **Fundamentos e estruturas:** eficiencia, inducao, recursao, mdc_euclides, ponteiros, registros, listas_encadeadas, pilhas, filas, conjuntos, hash, arvores, algoritmos_arvores, abb, grafos, imagens_pbm, automatos
- **Ordenação:** ordenacao, estabilidade, bubblesort, selecao_direta, insertionsort, mergesort, quicksort, heapsort, ordenacao_contagem, radixsort
- **Busca e grafos:** busca_sequencial, busca_binaria, bfs, ordenacao_topologica, dijkstra, kruskal, prim
