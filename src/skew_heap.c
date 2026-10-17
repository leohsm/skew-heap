/*
 * skew_heap.c - Implementacao da biblioteca de Skew Heap (Min-Heap)
 */
#include "skew_heap.h"

#include <stdlib.h>
#include <string.h>

/* ======================================================================
 * Operacao fundamental: MERGE
 * ====================================================================== */

SkewNo *skew_merge(SkewNo *h1, SkewNo *h2)
{
    SkewNo *temp;

    /* 1. Casos base */
    if (h1 == NULL) return h2;
    if (h2 == NULL) return h1;

    /* 2. Garante que h1 tem a menor raiz */
    if (h1->chave > h2->chave) {
        temp = h1;
        h1 = h2;
        h2 = temp;
    }

    /* 3. Funde o filho a direita de h1 com h2 */
    h1->dir = skew_merge(h1->dir, h2);

    /* 4. Heuristica de inversao: troca incondicional dos filhos */
    temp = h1->esq;
    h1->esq = h1->dir;
    h1->dir = temp;

    return h1;
}

SkewNo *skew_merge_iterativo(SkewNo *h1, SkewNo *h2)
{
    SkewNo *raiz, *atual, *resto, *temp;

    if (h1 == NULL) return h2;
    if (h2 == NULL) return h1;

    if (h1->chave > h2->chave) {
        temp = h1;
        h1 = h2;
        h2 = temp;
    }

    /*
     * Desce pelo caminho direito. Em cada no 'atual':
     *   - o antigo filho esquerdo vira o novo filho direito (a troca);
     *   - o novo filho esquerdo sera Merge(antigo direito, resto).
     * Isso reproduz a versao recursiva passo a passo.
     */
    raiz = h1;
    atual = h1;
    resto = h2;
    for (;;) {
        SkewNo *dir = atual->dir;
        atual->dir = atual->esq;

        if (dir == NULL) {
            atual->esq = resto;
            break;
        }
        if (dir->chave > resto->chave) {
            temp = dir;
            dir = resto;
            resto = temp;
        }
        atual->esq = dir;
        atual = dir;
    }
    return raiz;
}

SkewNo *skew_novo_no(int chave)
{
    SkewNo *no = (SkewNo *)malloc(sizeof(SkewNo));
    if (no == NULL) return NULL;
    no->chave = chave;
    no->esq = NULL;
    no->dir = NULL;
    return no;
}

/* ======================================================================
 * Criacao e destruicao
 * ====================================================================== */

SkewHeap *skew_criar(void)
{
    SkewHeap *h = (SkewHeap *)malloc(sizeof(SkewHeap));
    if (h == NULL) return NULL;
    h->raiz = NULL;
    h->tamanho = 0;
    return h;
}

/*
 * Libera a arvore sem recursao. A skew heap pode ficar com um ramo
 * esquerdo muito longo (ex.: insercoes em ordem decrescente), entao uma
 * liberacao recursiva poderia estourar a pilha. Aqui, rotacoes a direita
 * "achatam" a arvore enquanto os nos sao liberados.
 */
static void liberar_nos(SkewNo *no)
{
    while (no != NULL) {
        if (no->esq != NULL) {
            SkewNo *e = no->esq;
            no->esq = e->dir;
            e->dir = no;
            no = e;
        } else {
            SkewNo *d = no->dir;
            free(no);
            no = d;
        }
    }
}

void skew_limpar(SkewHeap *h)
{
    if (h == NULL) return;
    liberar_nos(h->raiz);
    h->raiz = NULL;
    h->tamanho = 0;
}

void skew_destruir(SkewHeap *h)
{
    if (h == NULL) return;
    liberar_nos(h->raiz);
    free(h);
}

/* ======================================================================
 * Consultas
 * ====================================================================== */

bool skew_vazia(const SkewHeap *h)
{
    return h == NULL || h->raiz == NULL;
}

size_t skew_tamanho(const SkewHeap *h)
{
    return h == NULL ? 0 : h->tamanho;
}

bool skew_minimo(const SkewHeap *h, int *saida)
{
    if (skew_vazia(h)) return false;
    if (saida != NULL) *saida = h->raiz->chave;
    return true;
}

static bool buscar_no(const SkewNo *no, int chave, size_t *visitados)
{
    if (no == NULL) return false;

    (*visitados)++;
    if (no->chave == chave) return true;

    /* Poda: pela propriedade de min-heap, todos os descendentes
     * tambem sao maiores que a chave procurada. */
    if (no->chave > chave) return false;

    if (buscar_no(no->esq, chave, visitados)) return true;
    return buscar_no(no->dir, chave, visitados);
}

bool skew_buscar(const SkewHeap *h, int chave, size_t *visitados)
{
    size_t contador = 0;
    bool achou = (h != NULL) && buscar_no(h->raiz, chave, &contador);
    if (visitados != NULL) *visitados = contador;
    return achou;
}

/* ======================================================================
 * Alteracoes
 * ====================================================================== */

bool skew_inserir(SkewHeap *h, int chave)
{
    SkewNo *novo;

    if (h == NULL) return false;
    novo = skew_novo_no(chave);
    if (novo == NULL) return false;

    h->raiz = skew_merge(h->raiz, novo);
    h->tamanho++;
    return true;
}

bool skew_remover_minimo(SkewHeap *h, int *saida)
{
    SkewNo *antiga, *esq, *dir;

    if (skew_vazia(h)) return false;

    antiga = h->raiz;
    esq = antiga->esq;
    dir = antiga->dir;
    if (saida != NULL) *saida = antiga->chave;

    free(antiga);
    h->raiz = skew_merge(esq, dir);
    h->tamanho--;
    return true;
}

/*
 * Procura a chave e substitui o no encontrado pela fusao dos seus filhos.
 * A propriedade de heap se mantem: os filhos sao >= ao no removido,
 * que por sua vez era >= ao seu pai. Retorna a nova raiz da subarvore.
 */
static SkewNo *remover_no(SkewNo *no, int chave, bool *removido)
{
    if (no == NULL || no->chave > chave) return no;

    if (no->chave == chave) {
        SkewNo *substituto = skew_merge(no->esq, no->dir);
        free(no);
        *removido = true;
        return substituto;
    }

    no->esq = remover_no(no->esq, chave, removido);
    if (!*removido) no->dir = remover_no(no->dir, chave, removido);
    return no;
}

bool skew_remover(SkewHeap *h, int chave)
{
    bool removido = false;

    if (h == NULL) return false;
    h->raiz = remover_no(h->raiz, chave, &removido);
    if (removido) h->tamanho--;
    return removido;
}

void skew_fundir(SkewHeap *destino, SkewHeap *origem)
{
    if (destino == NULL || origem == NULL || destino == origem) return;

    destino->raiz = skew_merge(destino->raiz, origem->raiz);
    destino->tamanho += origem->tamanho;

    origem->raiz = NULL;
    origem->tamanho = 0;
}

/* ======================================================================
 * Depuracao / visualizacao
 * ====================================================================== */

#define PREFIXO_MAX 256

static void imprimir_no(const SkewNo *no, FILE *saida, char *prefixo,
                        size_t len, const char *rotulo, bool ultimo)
{
    const char *ramo = ultimo ? "`-- " : "|-- ";
    const char *continua = ultimo ? "    " : "|   ";
    bool tem_esq, tem_dir;

    fprintf(saida, "%s%s%s: ", prefixo, ramo, rotulo);
    if (no == NULL) {
        fprintf(saida, "(nulo)\n");
        return;
    }
    fprintf(saida, "%d\n", no->chave);

    tem_esq = no->esq != NULL;
    tem_dir = no->dir != NULL;
    if (!tem_esq && !tem_dir) return;

    if (len + 4 >= PREFIXO_MAX) {
        fprintf(saida, "%s%s(...)\n", prefixo, continua);
        return;
    }
    memcpy(prefixo + len, continua, 5); /* 4 caracteres + '\0' */

    /* Mostra o filho nulo quando o irmao existe, para deixar
     * claro de que lado cada filho esta. */
    imprimir_no(no->esq, saida, prefixo, len + 4, "E", false);
    imprimir_no(no->dir, saida, prefixo, len + 4, "D", true);

    prefixo[len] = '\0';
}

void skew_imprimir(const SkewHeap *h, FILE *saida)
{
    char prefixo[PREFIXO_MAX];
    const SkewNo *r;

    if (saida == NULL) saida = stdout;
    if (skew_vazia(h)) {
        fprintf(saida, "(heap vazia)\n");
        return;
    }

    r = h->raiz;
    fprintf(saida, "%d\n", r->chave);
    if (r->esq == NULL && r->dir == NULL) return;

    prefixo[0] = '\0';
    imprimir_no(r->esq, saida, prefixo, 0, "E", false);
    imprimir_no(r->dir, saida, prefixo, 0, "D", true);
}

static bool valida_no(const SkewNo *no, size_t *contador)
{
    if (no == NULL) return true;
    (*contador)++;
    if (no->esq != NULL && no->esq->chave < no->chave) return false;
    if (no->dir != NULL && no->dir->chave < no->chave) return false;
    return valida_no(no->esq, contador) && valida_no(no->dir, contador);
}

bool skew_valida(const SkewHeap *h)
{
    size_t contador = 0;
    if (h == NULL) return false;
    if (!valida_no(h->raiz, &contador)) return false;
    return contador == h->tamanho;
}

static size_t altura_no(const SkewNo *no)
{
    size_t ae, ad;
    if (no == NULL) return 0;
    ae = altura_no(no->esq);
    ad = altura_no(no->dir);
    return 1 + (ae > ad ? ae : ad);
}

size_t skew_altura(const SkewHeap *h)
{
    return h == NULL ? 0 : altura_no(h->raiz);
}

size_t skew_caminho_direito(const SkewHeap *h)
{
    size_t n = 0;
    const SkewNo *no = (h == NULL) ? NULL : h->raiz;
    while (no != NULL) {
        n++;
        no = no->dir;
    }
    return n;
}
