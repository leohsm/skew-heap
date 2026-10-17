/*
 * teste.c - Testes automatizados da biblioteca Skew Heap
 */
#include <stdio.h>
#include <stdlib.h>

#include "skew_heap.h"

static int falhas = 0;
static int verificacoes = 0;

#define CHECAR(cond)                                                     \
    do {                                                                 \
        verificacoes++;                                                  \
        if (!(cond)) {                                                   \
            falhas++;                                                    \
            fprintf(stderr, "FALHOU %s:%d: %s\n", __FILE__, __LINE__, #cond); \
        }                                                                \
    } while (0)

/* Gerador pseudoaleatorio simples e deterministico (LCG). */
static unsigned long semente = 12345UL;
static int aleatorio(int max)
{
    semente = semente * 1103515245UL + 12345UL;
    return (int)((semente >> 16) % (unsigned long)max);
}

static int cmp_int(const void *a, const void *b)
{
    int x = *(const int *)a, y = *(const int *)b;
    return (x > y) - (x < y);
}

/* Compara a forma e as chaves de duas arvores. */
static int mesma_arvore(const SkewNo *a, const SkewNo *b)
{
    if (a == NULL || b == NULL) return a == b;
    return a->chave == b->chave && mesma_arvore(a->esq, b->esq) &&
           mesma_arvore(a->dir, b->dir);
}

static SkewNo *copiar(const SkewNo *no)
{
    SkewNo *c;
    if (no == NULL) return NULL;
    c = skew_novo_no(no->chave);
    c->esq = copiar(no->esq);
    c->dir = copiar(no->dir);
    return c;
}

static void liberar(SkewNo *no)
{
    if (no == NULL) return;
    liberar(no->esq);
    liberar(no->dir);
    free(no);
}

/* ---------------------------------------------------------------------- */

static void teste_heap_vazia(void)
{
    SkewHeap *h = skew_criar();
    int x = -1;
    size_t v = 99;

    CHECAR(h != NULL);
    CHECAR(skew_vazia(h));
    CHECAR(skew_tamanho(h) == 0);
    CHECAR(!skew_minimo(h, &x));
    CHECAR(!skew_remover_minimo(h, &x));
    CHECAR(!skew_buscar(h, 1, &v));
    CHECAR(v == 0);
    CHECAR(!skew_remover(h, 1));
    CHECAR(skew_valida(h));
    CHECAR(skew_altura(h) == 0);
    skew_destruir(h);
    skew_destruir(NULL); /* nao deve quebrar */
}

/* Exemplos do relatorio (Figuras 3, 4 e 5). */
static void teste_exemplos_relatorio(void)
{
    SkewHeap *h = skew_criar();
    int x;
    size_t v;

    skew_inserir(h, 20);
    CHECAR(h->raiz->chave == 20);

    skew_inserir(h, 10);
    CHECAR(h->raiz->chave == 10);
    CHECAR(h->raiz->esq && h->raiz->esq->chave == 20);
    CHECAR(h->raiz->dir == NULL);

    skew_inserir(h, 30);
    CHECAR(h->raiz->chave == 10);
    CHECAR(h->raiz->esq && h->raiz->esq->chave == 30);
    CHECAR(h->raiz->dir && h->raiz->dir->chave == 20);

    CHECAR(skew_remover_minimo(h, &x) && x == 10);
    CHECAR(h->raiz->chave == 20);
    CHECAR(h->raiz->esq && h->raiz->esq->chave == 30);
    CHECAR(h->raiz->dir == NULL);
    skew_limpar(h);

    /* Arvore da busca: 10 (E: 30, D: 20 (E: 25)) */
    h->raiz = skew_novo_no(10);
    h->raiz->esq = skew_novo_no(30);
    h->raiz->dir = skew_novo_no(20);
    h->raiz->dir->esq = skew_novo_no(25);
    h->tamanho = 4;
    CHECAR(skew_valida(h));
    CHECAR(skew_buscar(h, 25, &v));
    CHECAR(v == 4); /* 10, 30 (podado), 20, 25 */
    CHECAR(!skew_buscar(h, 15, &v));
    CHECAR(v == 3); /* 10, 30 (podado), 20 (podado) */
    CHECAR(!skew_buscar(h, 5, &v));
    CHECAR(v == 1); /* raiz ja e maior */

    skew_destruir(h);
}

/* A versao iterativa do merge deve gerar exatamente a mesma arvore. */
static void teste_merge_iterativo_igual(void)
{
    int rodada;
    for (rodada = 0; rodada < 200; rodada++) {
        SkewHeap *a = skew_criar(), *b = skew_criar();
        int i, na = aleatorio(60), nb = aleatorio(60);
        SkewNo *ca, *cb, *r1, *r2;

        for (i = 0; i < na; i++) skew_inserir(a, aleatorio(100));
        for (i = 0; i < nb; i++) skew_inserir(b, aleatorio(100));

        ca = copiar(a->raiz);
        cb = copiar(b->raiz);
        r1 = skew_merge(a->raiz, b->raiz);
        r2 = skew_merge_iterativo(ca, cb);
        CHECAR(mesma_arvore(r1, r2));

        liberar(r1);
        liberar(r2);
        a->raiz = b->raiz = NULL;
        a->tamanho = b->tamanho = 0;
        skew_destruir(a);
        skew_destruir(b);
    }
}

/* Operacoes aleatorias comparadas com um vetor de referencia. */
static void teste_aleatorio(void)
{
    enum { N = 4000 };
    static int ref[N];
    int nref = 0;
    int passo;
    SkewHeap *h = skew_criar();

    for (passo = 0; passo < 20000; passo++) {
        int op = aleatorio(10);
        int x;

        if (op < 5 && nref < N) { /* inserir */
            int v = aleatorio(1000) - 500;
            CHECAR(skew_inserir(h, v));
            ref[nref++] = v;
        } else if (op < 8) { /* remover minimo */
            int i, imin = 0;
            if (nref == 0) {
                CHECAR(!skew_remover_minimo(h, &x));
                continue;
            }
            for (i = 1; i < nref; i++)
                if (ref[i] < ref[imin]) imin = i;
            CHECAR(skew_remover_minimo(h, &x));
            CHECAR(x == ref[imin]);
            ref[imin] = ref[--nref];
        } else if (op < 9) { /* buscar */
            int v = aleatorio(1000) - 500, i, existe = 0;
            for (i = 0; i < nref; i++)
                if (ref[i] == v) existe = 1;
            CHECAR(skew_buscar(h, v, NULL) == (existe != 0));
        } else { /* remover chave qualquer */
            int v = aleatorio(1000) - 500, i, pos = -1;
            for (i = 0; i < nref; i++)
                if (ref[i] == v) { pos = i; break; }
            CHECAR(skew_remover(h, v) == (pos >= 0));
            if (pos >= 0) ref[pos] = ref[--nref];
        }

        if (passo % 500 == 0) CHECAR(skew_valida(h));
        CHECAR(skew_tamanho(h) == (size_t)nref);
    }
    CHECAR(skew_valida(h));
    skew_destruir(h);
}

/* Ordenacao (heapsort) e fusao. */
static void teste_ordenacao_e_fusao(void)
{
    enum { N = 5000 };
    static int vals[N];
    SkewHeap *a = skew_criar(), *b = skew_criar();
    int i, x, ok = 1;

    for (i = 0; i < N; i++) {
        vals[i] = aleatorio(100000);
        skew_inserir(i % 2 ? a : b, vals[i]);
    }
    skew_fundir(a, b);
    CHECAR(skew_tamanho(a) == N);
    CHECAR(skew_vazia(b));
    CHECAR(skew_valida(a));

    qsort(vals, N, sizeof vals[0], cmp_int);
    for (i = 0; i < N; i++) {
        if (!skew_remover_minimo(a, &x) || x != vals[i]) ok = 0;
    }
    CHECAR(ok);
    CHECAR(skew_vazia(a));

    skew_destruir(a);
    skew_destruir(b);
}

/* Insercao decrescente gera um ramo esquerdo de tamanho n: a liberacao
 * da memoria nao pode depender de recursao. */
static void teste_arvore_degenerada(void)
{
    enum { N = 200000 };
    SkewHeap *h = skew_criar();
    const SkewNo *no;
    int i, x, ramo_esq = 0;

    for (i = N; i >= 1; i--) skew_inserir(h, i);
    for (no = h->raiz; no != NULL; no = no->esq) ramo_esq++;
    CHECAR(ramo_esq == N);
    CHECAR(skew_caminho_direito(h) == 1);
    CHECAR(skew_minimo(h, &x) && x == 1);
    skew_destruir(h);
}

int main(void)
{
    teste_heap_vazia();
    teste_exemplos_relatorio();
    teste_merge_iterativo_igual();
    teste_aleatorio();
    teste_ordenacao_e_fusao();
    teste_arvore_degenerada();

    if (falhas == 0) {
        printf("OK: %d verificacoes passaram.\n", verificacoes);
        return EXIT_SUCCESS;
    }
    printf("%d de %d verificacoes FALHARAM.\n", falhas, verificacoes);
    return EXIT_FAILURE;
}
