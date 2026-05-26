#ifndef HISTORICO_H
#define HISTORICO_H

typedef struct {
    char casoNome[50];
    char dificuldade[10];
    int  secreto;
    int  tentativasUsadas;
    int  venceu;
} Sessao;

void salvarSessao(Sessao s);
void exibirHistorico(void);
void creditar(int valor);
int debitar(int valor);
int getSaldo(void);

#endif
