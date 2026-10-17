# Skew Heap em C

**UTFPR - Câmpus Medianeira | Ciência da Computação | Árvores e Grafos | 2026.2**

**Equipe:** Erik Mazzuco, Letícia Moro, Leonardo Herrero

Biblioteca em C de uma **Skew Heap** (Min-Heap), com um programa de exemplo e testes automatizados.
Trabalho do Seminário 2 – Heaps da disciplina **Árvores e Grafos** (UTFPR – Câmpus Medianeira, 2026.2).

A Skew Heap (Sleator & Tarjan, 1986) é uma heap auto-ajustável: não guarda nenhuma informação de
balanceamento nos nós (sem NPL, cor, altura ou nível). Todas as operações de alteração são feitas com a
**fusão (merge)**, que funde os caminhos direitos e depois **troca incondicionalmente** os filhos esquerdo e direito.

## Estrutura

```
include/skew_heap.h   interface pública da biblioteca
src/skew_heap.c       implementação
examples/exemplo.c    programa de exemplo (demonstração guiada + modo interativo)
tests/teste.c         testes automatizados
Makefile              compilação com make
compilar.bat          compilação no Windows sem make
```

## Compilação

Requer um compilador C99 (gcc/MinGW).

```sh
make            # gera libskewheap.a e o programa exemplo
make executar   # roda a demonstração
make teste      # compila e roda os testes
```

No Windows sem `make`, rode `compilar.bat`. Também é possível compilar direto:

```sh
gcc -std=c99 -Wall -Iinclude -c src/skew_heap.c -o skew_heap.o
ar rcs libskewheap.a skew_heap.o
gcc -std=c99 -Wall -Iinclude examples/exemplo.c -L. -lskewheap -o exemplo
```

## Programa de exemplo

```sh
./exemplo       # demonstração guiada
./exemplo -i    # menu interativo (inserir, remover, buscar, fundir, imprimir...)
```

A demonstração reproduz os exemplos do relatório:

1. Inserção de 20, 10 e 30 (Figura 3)
2. Remoção do mínimo (Figura 4)
3. Busca com poda pelo valor 25 (Figura 5)
4. Fusão de duas heaps
5. Remoção de uma chave qualquer
6. Uso como fila de prioridade / heapsort
7. Árvore alta com caminho direito curto

Exemplo de saída (inserção de 20, 10 e 30):

```
Inserir(30)  ->  Merge(raiz, 30)
10
|-- E: 30
`-- D: 20
  [tamanho=3  altura=2  caminho direito=2  valida=sim]
```

## API

```c
#include "skew_heap.h"

SkewHeap *h = skew_criar();

skew_inserir(h, 20);                 /* Merge(raiz, novo_no)        */
skew_inserir(h, 10);
skew_inserir(h, 30);

int min;
skew_minimo(h, &min);                /* O(1): min == 10             */
skew_remover_minimo(h, &min);        /* Merge(raiz->esq, raiz->dir) */

size_t visitados;
bool achou = skew_buscar(h, 30, &visitados);

skew_imprimir(h, stdout);
skew_destruir(h);
```

| Função | Descrição | Complexidade |
|---|---|---|
| `skew_criar` / `skew_destruir` / `skew_limpar` | cria, libera, esvazia | O(1) / O(n) / O(n) |
| `skew_inserir(h, x)` | insere `x` (merge com um nó isolado) | O(log n) amortizado |
| `skew_minimo(h, &x)` | lê o menor elemento (raiz) | O(1) |
| `skew_remover_minimo(h, &x)` | remove a raiz e funde os filhos | O(log n) amortizado |
| `skew_buscar(h, x, &visitados)` | busca com poda (para em nós maiores que `x`) | O(n) |
| `skew_remover(h, x)` | remove uma chave qualquer (extra) | O(n) |
| `skew_fundir(dest, orig)` | funde `orig` em `dest`; `orig` fica vazia | O(log n) amortizado |
| `skew_merge(h1, h2)` | merge recursivo dos nós (igual ao pseudocódigo) | O(log n) amortizado |
| `skew_merge_iterativo(h1, h2)` | mesmo resultado, sem recursão | O(log n) amortizado |
| `skew_imprimir`, `skew_valida`, `skew_altura`, `skew_caminho_direito` | depuração e visualização | O(n) |

Funções que podem falhar retornam `bool` (`false` para heap vazia, chave inexistente ou falta de memória).

## Operação fundamental: merge

```c
SkewNo *skew_merge(SkewNo *h1, SkewNo *h2)
{
    SkewNo *temp;

    if (h1 == NULL) return h2;
    if (h2 == NULL) return h1;

    if (h1->chave > h2->chave) {          /* h1 fica com a menor raiz */
        temp = h1; h1 = h2; h2 = temp;
    }

    h1->dir = skew_merge(h1->dir, h2);    /* funde pelo caminho direito */

    temp = h1->esq;                       /* troca incondicional */
    h1->esq = h1->dir;
    h1->dir = temp;

    return h1;
}
```

## Testes

`make teste` executa mais de 46 mil verificações:

- exemplos do relatório (formato exato da árvore após cada passo);
- 20 mil operações aleatórias comparadas com um vetor de referência;
- heapsort de 5 mil elementos após a fusão de duas heaps;
- `skew_merge_iterativo` gera a mesma árvore que `skew_merge`;
- árvore degenerada com 200 mil nós (a liberação de memória é iterativa e não estoura a pilha).

Os testes também passam com `-fsanitize=address,undefined`.

## Observações

- `skew_buscar`, `skew_remover`, `skew_imprimir`, `skew_valida` e `skew_altura` são recursivas, com profundidade
  igual à altura da árvore. Inserções em ordem decrescente geram uma árvore de altura n, então use-as com
  cuidado em heaps muito grandes e degeneradas.
- `skew_destruir`/`skew_limpar` não usam recursão.

## Referências

- SLEATOR, D. D.; TARJAN, R. E. *Self-adjusting heaps*. SIAM Journal on Computing, v. 15, n. 1, p. 52–69, 1986.
- University of Washington. *CSE 326: Leftist and Skew Heaps*.
- NIPKOW, T. *Skew Heap*. Archive of Formal Proofs, 2014.
