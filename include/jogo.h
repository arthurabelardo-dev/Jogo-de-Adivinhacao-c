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

typedef struct {
    int ativo;
    int duracao;
} EfeitoTemporario;

void jogarPartida(int idCaso);
int confirmarCaso(int idCaso);

int  validarPalpite(int min, int max);
void darFeedback(Caso c, int palpite, int secreto);
void exibirCabecalho(Caso c, int tentativasRestantes);
char *classificarPontuacao(int pontos);
int chancePistaFalsaPorReputacao(int reputacao);
int chanceMentiraPorReputacao(int reputacao);
int peritoPrecisoPorReputacao(int reputacao);
void iniciarEfeitoTemporario(EfeitoTemporario *efeito, int duracao);
int efeitoTemporarioAtivo(const EfeitoTemporario *efeito);
int consumirEfeitoTemporario(EfeitoTemporario *efeito);
int ajusteConfiabilidadePista(int bonusPrecisao, int penalidadeQualidade);
int calcularMultiplicadorRiscoRecompensa(int pistasColetadas);
int aplicarPenalidadeInvestigacaoExtensa(int recompensa, int pistasColetadas);

#endif
