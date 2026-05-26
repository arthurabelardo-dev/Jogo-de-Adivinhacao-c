#ifndef JOGO_H
#define JOGO_H

#include "historico.h"

typedef struct {
    char nome[50];
    char tema[100];
    int  intervaloMin;
    int  intervaloMax;
    int  tentativasMax;
    char dificuldade[10];
} Caso;

void jogarPartida(int idCaso);
int confirmarCaso(int idCaso);

int  validarPalpite(int min, int max);
void darFeedback(Caso c, int palpite, int secreto);
void exibirCabecalho(Caso c, int tentativasRestantes);
char *classificarPontuacao(int pontos);

#endif
