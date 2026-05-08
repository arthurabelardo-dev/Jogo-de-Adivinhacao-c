#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pistas.h"
#include "utils.h"

// ============================================================
// UTILITÁRIOS MATEMÁTICOS
// ============================================================

static int ehPrimo(int n) {
    if (n <= 1) return 0;
    if (n % 2 == 0) return n == 2;
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

static int somaDigitos(int n) {
    int sum = 0;
    while (n > 0) {
        sum += n % 10;
        n /= 10;
    }
    return sum;
}

static int ehQuadradoPerfeito(int n) {
    if (n < 0) return 0;
    for (int i = 0; i * i <= n; i++) {
        if (i * i == n) return 1;
    }
    return 0;
}

static int escolherIndicePorPeso(const BancoPistas *banco, const int *indices, int total, int usarConfiabilidadeDireta) {
    float pesos[20];
    float soma = 0.0f;
    float alvo = 0.0f;
    float acumulado = 0.0f;

    if (total <= 0) return -1;

    for (int i = 0; i < total; i++) {
        float conf = banco->pistas[indices[i]].confiabilidade;
        pesos[i] = usarConfiabilidadeDireta ? conf : (1.0f - conf);
        if (pesos[i] < 0.01f) pesos[i] = 0.01f;
        soma += pesos[i];
    }

    alvo = ((float)rand() / (float)RAND_MAX) * soma;
    for (int i = 0; i < total; i++) {
        acumulado += pesos[i];
        if (alvo <= acumulado) return indices[i];
    }
    return indices[total - 1];
}

// ============================================================
// PISTAS - CASO 1: O Último Suspiro do Magnata (1-50)
// ============================================================

static void inicializarPistasCaso1(BancoPistas *banco, int numeroSecreto) {
    banco->totalPistas = 0;
    banco->totalSuspeitos = 0;
    banco->minimoRequired = 1;
    banco->reputacaoGeral = 0.7f;
    
    // ===== PISTA 1: DIRETA - Contexto do quarto =====
    int idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "PERITO FORENSE: O laudo mostra que o codigo e %s.",
        (numeroSecreto % 2 == 0) ? "PAR" : "IMPAR");
    banco->pistas[idx].confiabilidade = 0.92f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 2: NARRATIVA - Horário do crime =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "VIZINHA (testemunha confiavel): O ritmo dos sons era cadenciado. "
        "No meu relogio, parecia seguir uma contagem em blocos de %d.",
        (numeroSecreto % 4 == 0) ? 4 : 5);
    banco->pistas[idx].confiabilidade = 0.68f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 3: DIRETA - Altura do código =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "TECNICO SEGURANCA: O cofre registrou duas falhas antes do acerto. "
        "No padrao desse modelo, o codigo cai na classe de resto %d quando dividido por 3.",
        numeroSecreto % 3);
    banco->pistas[idx].confiabilidade = 0.85f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 4: NARRATIVA - Idade da vítima =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "ESPOSA (suspeita, mas cooperando): Ele repetia que o numero favorito dele era %s.",
        ehPrimo(numeroSecreto) ? "primo" : "composto");
    banco->pistas[idx].confiabilidade = 0.55f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 5: FALSA - Informante suspeito =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "INFORMANTE (reputacao duvidosa): Vi alguem mencionando o numero %d na rua perto da mansao. "
        "Talvez seja isso... nao tenho certeza.",
        (numeroSecreto + 15) % 50 + 1);
    banco->pistas[idx].confiabilidade = 0.15f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 1;
    
    // ===== PISTA 6: FALSA - Delator anônimo =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "DICA ANONIMA (recebida por telefone): Um suspeito mencionou o numero %d "
        "em uma conversa no cassino da semana passada. Pode ser pista falsa.",
        (numeroSecreto - 8 + 50) % 50 + 1);
    banco->pistas[idx].confiabilidade = 0.08f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 2;
    
    // ===== PISTA 7: DIRETA - Padrão matemático =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "ANALISTA DE PADROES: A soma dos digitos do codigo e %d.",
        somaDigitos(numeroSecreto));
    banco->pistas[idx].confiabilidade = 0.88f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== SUSPEITOS =====
    banco->pistas[0].suspeitorId = 0; // Suspeito 0 = sistema
    
    // Suspeito 1: Informante de rua (pouco confiavel)
    banco->suspeitos[0].id = 1;
    strcpy(banco->suspeitos[0].nome, "Informante Anonimo");
    banco->suspeitos[0].reputacao = 0.20f;
    banco->suspeitos[0].forneceuFalsa = 0;
    banco->suspeitos[0].interrogado = 0;
    banco->suspeitos[0].eMentiroso = 0;
    
    // Suspeito 2: Delator (não confiavel)
    banco->suspeitos[1].id = 2;
    strcpy(banco->suspeitos[1].nome, "Delator Cassino");
    banco->suspeitos[1].reputacao = 0.10f;
    banco->suspeitos[1].forneceuFalsa = 0;
    banco->suspeitos[1].interrogado = 0;
    banco->suspeitos[1].eMentiroso = 0;
    
    banco->totalSuspeitos = 2;
}

// ============================================================
// PISTAS - CASO 2: Frequência de Fuga no Cassino (1-100)
// ============================================================

static void inicializarPistasCaso2(BancoPistas *banco, int numeroSecreto) {
    banco->totalPistas = 0;
    banco->totalSuspeitos = 0;
    banco->minimoRequired = 1;
    banco->reputacaoGeral = 0.75f;
    
    // ===== PISTA 1: DIRETA - Intervalo técnico =====
    int idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "TECNICO RADIO: A frequencia de fuga e %s.",
        (numeroSecreto % 2 == 0) ? "PAR" : "IMPAR");
    banco->pistas[idx].confiabilidade = 0.93f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 2: NARRATIVA - Padrão comportamental de mopretu =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "DOSSIE CRIMINAL: A gangue de mopretu sempre escolhe frequencias que sao numeros %s. "
        "Eh uma marca registrada deles - %s tem menor risco de interferencia.",
        numeroSecreto % 2 == 0 ? "PARES" : "IMPARES",
        numeroSecreto % 2 == 0 ? "frequencias pares" : "frequencias impares");
    banco->pistas[idx].confiabilidade = 0.72f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 3: DIRETA - Limites operacionais =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "ENGENHEIRO TELECOM: A frequencia %s multipla de 5.",
        (numeroSecreto % 5 == 0) ? "E" : "NAO E");
    banco->pistas[idx].confiabilidade = 0.87f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 4: NARRATIVA - Padrão histórico =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "HISTORICO DE CRIMES: No caderno de operacoes, as frequencias marcadas sempre deixavam resto %d na divisao por 3.",
        numeroSecreto % 3);
    banco->pistas[idx].confiabilidade = 0.70f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 5: FALSA - Infiltrado inimigo =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "INFORMANTE (origem duvidosa, pode ser dobrado): "
        "Ouvi na darknet que alguem falou do numero %d. Talvez seja a frequencia.",
        (numeroSecreto + 25) % 100 + 1);
    banco->pistas[idx].confiabilidade = 0.12f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 1;
    
    // ===== PISTA 6: FALSA - Policial corrupto =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "COLEGA POLICIAL (suspeita-se que recebe suborno): "
        "Um amigo meu que trabalha no cassino me contou em off que poderia ser %d. "
        "Nao tenho prova, mas ele jura que ouviu.",
        (numeroSecreto - 12 + 100) % 100 + 1);
    banco->pistas[idx].confiabilidade = 0.18f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 2;
    
    // ===== PISTA 7: DIRETA - Constraints físicos =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "SENSOR BLOQUEADOR: O bloco opera em frequencia com soma de digitos igual a %d.",
        somaDigitos(numeroSecreto));
    banco->pistas[idx].confiabilidade = 0.90f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== SUSPEITOS =====
    banco->suspeitos[0].id = 1;
    strcpy(banco->suspeitos[0].nome, "Informante Darknet");
    banco->suspeitos[0].reputacao = 0.15f;
    banco->suspeitos[0].forneceuFalsa = 0;
    banco->suspeitos[0].interrogado = 0;
    banco->suspeitos[0].eMentiroso = 0;
    
    banco->suspeitos[1].id = 2;
    strcpy(banco->suspeitos[1].nome, "Policial Corrupto");
    banco->suspeitos[1].reputacao = 0.25f;
    banco->suspeitos[1].forneceuFalsa = 0;
    banco->suspeitos[1].interrogado = 0;
    banco->suspeitos[1].eMentiroso = 0;
    
    banco->totalSuspeitos = 2;
}

// ============================================================
// PISTAS - CASO 3: Protocolo Apocalipse (1-200)
// ============================================================

static void inicializarPistasCaso3(BancoPistas *banco, int numeroSecreto) {
    banco->totalPistas = 0;
    banco->totalSuspeitos = 0;
    banco->minimoRequired = 1;
    banco->reputacaoGeral = 0.65f;
    
    // ===== PISTA 1: DIRETA - Intervalo crítico =====
    int idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "ANALISE DE LOGS: A porta infectada e %s.",
        (numeroSecreto % 2 == 0) ? "PAR" : "IMPAR");
    banco->pistas[idx].confiabilidade = 0.94f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 2: NARRATIVA - Assinatura do hacker =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "PERICIA CIBERNETICA: O padrao do atacante indica porta %s por 4.",
        (numeroSecreto % 4 == 0) ? "divisivel" : "nao divisivel");
    banco->pistas[idx].confiabilidade = 0.73f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 3: DIRETA - Limite superior =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "AUDITORIA FORENSE: O comportamento do malware indica porta %s por 5.",
        (numeroSecreto % 5 == 0) ? "divisivel" : "nao divisivel");
    banco->pistas[idx].confiabilidade = 0.91f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 4: NARRATIVA - Padrão de ataque =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "ARQUIVO CLASSIFIED: Entre os rastros, aparece classe de resto %d na divisao por 7.",
        numeroSecreto % 7);
    banco->pistas[idx].confiabilidade = 0.75f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 5: FALSA - Fonte comprometida =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "FONTE ANONIMA (recebida por meio duvidoso): Alguem que trabalha no data center "
        "do Pina passou a informacao de que a porta é %d. Nao tenho certeza se é confiavel.",
        (numeroSecreto + 40) % 200 + 1);
    banco->pistas[idx].confiabilidade = 0.08f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 1;
    
    // ===== PISTA 6: FALSA - Espiã rival =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "AGENTE RIVAL (concorrente, motivos duvidosos): Uma espia de outra agencia "
        "me contactou e sugeriu que a porta poderia ser %d. Desconfiança alta - ela quer "
        "que a missao falhe.",
        (numeroSecreto - 35 + 200) % 200 + 1);
    banco->pistas[idx].confiabilidade = 0.05f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 2;
    
    // ===== PISTA 7: DIRETA - Validação técnica =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "VALIDACAO FINAL: A porta %s um quadrado perfeito.",
        ehQuadradoPerfeito(numeroSecreto) ? "E" : "NAO E");
    banco->pistas[idx].confiabilidade = 0.89f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== SUSPEITOS =====
    banco->suspeitos[0].id = 1;
    strcpy(banco->suspeitos[0].nome, "Fonte Comprometida");
    banco->suspeitos[0].reputacao = 0.10f;
    banco->suspeitos[0].forneceuFalsa = 0;
    banco->suspeitos[0].interrogado = 0;
    banco->suspeitos[0].eMentiroso = 0;
    
    banco->suspeitos[1].id = 2;
    strcpy(banco->suspeitos[1].nome, "Espia Rival");
    banco->suspeitos[1].reputacao = 0.08f;
    banco->suspeitos[1].forneceuFalsa = 0;
    banco->suspeitos[1].interrogado = 0;
    banco->suspeitos[1].eMentiroso = 0;
    
    banco->totalSuspeitos = 2;
}

// ============================================================
// FUNÇÕES PÚBLICAS
// ============================================================

void inicializarBancoPistas(int idCaso, int numeroSecreto, BancoPistas *banco) {
    memset(banco, 0, sizeof(BancoPistas));
    banco->pistasColetadas = 0;
    
    if (idCaso == 1) {
        inicializarPistasCaso1(banco, numeroSecreto);
    } else if (idCaso == 2) {
        inicializarPistasCaso2(banco, numeroSecreto);
    } else if (idCaso == 3) {
        inicializarPistasCaso3(banco, numeroSecreto);
    }
    
    for (int i = 0; i < banco->totalPistas; i++) {
        banco->pistas[i].casoId = idCaso;
        banco->pistas[i].jaApresentada = 0;
    }
}

void apresentarPista(BancoPistas *banco, int numeroSecreto) {
    (void)numeroSecreto;
    int escolhasDiretasNarrativas[20];
    int escolhasFalsas[20];
    int totalDN = 0;
    int totalF = 0;
    int indiceDisponivel = -1;
    int triggerFalsa = 0;
    int chanceFalsa = 25;

    for (int i = 0; i < banco->totalPistas; i++) {
        if (banco->pistas[i].jaApresentada) {
            continue;
        }
        if (banco->pistas[i].tipo == PISTA_FALSA) {
            escolhasFalsas[totalF++] = i;
        } else {
            escolhasDiretasNarrativas[totalDN++] = i;
        }
    }

    if (banco->pistasColetadas >= 2) {
        if (banco->reputacaoGeral < 0.55f) {
            triggerFalsa = 1;
        }
        for (int i = 0; i < banco->totalSuspeitos; i++) {
            if (banco->suspeitos[i].reputacao < 0.20f) {
                triggerFalsa = 1;
                break;
            }
        }
    }

    if (banco->reputacaoGeral < 0.45f) {
        chanceFalsa = 50;
    }

    if (triggerFalsa && totalF > 0 && (rand() % 100) < chanceFalsa) {
        indiceDisponivel = escolherIndicePorPeso(banco, escolhasFalsas, totalF, 0);
        for (int i = 0; i < banco->totalSuspeitos; i++) {
            if (banco->suspeitos[i].id == banco->pistas[indiceDisponivel].suspeitorId) {
                banco->suspeitos[i].forneceuFalsa = 1;
                break;
            }
        }
    } else if (totalDN > 0) {
        indiceDisponivel = escolherIndicePorPeso(banco, escolhasDiretasNarrativas, totalDN, 1);
    } else if (totalF > 0) {
        indiceDisponivel = escolherIndicePorPeso(banco, escolhasFalsas, totalF, 0);
    }
    
    if (indiceDisponivel == -1) {
        printf(AMARELO "\n  PERITO: Ja analisei todas as pistas disponiveis. Nao ha mais nada para descobrir.\n\n" RESET);
        return;
    }
    
    Pista *pista = &banco->pistas[indiceDisponivel];
    pista->jaApresentada = 1;
    banco->pistasColetadas++;
    
    printf("\n");
    printf("  ============================================================\n");
    printf(CIANO "  PISTA COLETADA\n" RESET);
    printf("  ============================================================\n");
    printf("  %s\n", pista->descricao);
    printf("  ============================================================\n\n");

    if (triggerFalsa) {
        printf(AMARELO "  ALERTA DO PERITO: Ha fontes com historico duvidoso em circulacao.\n" RESET);
    }
    printf(CIANO "  [Pista %d/%d coletada]\n" RESET, banco->pistasColetadas, banco->totalPistas);
}

int verificarMinimoAceitacao(const BancoPistas *banco) {
    return banco->pistasColetadas >= banco->minimoRequired;
}

void exibirHistoricoPistas(const BancoPistas *banco) {
    printf("\n");
    printf(CIANO "  ============ HISTORICO DE PISTAS COLETADAS ==============\n" RESET);
    printf("  Total coletado: %d/%d pistas\n\n", banco->pistasColetadas, banco->totalPistas);
    
    int contadorPistas = 0;
    
    for (int i = 0; i < banco->totalPistas; i++) {
        if (banco->pistas[i].jaApresentada) {
            contadorPistas++;
            printf("  PISTA COLETADA: %s\n", banco->pistas[i].descricao);
        }
    }

    printf("\n  Resumo: %d evidencias listadas.\n", contadorPistas);
    if (banco->reputacaoGeral < 0.55f) {
        printf(AMARELO "  Aviso: existem relatos contraditorios entre as fontes.\n" RESET);
    }
    printf("  " CIANO "================================================\n\n" RESET);
}

static void gerarDeclaracaoSuspeito(int idCaso, int numeroSecreto, int mentiroso,
                                    char *saida, size_t tamanho, TipoPista *tipo, int *vinculoNumero) {
    int sorteio = rand() % 3;
    int numeroFalso = numeroSecreto;
    int maxVal = (idCaso == 1) ? 50 : (idCaso == 2) ? 100 : 200;

    numeroFalso = (numeroSecreto + (rand() % (maxVal - 1) + 1)) % maxVal + 1;

    if (mentiroso) {
        *tipo = PISTA_FALSA;
        *vinculoNumero = 0;

        if (sorteio == 0) {
            snprintf(saida, tamanho, "INTERROGADO: O codigo e %s.", (numeroSecreto % 2 == 0) ? "IMPAR" : "PAR");
        } else if (sorteio == 1) {
            snprintf(saida, tamanho, "INTERROGADO: Tenho quase certeza que o numero e %d.", numeroFalso);
        } else {
            snprintf(saida, tamanho, "INTERROGADO: Ele falava muito de resto %d na divisao por 3.", (numeroSecreto + 1) % 3);
        }
        return;
    }

    *tipo = PISTA_DIRETA;
    *vinculoNumero = 1;

    if (sorteio == 0) {
        snprintf(saida, tamanho, "INTERROGADO: O codigo e %s.", (numeroSecreto % 2 == 0) ? "PAR" : "IMPAR");
    } else if (sorteio == 1) {
        snprintf(saida, tamanho, "INTERROGADO: A soma dos digitos e %d.", somaDigitos(numeroSecreto));
    } else {
        snprintf(saida, tamanho, "INTERROGADO: Ele citou resto %d na divisao por 3.", numeroSecreto % 3);
    }
}

void prepararSuspeitosParaPartida(BancoPistas *banco) {
    float chanceMentiroso = 0.35f;

    for (int i = 0; i < banco->totalSuspeitos; i++) {
        banco->suspeitos[i].interrogado = 0;
        banco->suspeitos[i].eMentiroso = ((float)rand() / (float)RAND_MAX) < chanceMentiroso;
    }
}

int interrogarSuspeito(BancoPistas *banco, int idCaso, int numeroSecreto, int suspeitoIndex, Pista *pistaSaida) {
    if (suspeitoIndex < 0 || suspeitoIndex >= banco->totalSuspeitos || pistaSaida == NULL) {
        return 0;
    }

    Suspeito *suspeito = &banco->suspeitos[suspeitoIndex];
    if (suspeito->interrogado) {
        return 0;
    }

    suspeito->interrogado = 1;

    TipoPista tipo = PISTA_NARRATIVA;
    int vinculoNumero = 0;

    gerarDeclaracaoSuspeito(idCaso, numeroSecreto, suspeito->eMentiroso,
                            pistaSaida->descricao, sizeof(pistaSaida->descricao), &tipo, &vinculoNumero);

    pistaSaida->id = 0;
    pistaSaida->casoId = idCaso;
    pistaSaida->tipo = tipo;
    pistaSaida->confiabilidade = suspeito->eMentiroso ? 0.10f : 0.80f;
    pistaSaida->jaApresentada = 1;
    pistaSaida->vinculoNumero = vinculoNumero;
    pistaSaida->suspeitorId = suspeito->id;

    return 1;
}

int registrarPistaInterrogatorio(BancoPistas *banco, int idCaso, const Pista *pista) {
    if (banco->totalPistas >= 20 || pista == NULL) {
        return 0;
    }

    int idx = banco->totalPistas++;
    banco->pistas[idx] = *pista;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].casoId = idCaso;
    banco->pistas[idx].jaApresentada = 1;
    banco->pistasColetadas++;

    if (pista->tipo == PISTA_FALSA) {
        for (int i = 0; i < banco->totalSuspeitos; i++) {
            if (banco->suspeitos[i].id == pista->suspeitorId) {
                banco->suspeitos[i].forneceuFalsa = 1;
                break;
            }
        }
    }

    return 1;
}

void ajustarReputacaoSuspeito(BancoPistas *banco, int suspeitoId, float delta) {
    for (int i = 0; i < banco->totalSuspeitos; i++) {
        if (banco->suspeitos[i].id == suspeitoId) {
            banco->suspeitos[i].reputacao += delta;
            if (banco->suspeitos[i].reputacao < 0.0f) banco->suspeitos[i].reputacao = 0.0f;
            if (banco->suspeitos[i].reputacao > 1.0f) banco->suspeitos[i].reputacao = 1.0f;
            return;
        }
    }
}

void ajustarReputacaoGeral(BancoPistas *banco, float delta) {
    banco->reputacaoGeral += delta;
    if (banco->reputacaoGeral < 0.0f) banco->reputacaoGeral = 0.0f;
    if (banco->reputacaoGeral > 1.0f) banco->reputacaoGeral = 1.0f;
}
