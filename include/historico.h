#ifndef HISTORICO_H
#define HISTORICO_H

typedef struct {
    char casoNome[50];
    char dificuldade[10];
    int  secreto;
    int  tentativasUsadas;
    int  venceu;
} Sessao;

typedef enum {
    ITEM_SEGUNDA_CHANCE = 0,
    ITEM_ANALISE_EXTRA,
    ITEM_SCANNER_FORENSE,
    ITEM_INTUICAO,
    ITEM_MAIS_2_TENTATIVAS,
    ITEM_MAIS_1_INTERROGATORIO,
    TOTAL_ITENS_LOJA
} ItemLoja;

void salvarSessao(Sessao s);
void exibirHistorico(void);
void creditar(int valor);
int debitar(int valor);
int getSaldo(void);
int getScore(void);
void incrementar(int valor);
void decrementar(int valor);
int getConfiancaDelegacia(void);
void aumentarConfianca(int valor);
void reduzirConfianca(int valor);
int getQuantidadeItem(ItemLoja item);
int consumirItem(ItemLoja item);
int comprarItem(ItemLoja item);
void exibirLoja(void);

#endif
