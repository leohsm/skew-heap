/*
 * skew_heap.h - Biblioteca de Skew Heap (Min-Heap) em C
 *
 * Skew Heap (Heap Inclinada) - Sleator & Tarjan, 1986.
 * Heap auto-ajustavel: nao guarda nenhuma informacao de balanceamento
 * (sem NPL, cores, alturas ou niveis). Todas as operacoes de alteracao
 * sao baseadas na operacao de fusao (merge), que troca
 * incondicionalmente os filhos esquerdo e direito.
 *
 * Complexidade (amortizada):
 *   merge / inserir / remover_minimo : O(log n)   (pior caso isolado: O(n))
 *   minimo                           : O(1)
 *   buscar                           : O(n)
 *
 * Disciplina: Arvores e Grafos - UTFPR Campus Medianeira
 */
#ifndef SKEW_HEAP_H
#define SKEW_HEAP_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/* No da arvore: apenas a chave e os dois filhos. Nenhum campo extra. */
typedef struct SkewNo {
    int chave;
    struct SkewNo *esq;
    struct SkewNo *dir;
} SkewNo;

/* Descritor da heap: raiz e quantidade de elementos. */
typedef struct SkewHeap {
    SkewNo *raiz;
    size_t tamanho;
} SkewHeap;

/* ---------------- Criacao e destruicao ---------------- */

/* Cria uma heap vazia. Retorna NULL se faltar memoria. */
SkewHeap *skew_criar(void);

/* Libera todos os nos e o descritor. Aceita NULL. */
void skew_destruir(SkewHeap *h);

/* Remove todos os elementos, mantendo o descritor utilizavel. */
void skew_limpar(SkewHeap *h);

/* ---------------- Consultas ---------------- */

bool skew_vazia(const SkewHeap *h);
size_t skew_tamanho(const SkewHeap *h);

/* Copia o menor elemento (a raiz) para *saida. O(1).
 * Retorna false se a heap estiver vazia. */
bool skew_minimo(const SkewHeap *h, int *saida);

/* Busca uma chave qualquer. O(n), com poda: se um no ja e maior que a
 * chave procurada, seus descendentes nao sao visitados.
 * Se 'visitados' nao for NULL, recebe quantos nos foram examinados. */
bool skew_buscar(const SkewHeap *h, int chave, size_t *visitados);

/* ---------------- Alteracoes ---------------- */

/* Insere uma chave: Merge(raiz, novo_no). Retorna false se faltar memoria. */
bool skew_inserir(SkewHeap *h, int chave);

/* Remove o menor elemento: Merge(raiz->esq, raiz->dir).
 * Se 'saida' nao for NULL, recebe o valor removido.
 * Retorna false se a heap estiver vazia. */
bool skew_remover_minimo(SkewHeap *h, int *saida);

/* Remove uma ocorrencia de uma chave qualquer (extra, alem do pedido).
 * O no encontrado e substituido pela fusao de seus dois filhos.
 * Retorna false se a chave nao existir. */
bool skew_remover(SkewHeap *h, int chave);

/* Funde 'origem' em 'destino'. Ao final, 'origem' fica vazia
 * (o descritor continua valido e deve ser destruido pelo chamador). */
void skew_fundir(SkewHeap *destino, SkewHeap *origem);

/* ---------------- Operacao fundamental ---------------- */

/* Merge recursivo, exatamente como o pseudocodigo do relatorio:
 *   1. se H1 e nulo retorna H2; se H2 e nulo retorna H1;
 *   2. garante que H1 tem a menor raiz;
 *   3. H1.dir = Merge(H1.dir, H2);
 *   4. troca incondicional de H1.esq e H1.dir. */
SkewNo *skew_merge(SkewNo *h1, SkewNo *h2);

/* Mesma fusao, sem recursao (produz exatamente a mesma arvore).
 * Util quando o caminho direito fica muito longo e a recursao
 * poderia estourar a pilha. */
SkewNo *skew_merge_iterativo(SkewNo *h1, SkewNo *h2);

/* Aloca um no isolado (uma skew heap de um unico elemento). */
SkewNo *skew_novo_no(int chave);

/* ---------------- Depuracao / visualizacao ---------------- */

/* Imprime a arvore em formato hierarquico (E = esquerdo, D = direito). */
void skew_imprimir(const SkewHeap *h, FILE *saida);

/* Verifica a propriedade de heap (pai <= filhos) e o tamanho. */
bool skew_valida(const SkewHeap *h);

/* Altura da arvore (heap vazia = 0, um no = 1). */
size_t skew_altura(const SkewHeap *h);

/* Comprimento do caminho mais a direita (quantos nos a partir da raiz). */
size_t skew_caminho_direito(const SkewHeap *h);

#ifdef __cplusplus
}
#endif

#endif /* SKEW_HEAP_H */
