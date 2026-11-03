/* indice.js: interação das páginas de índice do site do EP de MAC0122.

   O gerador copia este arquivo para a pasta de saída. Ele faz três coisas:
   1. anima a abertura e o fechamento dos grupos (<details class="grupo">);
   2. abre o grupo indicado na âncora do endereço (index.html#item), que é
      para onde apontam as etiquetas da ficha de cada página;
   3. faz a busca dos índices tolerar erros de digitação (busca aproximada,
      com distância de edição), descartando o que estiver longe demais. */
(function () {
  'use strict';

  var reduzir = window.matchMedia && matchMedia('(prefers-reduced-motion: reduce)').matches;

  /* ---------- 1. abrir e fechar com animação ---------- */

  function alternar(d, abrir, animar) {
    var corpo = d.querySelector('.corpo');
    if (d._anim) { d._anim.cancel(); d._anim = null; }
    d.classList.remove('fechando');
    if (!corpo || reduzir || !animar || !corpo.animate) { d.open = abrir; return; }

    if (abrir) {
      d.open = true;
      var h = corpo.scrollHeight;
      d._anim = corpo.animate(
        [{ height: '0px', opacity: 0 }, { height: h + 'px', opacity: 1 }],
        { duration: 280, easing: 'cubic-bezier(.2, .7, .2, 1)' });
      d._anim.onfinish = function () { d._anim = null; };
    } else {
      var atual = corpo.offsetHeight;
      d.classList.add('fechando');                 /* a seta já gira de volta */
      d._anim = corpo.animate(
        [{ height: atual + 'px', opacity: 1 }, { height: '0px', opacity: 0 }],
        { duration: 220, easing: 'cubic-bezier(.4, 0, .6, 1)' });
      d._anim.onfinish = function () {
        d._anim = null; d.open = false; d.classList.remove('fechando');
      };
    }
  }

  var grupos = Array.prototype.slice.call(document.querySelectorAll('details.grupo'));
  grupos.forEach(function (d) {
    d.querySelector('summary').addEventListener('click', function (e) {
      if (e.target.closest('a')) return;           /* link dentro do título (Problema) */
      e.preventDefault();
      alternar(d, !d.open || d.classList.contains('fechando'), true);
    });
  });

  Array.prototype.forEach.call(document.querySelectorAll('[data-acao]'), function (b) {
    b.addEventListener('click', function () {
      var abrir = b.dataset.acao === 'abrir';
      grupos.forEach(function (d) { if (!d.hidden) alternar(d, abrir, true); });
    });
  });

  /* ---------- 2. âncora no endereço ---------- */

  function irParaAncora() {
    if (!location.hash) return;
    var e = document.getElementById(decodeURIComponent(location.hash.slice(1)));
    if (!e) return;
    if (e.tagName === 'DETAILS') alternar(e, true, false);
    e.scrollIntoView({ block: 'center' });
  }
  irParaAncora();
  addEventListener('hashchange', irParaAncora);

  /* ---------- 3. busca aproximada ---------- */

  var campo = document.getElementById('busca');
  var lista = document.querySelector('.grupos');
  if (!campo || !lista) return;

  var LIMIAR = 0.7;          /* abaixo disso a palavra está "longe demais" */
  var info = document.querySelector('.busca-info');
  var vazio = document.querySelector('.busca-vazia');

  /* minúsculas e sem acento: "Árvore" e "arvore" são a mesma palavra */
  function normalizar(s) {
    return s.normalize('NFD').replace(/[̀-ͯ]/g, '').toLowerCase();
  }
  function palavras(s) {
    return normalizar(s).split(/[^a-z0-9]+/).filter(Boolean);
  }

  /* distância de edição com transposição (Damerau-Levenshtein restrita):
     quantas inserções, remoções, trocas ou inversões de letras vizinhas
     transformam a em b */
  function distancia(a, b) {
    var m = a.length, n = b.length, d = [], i, j;
    for (i = 0; i <= m; i++) { d[i] = [i]; }
    for (j = 0; j <= n; j++) { d[0][j] = j; }
    for (i = 1; i <= m; i++) {
      for (j = 1; j <= n; j++) {
        var custo = a[i - 1] === b[j - 1] ? 0 : 1;
        d[i][j] = Math.min(d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + custo);
        if (i > 1 && j > 1 && a[i - 1] === b[j - 2] && a[i - 2] === b[j - 1])
          d[i][j] = Math.min(d[i][j], d[i - 2][j - 2] + 1);
      }
    }
    return d[m][n];
  }

  /* quão parecido o termo digitado é com uma palavra do texto (0 a 1).
     Compara com a palavra inteira e com o começo dela, para que "dijks"
     já encontre "dijkstra" enquanto a pessoa digita. */
  function semelhanca(termo, palavra) {
    if (palavra.indexOf(termo) === 0) return 1;
    if (palavra.indexOf(termo) > 0) return 0.9;
    if (termo.length < 4) return 0;                /* termos curtos: só exatos */
    var melhor = distancia(termo, palavra);
    for (var k = termo.length - 1; k <= termo.length + 1; k++)
      if (k > 0 && k < palavra.length) melhor = Math.min(melhor, distancia(termo, palavra.slice(0, k)));
    return 1 - melhor / termo.length;
  }

  /* nota de um texto para a busca: todos os termos precisam aparecer (com
     semelhança >= LIMIAR); a nota é a média. 0 = não serve. */
  function nota(termos, ps) {
    var soma = 0;
    for (var t = 0; t < termos.length; t++) {
      var melhor = 0;
      for (var p = 0; p < ps.length && melhor < 1; p++)
        melhor = Math.max(melhor, semelhanca(termos[t], ps[p]));
      if (melhor < LIMIAR) return 0;
      soma += melhor;
    }
    return soma / termos.length;
  }

  /* cada item guarda as palavras do seu nome e das páginas que contém */
  var itens = Array.prototype.map.call(lista.children, function (el, i) {
    var valor = el.querySelector('.valor');
    var paginas = Array.prototype.map.call(el.querySelectorAll('.paginas li, .destino, .exp a'),
      function (li) { return { el: li, ps: palavras(li.textContent) }; });
    return { el: el, ordem: i, ps: palavras(valor ? valor.textContent : el.textContent), paginas: paginas };
  });

  function limpar() {
    itens.forEach(function (it) {
      it.el.hidden = false;
      it.paginas.forEach(function (p) { p.el.classList.remove('achado'); });
      if (it.abertoPelaBusca) { it.el.open = false; it.abertoPelaBusca = false; }
      lista.appendChild(it.el);                    /* volta à ordem alfabética */
    });
  }

  function buscar() {
    var termos = palavras(campo.value);
    limpar();
    if (!termos.length) { info.textContent = ''; vazio.hidden = true; return; }

    var achados = [];
    itens.forEach(function (it) {
      var n = nota(termos, it.ps), pelaPagina = false;
      it.paginas.forEach(function (p) {
        var np = nota(termos, p.ps) * 0.95;        /* o nome do item vale um pouco mais */
        if (np > 0) {
          if (p.el.tagName === 'LI') p.el.classList.add('achado');
          if (np > n) { n = np; pelaPagina = true; }
        }
      });
      if (n > 0) {
        achados.push({ it: it, n: n });
        if (pelaPagina && it.el.tagName === 'DETAILS' && !it.el.open) {
          it.el.open = true; it.abertoPelaBusca = true;
        }
      } else {
        it.el.hidden = true;
      }
    });

    achados.sort(function (a, b) { return b.n - a.n || a.it.ordem - b.it.ordem; });
    achados.forEach(function (a) { lista.appendChild(a.it.el); });
    itens.forEach(function (it) { if (it.el.hidden) lista.appendChild(it.el); });

    info.textContent = achados.length === 1 ? '1 resultado' : achados.length + ' resultados';
    vazio.hidden = achados.length > 0;
  }

  campo.addEventListener('input', buscar);
  campo.addEventListener('keydown', function (e) {
    if (e.key === 'Escape') { campo.value = ''; buscar(); }
    if (e.key === 'Enter') {                       /* Enter abre o primeiro resultado */
      var primeiro = lista.querySelector(':scope > :not([hidden])');
      if (!primeiro) return;
      var link = primeiro.tagName === 'A' ? primeiro
        : primeiro.querySelector('.paginas li.achado a') || primeiro.querySelector('.paginas a');
      if (link) location.href = link.href;
    }
  });
  if (campo.value) buscar();                       /* o navegador pode ter guardado o texto */
})();
