#ifndef HISTORICO_H
#define HISTORICO_H

typedef struct {
    char caso_nome[50];
    char dificuldade[10];
    int  secreto;
    int  tentativas_usadas;
    int  venceu;
} Sessao;

void salvar_sessao(Sessao s);
void exibir_historico(void);
void creditar(int valor);
int debitar(int valor);
int getSaldo(void);

#endif
