#ifndef JOGO_H
#define JOGO_H

#include "historico.h"

typedef struct {
    char nome[50];
    char tema[100];
    int  intervalo_min;
    int  intervalo_max;
    int  tentativas_max;
    char dificuldade[10];
} Caso;

void jogar_partida(int id_caso);
int confirmar_caso(int id_caso);

int  validar_palpite(int min, int max);
void dar_feedback(Caso c, int palpite, int secreto);
void exibir_cabecalho(Caso c, int tentativas_restantes);
char *classificar_pontuacao(int pontos);

#endif
