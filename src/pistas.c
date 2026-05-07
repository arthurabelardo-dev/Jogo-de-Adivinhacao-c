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

// ============================================================
// PISTAS - CASO 1: O Último Suspiro do Magnata (1-50)
// ============================================================

static void inicializarPistas_Caso1(BancoPistas *banco, int numeroSecreto) {
    banco->totalPistas = 0;
    banco->totalSuspeitos = 0;
    banco->minimoRequired = 3;
    banco->reputacaoGeral = 0.7f;
    
    // ===== PISTA 1: DIRETA - Contexto do quarto =====
    int idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "PERITO FORENSE: Achei marcas de queimadura no teclado numerico do cofre. "
        "A ultima digital marca os botoes %d e %d. Isso aponta para um codigo entre %d e %d.",
        numeroSecreto / 10, numeroSecreto % 10,
        (numeroSecreto / 10) * 10, ((numeroSecreto / 10) + 1) * 10);
    banco->pistas[idx].confiabilidade = 0.92f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 2: NARRATIVA - Horário do crime =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    int horaDoHouror = (numeroSecreto / 5) + 9; // Interpretação: "exatamente tantas horas antes da meia-noite"
    sprintf(banco->pistas[idx].descricao,
        "VIZINHA (testemunha confiavel): Ouvi barulho de vidro quebrando exatamente %d vezes em sequencia. "
        "Parecia ritmado como alguem digitando... de %d em %d.",
        numeroSecreto % 3 + 1, numeroSecreto % 7 + 1, numeroSecreto % 5 + 1);
    banco->pistas[idx].confiabilidade = 0.68f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 3: DIRETA - Altura do código =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "TECNICO SEGURANCA: O sistema registrou %d tentativas de acesso falhadas ANTES do sucesso. "
        "Padrao muito especifico - o assassino conhecia o codigo, apenas testou a senha %d vez antes de acertar.",
        numeroSecreto % 5 + 1, numeroSecreto % 3 + 1);
    banco->pistas[idx].confiabilidade = 0.85f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 4: NARRATIVA - Idade da vítima =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "ESPOSA (suspeita, mas cooperando): Meu marido tinha uma obsessao com numeros que representavam "
        "datas significativas. Sua filha nasceu num %d de um mes. O codigo pode estar ligado a isso.",
        numeroSecreto % 28 + 1);
    banco->pistas[idx].confiabilidade = 0.55f;
    banco->pistas[idx].vinculoNumero = (numeroSecreto % 7 == 0) ? 1 : 0;
    
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
        "ANALISTA DE PADROES: O codigo definitivamente esta ENTRE %d e %d. "
        "As assinaturas de calor no teclado sao conclusivas sobre esse intervalo.",
        numeroSecreto - 5 > 1 ? numeroSecreto - 5 : 1, numeroSecreto + 5);
    banco->pistas[idx].confiabilidade = 0.88f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== SUSPEITOS =====
    banco->pistas[0].suspeitorId = 0; // Suspeito 0 = sistema
    
    // Suspeito 1: Informante de rua (pouco confiavel)
    banco->suspeitos[0].id = 1;
    strcpy(banco->suspeitos[0].nome, "Informante Anonimo");
    banco->suspeitos[0].reputacao = 0.20f;
    banco->suspeitos[0].forneceuFalsa = 0;
    
    // Suspeito 2: Delator (não confiavel)
    banco->suspeitos[1].id = 2;
    strcpy(banco->suspeitos[1].nome, "Delator Cassino");
    banco->suspeitos[1].reputacao = 0.10f;
    banco->suspeitos[1].forneceuFalsa = 0;
    
    banco->totalSuspeitos = 2;
}

// ============================================================
// PISTAS - CASO 2: Frequência de Fuga no Cassino (1-100)
// ============================================================

static void inicializarPistas_Caso2(BancoPistas *banco, int numeroSecreto) {
    banco->totalPistas = 0;
    banco->totalSuspeitos = 0;
    banco->minimoRequired = 4;
    banco->reputacaoGeral = 0.75f;
    
    // ===== PISTA 1: DIRETA - Intervalo técnico =====
    int idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "TECNICO RADIO: A frequencia esta DEFINITIVAMENTE entre %d e %d MHz. "
        "Os analisadores de espectro nao mentem. Confianca 100%% nesse intervalo.",
        numeroSecreto - 8 > 1 ? numeroSecreto - 8 : 1, numeroSecreto + 8);
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
        "ENGENHEIRO TELECOM: Analisamos as transmissoes precedentes deles. "
        "A frequencia usada é MAIOR que %d MHz com 87%% de certeza.",
        numeroSecreto - 20 > 1 ? numeroSecreto - 20 : 1);
    banco->pistas[idx].confiabilidade = 0.87f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 4: NARRATIVA - Padrão histórico =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    int primeiroAlgarismo = numeroSecreto / 10;
    sprintf(banco->pistas[idx].descricao,
        "HISTORICO DE CRIMES: Em operacoes de fuga anteriores, mopretu usou frequencias "
        "que comecam com %d algo (tipo %d0, %d5, %d9, etc). "
        "Esse padrao aparece em 6 de seus 8 ultimos crimes.",
        primeiroAlgarismo, primeiroAlgarismo, primeiroAlgarismo, primeiroAlgarismo);
    banco->pistas[idx].confiabilidade = 0.70f;
    banco->pistas[idx].vinculoNumero = (numeroSecreto / 10 == primeiroAlgarismo) ? 1 : 0;
    
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
        "SENSOR BLOQUEADOR: O bloqueador deles opera em determinada faixa. "
        "Podemos DESCARTAR completamente frequencias abaixo de %d MHz com confianca.",
        numeroSecreto - 15 > 1 ? numeroSecreto - 15 : 1);
    banco->pistas[idx].confiabilidade = 0.90f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== SUSPEITOS =====
    banco->suspeitos[0].id = 1;
    strcpy(banco->suspeitos[0].nome, "Informante Darknet");
    banco->suspeitos[0].reputacao = 0.15f;
    banco->suspeitos[0].forneceuFalsa = 0;
    
    banco->suspeitos[1].id = 2;
    strcpy(banco->suspeitos[1].nome, "Policial Corrupto");
    banco->suspeitos[1].reputacao = 0.25f;
    banco->suspeitos[1].forneceuFalsa = 0;
    
    banco->totalSuspeitos = 2;
}

// ============================================================
// PISTAS - CASO 3: Protocolo Apocalipse (1-200)
// ============================================================

static void inicializarPistas_Caso3(BancoPistas *banco, int numeroSecreto) {
    banco->totalPistas = 0;
    banco->totalSuspeitos = 0;
    banco->minimoRequired = 5;
    banco->reputacaoGeral = 0.65f;
    
    // ===== PISTA 1: DIRETA - Intervalo crítico =====
    int idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "ANALISE DE LOGS: Os registros do firewall indicam DEFINITIVAMENTE que a porta "
        "esta entre %d e %d. Isolamos com 94%% de precisao.",
        numeroSecreto - 15 > 1 ? numeroSecreto - 15 : 1, numeroSecreto + 15);
    banco->pistas[idx].confiabilidade = 0.94f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 2: NARRATIVA - Assinatura do hacker =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    int primeiroDigito = numeroSecreto / 100;
    sprintf(banco->pistas[idx].descricao,
        "PERICIA CIBERNETICA: O CH do Pina tem uma assinatura digital bem estudada. "
        "Todos seus exploits recentes usam portas que comecam com %d (como %d00, %d50, etc). "
        "Eh obsessao dele com numeros 'redondos'.",
        primeiroDigito, primeiroDigito, primeiroDigito);
    banco->pistas[idx].confiabilidade = 0.73f;
    banco->pistas[idx].vinculoNumero = (numeroSecreto / 100 == primeiroDigito) ? 1 : 0;
    
    // ===== PISTA 3: DIRETA - Limite superior =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "AUDITORIA FORENSE: Rastreamos todos os pacotes maliciosos ate a origem. "
        "DESCARTAMOS COMPLETAMENTE portas acima de %d - a engenharia de rede do Pina nao permite.",
        numeroSecreto + 25);
    banco->pistas[idx].confiabilidade = 0.91f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 4: NARRATIVA - Padrão de ataque =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "ARQUIVO CLASSIFIED: O Pina tem obcessao com numeros %s desde seus primeiros crimes. "
        "Dos 12 ataques documentados, 11 usaram portas %s. Eh praticamente uma obsessao psicopata.",
        numeroSecreto % 2 == 0 ? "PARES" : "IMPARES",
        numeroSecreto % 2 == 0 ? "pares" : "impares");
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
        "VALIDACAO FINAL: Usando engenharia reversa no codigo-fonte do malware, "
        "confirmamos que a porta é MAIOR que %d. Isso reduz significativamente o escopo.",
        numeroSecreto - 30 > 1 ? numeroSecreto - 30 : 1);
    banco->pistas[idx].confiabilidade = 0.89f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== SUSPEITOS =====
    banco->suspeitos[0].id = 1;
    strcpy(banco->suspeitos[0].nome, "Fonte Comprometida");
    banco->suspeitos[0].reputacao = 0.10f;
    banco->suspeitos[0].forneceuFalsa = 0;
    
    banco->suspeitos[1].id = 2;
    strcpy(banco->suspeitos[1].nome, "Espia Rival");
    banco->suspeitos[1].reputacao = 0.08f;
    banco->suspeitos[1].forneceuFalsa = 0;
    
    banco->totalSuspeitos = 2;
}

// ============================================================
// FUNÇÕES PÚBLICAS
// ============================================================

void inicializarBancoPistas(int idCaso, int numeroSecreto, BancoPistas *banco) {
    memset(banco, 0, sizeof(BancoPistas));
    banco->pistasColetadas = 0;
    
    if (idCaso == 1) {
        inicializarPistas_Caso1(banco, numeroSecreto);
    } else if (idCaso == 2) {
        inicializarPistas_Caso2(banco, numeroSecreto);
    } else if (idCaso == 3) {
        inicializarPistas_Caso3(banco, numeroSecreto);
    }
    
    for (int i = 0; i < banco->totalPistas; i++) {
        banco->pistas[i].jaApresentada = 0;
    }
}

void apresentarPista(BancoPistas *banco, int numeroSecreto) {
    // Encontrar uma pista ainda não apresentada
    int indiceDisponivel = -1;
    for (int i = 0; i < banco->totalPistas; i++) {
        if (!banco->pistas[i].jaApresentada) {
            indiceDisponivel = i;
            break;
        }
    }
    
    if (indiceDisponivel == -1) {
        printf(AMARELO "\n  PERITO: Ja analisei todas as pistas disponiveis. Nao ha mais nada para descobrir.\n\n" RESET);
        return;
    }
    
    Pista *pista = &banco->pistas[indiceDisponivel];
    pista->jaApresentada = 1;
    banco->pistasColetadas++;
    
    // Exibir a pista com visual diferente de acordo com o tipo
    printf("\n");
    printf("  ============================================================\n");
    
    int percentual = (int)(pista->confiabilidade * 100);
    
    if (pista->tipo == PISTA_DIRETA) {
        printf(VERDE "  [PISTA DIRETA] Confiabilidade: %d%%\n" RESET, percentual);
    } else if (pista->tipo == PISTA_NARRATIVA) {
        printf(AMARELO "  [PISTA NARRATIVA] Confiabilidade: %d%%\n" RESET, percentual);
    } else {
        printf(VERMELHO "  [PISTA SUSPEITA/FALSA] Confiabilidade: %d%%\n" RESET, percentual);
    }
    
    printf("  ============================================================\n");
    printf("  %s\n", pista->descricao);
    printf("  ============================================================\n\n");
    
    printf(CIANO "  [Pista %d/%d coletada]\n" RESET, banco->pistasColetadas, banco->totalPistas);
}

int verificarMinimoAceitacao(const BancoPistas *banco) {
    return banco->pistasColetadas >= banco->minimoRequired;
}

void exibirHistoricoPistas(const BancoPistas *banco) {
    printf("\n");
    printf(CIANO "  ============ HISTORICO DE PISTAS COLETADAS ==============\n" RESET);
    printf("  Total coletado: %d/%d pistas\n\n", banco->pistasColetadas, banco->totalPistas);
    
    int contadorDireta = 0, contadorNarrativa = 0, contadorFalsa = 0;
    
    for (int i = 0; i < banco->totalPistas; i++) {
        if (banco->pistas[i].jaApresentada) {
            int percentual = (int)(banco->pistas[i].confiabilidade * 100);
            
            if (banco->pistas[i].tipo == PISTA_DIRETA) {
                printf(VERDE "  [D] " RESET);
                contadorDireta++;
            } else if (banco->pistas[i].tipo == PISTA_NARRATIVA) {
                printf(AMARELO "  [N] " RESET);
                contadorNarrativa++;
            } else {
                printf(VERMELHO "  [F] " RESET);
                contadorFalsa++;
            }
            printf("Conf: %3d%% | %s\n", percentual, banco->pistas[i].descricao);
        }
    }
    
    printf("\n  Resumo: %d Diretas | %d Narrativas | %d Falsas\n", contadorDireta, contadorNarrativa, contadorFalsa);
    printf("  " CIANO "================================================\n\n" RESET);
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
