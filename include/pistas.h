#ifndef PISTAS_H
#define PISTAS_H

// Tipos de pistas
typedef enum {
    PISTA_DIRETA,     // Aponta claramente para elementos do número
    PISTA_NARRATIVA,  // Exige interpretação, sem indicar número diretamente
    PISTA_FALSA       // Pode confundir com as verdadeiras
} TipoPista;

// Estrutura para suspeitos que podem forneceder pistas falsas
typedef struct {
    int id;
    char nome[50];
    float reputacao;  // 0.0-1.0 (0=não confia, 1=confia totalmente)
    int forneceuFalsa; // Se já forneceu pista falsa
} Suspeito;

// Estrutura de dados para cada pista
typedef struct {
    int id;
    int casoId;
    TipoPista tipo;
    char descricao[300];
    float confiabilidade;  // 0.0-1.0 (0.0=nunca confiar, 1.0=totalmente confiavel)
    int jaApresentada;     // Flag se já foi mostrada ao jogador
    int vinculoNumero;     // Se é verdadeira para o caso específico
    int suspeitorId;       // ID do suspeito que forneceu (se tipo=FALSA)
} Pista;

// Estrutura para gerenciar todas as pistas de um caso
typedef struct {
    Pista pistas[20];
    int totalPistas;
    int pistasColetadas;   // Quantas pistas únicas o jogador viu
    int minimoRequired;    // Mínimo de pistas para permitir chutar
    int maxPistasPermitidas; // Máximo de pistas que podem ser coletadas nesta dificuldade
    Suspeito suspeitos[5]; // Suspeitos que podem fornecer info
    int totalSuspeitos;
    float reputacaoGeral;  // Influencia quantas falsas aparecem
} BancoPistas;

// Funções
void inicializarBancoPistas(int idCaso, int numeroSecreto, BancoPistas *banco);
void apresentarPista(BancoPistas *banco, int numeroSecreto);
int verificarMinimoAceitacao(const BancoPistas *banco);
void exibirHistoricoPistas(const BancoPistas *banco);
void ajustarReputacaoSuspeito(BancoPistas *banco, int suspeitoId, float delta);
void ajustarReputacaoGeral(BancoPistas *banco, float delta);

#endif
