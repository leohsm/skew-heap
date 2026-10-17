/*
 * exemplo.c - Programa de exemplo da biblioteca Skew Heap
 *
 * Uso:
 *   ./exemplo        executa a demonstracao guiada (exemplos do relatorio)
 *   ./exemplo -i     abre o menu interativo
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "skew_heap.h"

static void titulo(const char *texto)
{
    printf("\n============================================================\n");
    printf(" %s\n", texto);
    printf("============================================================\n");
}

static void mostrar(const SkewHeap *h)
{
    skew_imprimir(h, stdout);
    printf("  [tamanho=%lu  altura=%lu  caminho direito=%lu  valida=%s]\n",
           (unsigned long)skew_tamanho(h), (unsigned long)skew_altura(h),
           (unsigned long)skew_caminho_direito(h),
           skew_valida(h) ? "sim" : "NAO");
}

static SkewHeap *criar_ou_sair(void)
{
    SkewHeap *h = skew_criar();
    if (h == NULL) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        exit(EXIT_FAILURE);
    }
    return h;
}

static void inserir_ou_sair(SkewHeap *h, int chave)
{
    if (!skew_inserir(h, chave)) {
        fprintf(stderr, "Erro: memoria insuficiente.\n");
        exit(EXIT_FAILURE);
    }
}

/* ---------------------------------------------------------------------- */

static void demo_insercao_remocao(void)
{
    SkewHeap *h = criar_ou_sair();
    int valores[] = {20, 10, 30};
    int removido;
    size_t i;

    titulo("1. Insercao de 20, 10 e 30 (Figura 3 do relatorio)");
    for (i = 0; i < sizeof valores / sizeof valores[0]; i++) {
        printf("\nInserir(%d)  ->  Merge(raiz, %d)\n", valores[i], valores[i]);
        inserir_ou_sair(h, valores[i]);
        mostrar(h);
    }
    printf("\nResultado esperado: raiz 10, filho esquerdo 30, filho direito 20.\n");

    titulo("2. Remocao do minimo (Figura 4 do relatorio)");
    skew_remover_minimo(h, &removido);
    printf("RemoverMinimo() removeu %d  ->  Merge(30, 20)\n\n", removido);
    mostrar(h);
    printf("\nResultado esperado: raiz 20, filho esquerdo 30, direito nulo.\n");

    skew_destruir(h);
}

static void demo_busca(void)
{
    SkewHeap *h = criar_ou_sair();
    int alvos[] = {25, 30, 15, 99};
    size_t i;

    titulo("3. Busca (Figura 5 do relatorio)");

    /* Monta manualmente a arvore usada no exemplo do relatorio:
     *        10
     *       /  \
     *     30    20
     *          /
     *        25
     */
    h->raiz = skew_novo_no(10);
    h->raiz->esq = skew_novo_no(30);
    h->raiz->dir = skew_novo_no(20);
    h->raiz->dir->esq = skew_novo_no(25);
    h->tamanho = 4;
    mostrar(h);

    printf("\nA busca percorre a arvore, mas descarta qualquer ramo cuja\n"
           "raiz ja seja maior que o valor procurado.\n\n");
    for (i = 0; i < sizeof alvos / sizeof alvos[0]; i++) {
        size_t visitados;
        bool achou = skew_buscar(h, alvos[i], &visitados);
        printf("Buscar(%2d): %-15s (%lu de %lu nos visitados)\n", alvos[i],
               achou ? "encontrado" : "nao encontrado",
               (unsigned long)visitados, (unsigned long)skew_tamanho(h));
    }

    skew_destruir(h);
}

static void demo_fusao(void)
{
    SkewHeap *a = criar_ou_sair();
    SkewHeap *b = criar_ou_sair();
    int va[] = {12, 5, 24, 18};
    int vb[] = {7, 30, 9, 15};
    size_t i;

    titulo("4. Fusao (Merge) de duas skew heaps");
    for (i = 0; i < 4; i++) inserir_ou_sair(a, va[i]);
    for (i = 0; i < 4; i++) inserir_ou_sair(b, vb[i]);

    printf("Heap A (12, 5, 24, 18):\n");
    mostrar(a);
    printf("\nHeap B (7, 30, 9, 15):\n");
    mostrar(b);

    skew_fundir(a, b);
    printf("\nMerge(A, B):\n");
    mostrar(a);
    printf("\nHeap B apos a fusao: ");
    skew_imprimir(b, stdout);

    skew_destruir(a);
    skew_destruir(b);
}

static void demo_remover_chave(void)
{
    SkewHeap *h = criar_ou_sair();
    int valores[] = {8, 3, 14, 6, 11, 1, 9};
    size_t i;

    titulo("5. Remocao de uma chave qualquer (extra)");
    for (i = 0; i < sizeof valores / sizeof valores[0]; i++)
        inserir_ou_sair(h, valores[i]);
    mostrar(h);

    printf("\nRemover(6): o no 6 e substituido por Merge(filhos de 6)\n\n");
    skew_remover(h, 6);
    mostrar(h);

    skew_destruir(h);
}

static void demo_fila_prioridade(void)
{
    SkewHeap *h = criar_ou_sair();
    int valores[] = {42, 17, 8, 99, 23, 4, 15, 16, 61, 3, 50, 27};
    size_t n = sizeof valores / sizeof valores[0];
    size_t i;
    int x;

    titulo("6. Aplicacao: fila de prioridade / ordenacao (heapsort)");
    printf("Entrada:  ");
    for (i = 0; i < n; i++) {
        printf("%d ", valores[i]);
        inserir_ou_sair(h, valores[i]);
    }
    printf("\n\nArvore apos as insercoes (note a inclinacao para a esquerda):\n");
    mostrar(h);

    printf("\nSaida (remocoes sucessivas do minimo):  ");
    while (skew_remover_minimo(h, &x)) printf("%d ", x);
    printf("\n");

    skew_destruir(h);
}

static void demo_degenerado(void)
{
    SkewHeap *h = criar_ou_sair();
    int i, x;

    titulo("7. Arvore alta, mas caminho direito curto");
    printf("Inserindo 6, 5, 4, 3, 2, 1 (ordem decrescente): cada novo\n"
           "elemento vira raiz e a arvore antiga vai para a esquerda.\n\n");
    for (i = 6; i >= 1; i--) inserir_ou_sair(h, i);
    mostrar(h);

    printf("\nA altura e n, mas o merge so percorre caminhos DIREITOS, que\n"
           "aqui tem 1 no. Por isso remover o minimo continua barato:\n"
           "Merge(esquerda, nulo) devolve a esquerda imediatamente.\n\n");
    skew_remover_minimo(h, &x);
    printf("Removido: %d\n", x);
    mostrar(h);

    printf("\nInserir 0 tambem e imediato (0 vira a nova raiz):\n\n");
    inserir_ou_sair(h, 0);
    mostrar(h);

    skew_destruir(h);
}

/* ---------------------------------------------------------------------- */

static int ler_inteiro(const char *mensagem, int *valor)
{
    char linha[128];
    char *fim;
    long v;

    printf("%s", mensagem);
    fflush(stdout);
    if (fgets(linha, sizeof linha, stdin) == NULL) return -1; /* EOF */

    v = strtol(linha, &fim, 10);
    if (fim == linha) return 0;
    while (*fim == ' ' || *fim == '\t' || *fim == '\r' || *fim == '\n') fim++;
    if (*fim != '\0') return 0;

    *valor = (int)v;
    return 1;
}

static void modo_interativo(void)
{
    SkewHeap *h = criar_ou_sair();
    int opcao, valor, r;

    printf("Skew Heap (Min-Heap) - modo interativo\n");
    for (;;) {
        printf("\n1) Inserir   2) Remover minimo   3) Ver minimo   4) Buscar\n"
               "5) Remover chave   6) Imprimir   7) Fundir com nova heap\n"
               "8) Limpar   0) Sair\n");
        r = ler_inteiro("> ", &opcao);
        if (r < 0) break;
        if (r == 0) {
            printf("Opcao invalida.\n");
            continue;
        }

        switch (opcao) {
        case 0:
            skew_destruir(h);
            return;
        case 1:
            if (ler_inteiro("Valor: ", &valor) == 1) {
                inserir_ou_sair(h, valor);
                mostrar(h);
            } else {
                printf("Valor invalido.\n");
            }
            break;
        case 2:
            if (skew_remover_minimo(h, &valor)) {
                printf("Removido: %d\n", valor);
                mostrar(h);
            } else {
                printf("Heap vazia.\n");
            }
            break;
        case 3:
            if (skew_minimo(h, &valor)) printf("Minimo: %d\n", valor);
            else printf("Heap vazia.\n");
            break;
        case 4:
            if (ler_inteiro("Valor: ", &valor) == 1) {
                size_t visitados;
                bool achou = skew_buscar(h, valor, &visitados);
                printf("%d %s (%lu nos visitados)\n", valor,
                       achou ? "encontrado" : "nao encontrado",
                       (unsigned long)visitados);
            } else {
                printf("Valor invalido.\n");
            }
            break;
        case 5:
            if (ler_inteiro("Valor: ", &valor) == 1) {
                if (skew_remover(h, valor)) mostrar(h);
                else printf("%d nao esta na heap.\n", valor);
            } else {
                printf("Valor invalido.\n");
            }
            break;
        case 6:
            mostrar(h);
            break;
        case 7: {
            SkewHeap *outra = criar_ou_sair();
            int qtd, i;
            if (ler_inteiro("Quantos valores na nova heap? ", &qtd) != 1 || qtd < 0) {
                printf("Quantidade invalida.\n");
                skew_destruir(outra);
                break;
            }
            for (i = 0; i < qtd; i++) {
                if (ler_inteiro("  valor: ", &valor) != 1) {
                    printf("  valor invalido, ignorado.\n");
                    continue;
                }
                inserir_ou_sair(outra, valor);
            }
            printf("Nova heap:\n");
            mostrar(outra);
            skew_fundir(h, outra);
            skew_destruir(outra);
            printf("\nResultado da fusao:\n");
            mostrar(h);
            break;
        }
        case 8:
            skew_limpar(h);
            printf("Heap esvaziada.\n");
            break;
        default:
            printf("Opcao invalida.\n");
        }
    }
    skew_destruir(h);
}

int main(int argc, char **argv)
{
    if (argc > 1 && strcmp(argv[1], "-i") == 0) {
        modo_interativo();
        return EXIT_SUCCESS;
    }

    printf("Demonstracao da biblioteca Skew Heap (Min-Heap)\n");
    printf("Execute com -i para o modo interativo.\n");

    demo_insercao_remocao();
    demo_busca();
    demo_fusao();
    demo_remover_chave();
    demo_fila_prioridade();
    demo_degenerado();

    printf("\nFim da demonstracao.\n");
    return EXIT_SUCCESS;
}
