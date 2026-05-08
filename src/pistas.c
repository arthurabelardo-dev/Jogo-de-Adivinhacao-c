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

static void inicializarPistas_Caso1(BancoPistas *banco, int numeroSecreto) {
    banco->totalPistas = 0;
    banco->totalSuspeitos = 0;
    banco->minimoRequired = 1;
    banco->maxPistasPermitidas = 4;
    banco->reputacaoGeral = 0.7f;
    
    // ===== PISTA 1: DIRETA - Análise do Cofre (Temática: Segurança de Mansão) =====
    int idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "ESPECIALISTA EM COFRES: O modelo antigo de Gaga registra TUDO nos logs internos. "
        "A digital no teclado aponta para um numero %s. Cofres dessa era sao impecaveis nessas medicoes.",
        (numeroSecreto % 2 == 0) ? "PAR" : "IMPAR");
    banco->pistas[idx].confiabilidade = 0.92f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 2: NARRATIVA - Hábitos do Magnata =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "MORDOMO PESSOAL (ex-empregado): O Sr. Gaga tinha rituais estranhos. "
        "Frequentemente mencionava que 'o numero sagrado divide-se em %d partes' quando discutia "
        "seus codigos secretos com a familia. Eh uma obsessao dele com simetria.",
        (numeroSecreto % 4 == 0) ? 4 : (numeroSecreto % 3 == 0) ? 3 : 5);
    banco->pistas[idx].confiabilidade = 0.68f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 3: DIRETA - Padrão de Segurança (Temática: Sistema de Proteção) =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "DESIGNER DO SISTEMA DE SEGURANCA: Esse cofre especifico tem uma caracteristica unica. "
        "Os codigos que funcionam SEMPRE deixam resto %d na divisao por 3. Eh a assinatura do sistema.",
        numeroSecreto % 3);
    banco->pistas[idx].confiabilidade = 0.85f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 4: NARRATIVA - Lenda da Familia =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "HISTORIADOR DA FAMILIA: Na genealogia dos Gaga, ha um padrao curioso nos tresouros familiares. "
        "Todos os cofres da linhagem usam numeros %s - ha registros de 200 anos disso. "
        "E praticamente uma tradicao de classe entre a elite.",
        ehPrimo(numeroSecreto) ? "PRIMOS (indivisiveis, como o poder deles)" : "COMPOSTOS (feitos de multiplas partes, como sua influencia)");
    banco->pistas[idx].confiabilidade = 0.55f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 5: DIRETA - Analise de Cameras de Seguranca =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "CHEFE DE SEGURANCA (agora demitido): As cameras do corredor capturam o reflexo no cofre. "
        "No momento da abertura, o padrao dos toques indica que o codigo deixa resto %d quando dividido por 10. "
        "Nao e prova completa, mas e um detalhe consistente da gravacao.",
        numeroSecreto % 10);
    banco->pistas[idx].confiabilidade = 0.90f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 6: FALSA - Jornalista Sensacionalista =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "JORNALISTA DO SCANDAL (tablóide rosa): Fontes próximas à familia Gaga revelaram que "
        "o numero secreto e %d. Eh boato de bastidores, mas circula na alta sociedade.",
        (numeroSecreto + 20) % 50 + 1);
    banco->pistas[idx].confiabilidade = 0.10f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 1;
    
    // ===== PISTA 7: DIRETA - Dados do Fabricante do Cofre =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "FABRICANTE SUICO DO COFRE: Nossos arquivos de 1987 mostram que o codigo desse modelo "
        "tem uma propriedade: a soma dos seus algarismos e EXATAMENTE %d. Essa e a MARCA do nosso design.",
        somaDigitos(numeroSecreto));
    banco->pistas[idx].confiabilidade = 0.88f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 8 (EXTRA): NARRATIVA - Carta Criptografada =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "CRIPTOGRAFO APOSENTADO: Encontramos uma carta de amor entre Gaga e sua primeira esposa (falecida). "
        "Usa um codigo interior: 'meu amor, teu segredo danca em passos %s no silencio'. "
        "Parece uma referencia romantica ao comportamento matematico da senha.",
        (numeroSecreto % 2 == 0) ? "aos pares" : "impares");
    banco->pistas[idx].confiabilidade = 0.65f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 9 (EXTRA): FALSA - Rival Empresarial =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "RIVAL CORPORATIVO (concorrente): Tenho conexoes na empresa de Gaga. "
        "Um antigo CFO me contou que o numero deve ser %d. Mas nao confio nele completamente.",
        (numeroSecreto - 12 + 50) % 50 + 1);
    banco->pistas[idx].confiabilidade = 0.12f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 2;
    
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
    banco->minimoRequired = 1;
    banco->maxPistasPermitidas = 3;
    banco->reputacaoGeral = 0.75f;
    
    // ===== PISTA 1: DIRETA - Perito do cassino =====
    int idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "PERITO DO CASSINO: Os registros eletronicos da sala de jogo mostram que a senha "
        "e um numero %s. Consegui acessar os logs do sistema.",
        (numeroSecreto % 2 == 0) ? "PAR" : "IMPAR");
    banco->pistas[idx].confiabilidade = 0.93f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 2: NARRATIVA - Padrão dos roubos anteriores =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "GERENTE DE SEGURANCA (relatorio confidencial): Analisando os casos anteriores de sabotagem, "
        "os criminosos SEMPRE usaram codigos que sao numeros %s. Eh quase uma assinatura deles - "
        "nos apostamos que se a tendencia continuar, sera %s novamente.",
        numeroSecreto % 2 == 0 ? "PARES" : "IMPARES",
        numeroSecreto % 2 == 0 ? "PAR" : "IMPAR");
    banco->pistas[idx].confiabilidade = 0.68f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 3: DIRETA - Especialista em sistemas =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "ESPECIALISTA EM SISTEMAS: O firewall do cassino registra tentativas de acesso. "
        "Baseado nos padroes, a senha %s divisivel por 5 (estrutura padrao dos codigos antigos do sistema).",
        (numeroSecreto % 5 == 0) ? "E" : "NAO E");
    banco->pistas[idx].confiabilidade = 0.87f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 4: NARRATIVA - Arquivo histórico de incidentes =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "ARQUIVO DE INCIDENTES: Consultei documentos antigos do cassino. Em TODOS os "
        "acessos nao autorizados que conseguimos rastrear, o numero de acesso deixava resto %d "
        "quando dividido por 3. Os criminosos nao sabem, mas repetimos esse padrao.",
        numeroSecreto % 3);
    banco->pistas[idx].confiabilidade = 0.65f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 5: FALSA - Informante da rota negra =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "INFORMANTE (rota negra, confiabilidade duvidosa): "
        "Alguem na dark web disse que pode ser %d. Nao tenho certeza, mas essa pessoa afirma "
        "ter acesso aos hackersque supostamente sabem a senha.",
        (numeroSecreto + 25) % 100 + 1);
    banco->pistas[idx].confiabilidade = 0.14f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 1;
    
    // ===== PISTA 6: FALSA - Segurança interna comprometida =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "SEGURANCA INTERNA (suspeita-se que foi cooptada): "
        "Um colega que trabalha aqui me chamou no privado e susurrou que poderia ser %d. "
        "Ele nao quis falar com ninguem mais. Pode ser desinformacao para nos enganar.",
        (numeroSecreto - 12 + 100) % 100 + 1);
    banco->pistas[idx].confiabilidade = 0.16f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 2;
    
    // ===== PISTA 7: DIRETA - Detector de anomalias =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "DETECTOR DE ANOMALIAS: O sistema de monitoramento flagra padroes anormais. "
        "A senha tem soma de digitos igual a %d (caracteristica que nenhum hacker esperaria).",
        somaDigitos(numeroSecreto));
    banco->pistas[idx].confiabilidade = 0.90f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== SUSPEITOS =====
    banco->suspeitos[0].id = 1;
    strcpy(banco->suspeitos[0].nome, "Informante Rota Negra");
    banco->suspeitos[0].reputacao = 0.15f;
    banco->suspeitos[0].forneceuFalsa = 0;
    
    banco->suspeitos[1].id = 2;
    strcpy(banco->suspeitos[1].nome, "Seguranca Comprometida");
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
    banco->minimoRequired = 1;
    banco->maxPistasPermitidas = 2;
    banco->reputacaoGeral = 0.65f;
    
    // ===== PISTA 1: DIRETA - Analista de malware =====
    int idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "ANALISTA DE MALWARE: Os registros de trafego mostram que a porta comprometida e %s. "
        "O padrao de operacao do worm aponta claramente para isso.",
        (numeroSecreto % 2 == 0) ? "PAR" : "IMPAR");
    banco->pistas[idx].confiabilidade = 0.94f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 2: NARRATIVA - Assinatura do ciberataque =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "ENGENHEIRO DE SEGURANCA: Comparando com ataques anteriores da mesma célula, "
        "vemos um padrao claro - eles usam portas que sao %s por 4. Eh quase uma assinatura "
        "cibernetica deles, como uma digital no mundo virtual.",
        (numeroSecreto % 4 == 0) ? "DIVISEIS" : "NAO DIVISEIS");
    banco->pistas[idx].confiabilidade = 0.70f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 3: DIRETA - Firewall inteligente =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "FIREWALL INTELIGENTE: A IA de deteccao anomalica identificou que a porta explorada "
        "%s multipla de 5 (estrutura que o ataque necessita para funcionar).",
        (numeroSecreto % 5 == 0) ? "E" : "NAO E");
    banco->pistas[idx].confiabilidade = 0.91f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 4: NARRATIVA - Inspeção de tráfego =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_NARRATIVA;
    sprintf(banco->pistas[idx].descricao,
        "INSPECAO DE TRAFICO: Nos pacotes capturados, encontramos evidencias codificadas. "
        "A chave criptografica deixa resto %d quando dividida por 7. Eh o tipo de assinatura "
        "que hackers nao conseguem mudar facilmente.",
        numeroSecreto % 7);
    banco->pistas[idx].confiabilidade = 0.72f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== PISTA 5: FALSA - Tecnólogo do data center =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "TECNÓLOGO DO DATA CENTER (suspeita-se que foi COOPTADO): "
        "Um funcionario de dentro do servidor passou uma dica anonima dizendo que pode ser porta %d. "
        "Desconfianca ALTA - sabemos que foi abordado pelas mesmas pessoas.",
        (numeroSecreto + 40) % 200 + 1);
    banco->pistas[idx].confiabilidade = 0.09f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 1;
    
    // ===== PISTA 6: FALSA - Agente rival do ciber crime =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_FALSA;
    sprintf(banco->pistas[idx].descricao,
        "AGENTE RIVAL (celula criminosa concorrente): Uma inteligencia de outra organizacao "
        "hackerista nos passou um 'tip' anonimo sugerindo porta %d. Motivacao: sabotagem. "
        "Eles querem que falhemos para assumir o mercado negro.",
        (numeroSecreto - 35 + 200) % 200 + 1);
    banco->pistas[idx].confiabilidade = 0.05f;
    banco->pistas[idx].vinculoNumero = 0;
    banco->pistas[idx].suspeitorId = 2;
    
    // ===== PISTA 7: DIRETA - Validação criptográfica =====
    idx = banco->totalPistas++;
    banco->pistas[idx].id = idx + 1;
    banco->pistas[idx].tipo = PISTA_DIRETA;
    sprintf(banco->pistas[idx].descricao,
        "VALIDACAO CRIPTOGRAFICA: A analise dos algoritmos de encriptacao revela que a porta "
        "%s um quadrado perfeito (requisito do protocolo de seguranca que o ataque explora).",
        ehQuadradoPerfeito(numeroSecreto) ? "E" : "NAO E");
    banco->pistas[idx].confiabilidade = 0.89f;
    banco->pistas[idx].vinculoNumero = 1;
    
    // ===== SUSPEITOS =====
    banco->suspeitos[0].id = 1;
    strcpy(banco->suspeitos[0].nome, "Tecnólogo Cooptado");
    banco->suspeitos[0].reputacao = 0.10f;
    banco->suspeitos[0].forneceuFalsa = 0;
    
    banco->suspeitos[1].id = 2;
    strcpy(banco->suspeitos[1].nome, "Celula Rival Hacker");
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
        banco->pistas[i].casoId = idCaso;
        banco->pistas[i].jaApresentada = 0;
    }
}

void apresentarPista(BancoPistas *banco, int numeroSecreto) {
    (void)numeroSecreto;
    
    // Verificar se já atingiu o máximo de pistas permitidas
    if (banco->pistasColetadas >= banco->maxPistasPermitidas) {
        printf(VERMELHO "\n  PERITO: Ja coletamos pistas suficientes para esta dificuldade (%d/%d).\n" RESET, 
            banco->pistasColetadas, banco->maxPistasPermitidas);
        printf(AMARELO "  Use o seu julgamento investigativo para fazer um palpite!\n\n" RESET);
        return;
    }
    
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
    printf(CIANO "  [Pista %d/%d coletada]\n" RESET, banco->pistasColetadas, banco->maxPistasPermitidas);
}

int verificarMinimoAceitacao(const BancoPistas *banco) {
    return banco->pistasColetadas >= banco->minimoRequired;
}

void exibirHistoricoPistas(const BancoPistas *banco) {
    printf("\n");
    printf(CIANO "  ============ HISTORICO DE PISTAS COLETADAS ==============\n" RESET);
    printf("  Total coletado: %d/%d pistas\n\n", banco->pistasColetadas, banco->maxPistasPermitidas);
    
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
