#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "jogo.h"
#include "historico.h"
#include "utils.h"
#include "tui.h"

#define MAX_PISTAS 4
#define MAX_INTERROGADOS 5
#define MAX_REGISTROS 16
#define MAX_CONSULTAS 2
#define MAX_FEEDBACK 340
#define MAX_POOL_PISTAS 12
#define MAX_HISTORIA 8
#define MAX_CARACTERISTICAS 4
#define REP_ALTA 70
#define REP_BAIXA 30
#define CONFIANCA_ALTA 70
#define CONFIANCA_BAIXA 30
#define CONFIANCA_SCANNER 50

static int clampPercent(int valor) {
    if (valor < 0) {
        return 0;
    }
    if (valor > 100) {
        return 100;
    }
    return valor;
}

int chancePistaFalsaPorReputacao(int reputacao) {
    const int base = 20;
    if (reputacao >= REP_ALTA) {
        return base / 2;
    }
    if (reputacao < REP_BAIXA) {
        return clampPercent((base * 180) / 100);
    }
    return base;
}

int chanceMentiraPorReputacao(int reputacao) {
    if (reputacao >= REP_ALTA) {
        return 10;
    }
    if (reputacao < REP_BAIXA) {
        return 50;
    }
    return 25;
}

int peritoPrecisoPorReputacao(int reputacao) {
    return reputacao >= REP_ALTA;
}

static int calcularTentativasPorConfianca(int confianca, int tentativasBase) {
    if (confianca < CONFIANCA_BAIXA && tentativasBase > 1) {
        return tentativasBase - 1;
    }
    return tentativasBase;
}

typedef enum {
    PROP_PAR = 0,
    PROP_MULT3,
    PROP_MULT5,
    PROP_MULT7,
    PROP_DIGITO_ALTO,
    PROP_SOMA_DIGITOS_PAR,
    PROP_PRIMO
} PropriedadeAlvo;

typedef struct {
    int id;
    const char *nome;
    const char *dificuldade;
    const char *codinome;
    const char *alvo;
    const char *ilustracao[9];
    const char *relatorio[5];
    const char *historiaVitima[MAX_HISTORIA];
    const char *maior;
    const char *menor;
    int maxPistas;
    int min;
    int max;
    int tentativas;
    int bonus;
    int recompensaBase;
} CasoInfo;

typedef struct {
    int valor;
    char leitura[24];
} RegistroPalpite;

typedef struct {
    const char *nome;
    const char *vinculo;
    PropriedadeAlvo propriedade;
    const char *falaTrue;
    const char *falaFalse;
} InterrogadoInfo;

typedef struct {
    PropriedadeAlvo propriedade;
    const char *textoTrue;
    const char *textoFalse;
} PistaInfo;

typedef struct {
    int segundaChance;
    int analiseExtra;
    int scannerForense;
    int intuicao;
    int bonusTentativas;
    int bonusInterrogatorio;
} ItensAtivosCaso;

static const InterrogadoInfo interrogados[][MAX_INTERROGADOS] = {
    {
        {"Mordomo Estevao", "cuidava da biblioteca", PROP_PAR,
         "As marcas de fuligem do teclado batem com teclas de numero par.",
         "As marcas de fuligem do teclado batem com teclas de numero impar."},
        {"Helena Field", "herdeira e ultima visita", PROP_MULT3,
         "Gaga usava codigo divisivel por 3 em noites de contrato.",
         "Gaga evitava codigo divisivel por 3 em noites de contrato."},
        {"Vigia Ramos", "monitorava o corredor", PROP_MULT5,
         "No reflexo da camera, o ultimo toque caiu em tecla 0 ou 5.",
         "No reflexo da camera, o ultimo toque nao caiu em tecla 0 ou 5."},
        {"Jardineiro Otavio", "podava o jardim lateral", PROP_PRIMO,
         "Ele comentou que a vitima preferia numeros primos para o cofre antigo.",
         "Ele comentou que a vitima evitava numeros primos no cofre antigo."},
        {"Contadora Nair", "auditava gastos da familia", PROP_SOMA_DIGITOS_PAR,
         "No caderno de auditoria, a soma dos algarismos da chave era par.",
         "No caderno de auditoria, a soma dos algarismos da chave era impar."}
    },
    {
        {"Dealer Nando", "viu a mesa VIP", PROP_MULT3,
         "A gangue testava canais em saltos de tres.",
         "A gangue testava canais fora dos saltos de tres."},
        {"Cantora Lia", "estava no palco", PROP_DIGITO_ALTO,
         "No retorno do palco, o canal fechava com ultimo digito 5 ou maior.",
         "No retorno do palco, o canal fechava com ultimo digito abaixo de 5."},
        {"Piloto Varela", "aguardava no heliponto", PROP_SOMA_DIGITOS_PAR,
         "As rotas de fuga usavam frequencia com soma de algarismos par.",
         "As rotas de fuga usavam frequencia com soma de algarismos impar."},
        {"Crupie Maia", "trabalhava na roleta", PROP_PAR,
         "A frequencia final parecia numero par.",
         "A frequencia final parecia numero impar."},
        {"Seguranca Tito", "vigiava a doca de saida", PROP_MULT7,
         "Os radios da quadrilha seguiam blocos multiplos de 7.",
         "Os radios da quadrilha nao seguiam blocos multiplos de 7."}
    },
    {
        {"Analista Iara", "revisou os logs", PROP_PRIMO,
         "Nos logs limpos, a porta de comando segue padrao de numero primo.",
         "Nos logs limpos, a porta de comando nao segue padrao de numero primo."},
        {"Operador Breno", "isolou a subestacao", PROP_MULT5,
         "A sonda marcou porta terminada em 0 ou 5.",
         "A sonda descartou porta terminada em 0 ou 5."},
        {"Invasor CH", "suspeito principal", PROP_PAR,
         "Meu payload sempre dorme em porta par.",
         "Meu payload sempre dorme em porta impar."},
        {"Engenheira Celia", "projetou o backbone", PROP_MULT7,
         "A rotina critica reservava portas em blocos multiplos de 7.",
         "A rotina critica evitava portas em blocos multiplos de 7."},
        {"Tecnico Raul", "estava no SOC da madrugada", PROP_SOMA_DIGITOS_PAR,
         "No relatorio final, a soma dos algarismos da porta era par.",
         "No relatorio final, a soma dos algarismos da porta era impar."}
    }
};

static const char *caracteristicasSuspeitos[][MAX_INTERROGADOS][MAX_CARACTERISTICAS] = {
    {
        {
            "Mordomo Estevao",
            "Conhece cada rotina da mansao e o horario de troca de senha.",
            "Nega tocar no teclado sem luvas, mas circulou perto da biblioteca no apagao.",
            "Tem motivo financeiro e acesso fisico privilegiado."
        },
        {
            "Helena Field",
            "Diz que saiu cedo, mas evita detalhar o trajeto completo ate a garagem.",
            "Usa acessorios discretos e alterna pulseira metalica em reunioes de familia.",
            "Tem interesse direto no testamento e no controle das empresas."
        },
        {
            "Vigia Ramos",
            "Opera cameras e preenche registros de horario de forma manual.",
            "Alega microfalhas de energia no exato periodo do acesso ao cofre.",
            "Evita confronto e costuma ajustar relatorios depois do turno."
        },
        {
            "Jardineiro Otavio",
            "Trabalha ha anos na mansao e conhece entradas pouco vigiadas.",
            "Faz poda noturna na ala leste, proxima ao corredor da biblioteca.",
            "Disse ter visto movimentacao nos fundos, mas sem identificar rostos."
        },
        {
            "Contadora Nair",
            "Entrou ha 6 meses e ganhou acesso a contratos sensiveis.",
            "Detectou lancamentos apagados antes do homicidio e guardou copia parcial.",
            "Costuma usar pulseira de relogio de aco e tem atrito com a herdeira pela auditoria."
        }
    },
    {
        {
            "Dealer Nando",
            "Controlava mesa VIP e conhecia codigos internos de emergencia.",
            "Afirma que o plano de fuga ja estava alinhado antes do primeiro disparo.",
            "Possui dividas antigas com apostadores que frequentam a area VIP."
        },
        {
            "Cantora Lia",
            "Estava no palco com retorno de audio aberto durante o caos.",
            "Diz ter ouvido dois codigos de radio e uma contagem curta ao fundo.",
            "Mantem rotina de monitor intra-auricular ligado ate o fim do show."
        },
        {
            "Piloto Varela",
            "Esperava no heliponto sem linha de visao para o salao principal.",
            "Alega ter recebido so um codigo curto para decolagem.",
            "Ja trabalhou com rotas de fuga marcadas por janelas de segundos."
        },
        {
            "Crupie Maia",
            "Ficou na roleta proxima da saida de servico durante a confusao.",
            "Tem memoria forte para padroes numericos e horarios de mesa.",
            "Notou pressa incomum de um seguranca antes da queda de camera."
        },
        {
            "Seguranca Tito",
            "Era responsavel pela camera da doca de saida na janela critica.",
            "Admitiu desligamento de camera por manutencao sem ordem formal.",
            "Conhecia o canal de emergencia do gerente Lucas."
        }
    },
    {
        {
            "Analista Iara",
            "Cuida da triagem de logs e checksums no SOC central.",
            "Insiste que somente o log frio manteve trilha consistente.",
            "Detectou divergencia entre tres relatorios gerados no mesmo minuto."
        },
        {
            "Operador Breno",
            "Executou isolamento da subestacao apos alerta de intrusao.",
            "Diz que seguiu roteiro padrao sem tocar nas regras de firewall.",
            "Tem acesso operacional a portas de contingencia e roteadores."
        },
        {
            "Invasor CH",
            "Suspeito principal, conhecido por blefar em interrogatorio tecnico.",
            "Mistura detalhes reais com provocacao para gerar falso rastro.",
            "Cita termos internos que exigiriam apoio de alguem de dentro."
        },
        {
            "Engenheira Celia",
            "Projetou regras de backbone e politicas de segmentacao.",
            "Alterou regra de firewall sem chamado oficial no turno da crise.",
            "Sustenta que a alteracao foi preventiva e transparente."
        },
        {
            "Tecnico Raul",
            "Estava no plantao do storage na madrugada da intrusao.",
            "Removeu blocos de log alegando pane e corrupcao de disco.",
            "Alega ordem verbal de urgencia sem registro no sistema."
        }
    }
};

static const CasoInfo casos[] = {
    {
        1,
        "O Ultimo Suspiro do Magnata",
        "Facil",
        "COFRE-FANTASMA",
        "codigo do cofre",
        {
            "        ______________________________",
            "       |  BIBLIOTECA FIELD - 23:17   |",
            "       |   ___       ________         |",
            "       |  |   |     |  COFRE |        |",
            "       |  |___|     |  [##]  |        |",
            "       |   livro     |_______|        |",
            "       |      fita UV no teclado      |",
            "       |______________________________|",
            "          calor residual desaparecendo"
        },
        {
            "O bilionario Gaga de Big Field foi encontrado morto na biblioteca.",
            "O cofre esta vazio, mas o ouro ficou para tras.",
            "O invasor levou apenas um pendrive com chaves industriais.",
            "Luz UV revelou calor residual no teclado numerico.",
            "Encontre o codigo antes que a assinatura esfrie."
        },
        {
            "A vitima controlava pessoalmente o cofre e trocava senha toda sexta.",
            "O mordomo discutiu com a vitima no fim da tarde por causa de documentos da heranca.",
            "O jardineiro disse que viu alguem sair apressado do corredor da biblioteca no apagao.",
            "A contadora encontrou lancamentos apagados horas antes do homicidio.",
            "Helena Field afirmou que saiu cedo e nao passou pela biblioteca naquela noite.",
            "No video da porta, uma pessoa para por um segundo e ajusta a gola do casaco antes de digitar.",
            "No espelho do corredor, aparece um brilho metalico no pulso durante o acesso ao cofre.",
            "No livro de ocorrencias, o unico item pessoal marcado como 'esquecido e recuperado' foi um acessorio de pulso."
        },
        "A assinatura termica sobe. O codigo e MAIOR.",
        "O rastro esfriou abaixo desse ponto. O codigo e MENOR.",
        4,
        1, 50, 7, 100, 100
    },
    {
        2,
        "Frequencia de Fuga no Cassino",
        "Medio",
        "RADIO-OURO",
        "frequencia de radio",
        {
            "        ______________________________",
            "       | CASSINO SANTO AMARO - NOITE |",
            "       |  roleta   fichas   palco     |",
            "       |    ( )      []      ||       |",
            "       |        antena quebrada       |",
            "       |  saida fundos -> heliponto   |",
            "       |       sinal curto no radio   |",
            "       |______________________________|",
            "             chiado, sirene, fuga"
        },
        {
            "Uma gangue escapou do cassino com radios criptografados.",
            "O helicoptero de fuga aguarda transmissao curta.",
            "A central interceptou ruido para limitar a faixa.",
            "Cada tentativa de sintonia denuncia sua posicao.",
            "Ache a frequencia exata antes que o sinal suma."
        },
        {
            "A vitima foi o gerente Lucas, morto ao tentar acionar alarme silencioso.",
            "Lucas conhecia o canal de emergencia usado no cassino em dias de risco.",
            "Dealer Nando disse que o piloto ja estava pronto antes do assalto comecar.",
            "Cantora Lia ouviu dois codigos diferentes no retorno do palco.",
            "O piloto disse que recebeu apenas um codigo curto e decolou sem segunda chamada.",
            "Seguranca Tito desligou a camera da doca por 4 minutos sem autorizacao.",
            "No log do interfone da doca ha duas chamadas com 11 segundos de intervalo.",
            "No relatorio de mesa, faltaram 7 fichas pretas exatamente na janela do apagao."
        },
        "A onda portadora esta acima. A frequencia e MAIOR.",
        "Voce passou do pico. A frequencia e MENOR.",
        3,
        1, 100, 6, 200, 150
    },
    {
        3,
        "Protocolo Apocalipse",
        "Dificil",
        "PORTA-ZERO",
        "porta infectada",
        {
            "        ______________________________",
            "       | BACKBONE ESTADUAL - 03:06   |",
            "       |  subestacao A == roteador    |",
            "       |       ||          [X]        |",
            "       |  cidade piscando no mapa     |",
            "       |  logs truncados em cascata   |",
            "       |  terminal: ACESSO NEGADO     |",
            "       |______________________________|",
            "           uma porta mantem o pulso"
        },
        {
            "Um malware entrou na malha eletrica pelo backbone estadual.",
            "O invasor CH do Pina camuflou a porta de comando.",
            "A equipe isolou um intervalo de portas suspeitas.",
            "Cada varredura agressiva consome tempo e aumenta o alerta.",
            "Descubra a porta infectada antes do colapso operacional."
        },
        {
            "A vitima foi o engenheiro chefe Paulo, morto apos denunciar intrusao.",
            "Paulo deixou bilhete: 'confiem apenas no log frio do SOC'.",
            "Analista Iara encontrou 3 relatorios com checksum diferente.",
            "Engenheira Celia alterou regra de firewall sem abrir chamado oficial.",
            "Tecnico Raul apagou registros da madrugada alegando pane no storage.",
            "Invasor CH citou nomenclatura interna de portas que nao aparece em documentacao publica.",
            "Operador Breno registrou isolamento as 03:11, mas o relogio da subestacao congelou em 03:07.",
            "No console externo, um comando interno foi emitido sem passagem de cracha fisico na catraca do SOC."
        },
        "O pacote ecoou mais acima. A porta e MAIOR.",
        "O checksum quebrou abaixo daqui. A porta e MENOR.",
        2,
        1, 200, 5, 350, 220
    }
};

static const PistaInfo pistasCaso1[] = {
    {PROP_PAR, "Pericia UV: o codigo e PAR. Dica: pares terminam em 0, 2, 4, 6 ou 8.",
     "Pericia UV: o codigo e IMPAR. Dica: impares terminam em 1, 3, 5, 7 ou 9."},
    {PROP_MULT3, "No diario da vitima: codigo divisivel por 3. Dica: soma dos algarismos divisivel por 3.",
     "No diario da vitima: codigo nao divisivel por 3. Dica: soma dos algarismos nao fecha multiplo de 3."},
    {PROP_MULT5, "Analise do teclado: codigo divisivel por 5. Dica: termina em 0 ou 5.",
     "Analise do teclado: codigo nao divisivel por 5. Dica: nao termina em 0 nem 5."},
    {PROP_PRIMO, "Relato da secretaria: combinacao prima. Dica: primo tem exatamente 2 divisores.",
     "Relato da secretaria: combinacao nao prima. Dica: tem mais de 2 divisores."},
    {PROP_MULT7, "Anotacao fiscal: codigo em bloco multiplo de 7.",
     "Anotacao fiscal: codigo fora dos blocos multiplos de 7."},
    {PROP_DIGITO_ALTO, "Na fita de seguranca: ultimo digito e 5 ou maior.",
     "Na fita de seguranca: ultimo digito e menor que 5."},
    {PROP_SOMA_DIGITOS_PAR, "No contrato rasgado: soma dos algarismos e PAR.",
     "No contrato rasgado: soma dos algarismos e IMPAR."}
};

static const PistaInfo pistasCaso2[] = {
    {PROP_PAR, "Mesa de som: frequencia final PAR.", "Mesa de som: frequencia final IMPAR."},
    {PROP_MULT3, "Caderno da mesa VIP: frequencia multiplo de 3.",
     "Caderno da mesa VIP: frequencia nao multiplo de 3."},
    {PROP_MULT5, "Ruido do radio: frequencia divisivel por 5.",
     "Ruido do radio: frequencia nao divisivel por 5."},
    {PROP_PRIMO, "Interceptacao externa: frequencia prima.",
     "Interceptacao externa: frequencia nao prima."},
    {PROP_MULT7, "Rota tatica: canal em bloco multiplo de 7.",
     "Rota tatica: canal fora de bloco multiplo de 7."},
    {PROP_DIGITO_ALTO, "Leitura da antena: ultimo digito 5 ou maior.",
     "Leitura da antena: ultimo digito abaixo de 5."},
    {PROP_SOMA_DIGITOS_PAR, "Mixagem forense: soma dos algarismos da frequencia e PAR.",
     "Mixagem forense: soma dos algarismos da frequencia e IMPAR."}
};

static const PistaInfo pistasCaso3[] = {
    {PROP_PRIMO, "SOC limpo: porta infectada prima.", "SOC limpo: porta infectada nao prima."},
    {PROP_PAR, "Telemetria da subestacao: porta PAR.", "Telemetria da subestacao: porta IMPAR."},
    {PROP_MULT5, "Firewall legado: porta divisivel por 5.", "Firewall legado: porta nao divisivel por 5."},
    {PROP_MULT7, "Plano de contingencia: porta multiplo de 7.", "Plano de contingencia: porta nao multiplo de 7."},
    {PROP_SOMA_DIGITOS_PAR, "Relatorio da operadora: soma dos algarismos da porta e PAR.",
     "Relatorio da operadora: soma dos algarismos da porta e IMPAR."},
    {PROP_DIGITO_ALTO, "Sonda de rede: ultimo digito da porta e 5 ou maior.",
     "Sonda de rede: ultimo digito da porta e menor que 5."}
};

static const CasoInfo *obterCaso(int idCaso) {
    for (size_t i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
        if (casos[i].id == idCaso) {
            return &casos[i];
        }
    }
    return &casos[0];
}

static int indiceCaso(int idCaso) {
    for (size_t i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
        if (casos[i].id == idCaso) {
            return (int)i;
        }
    }
    return 0;
}

static void obterPoolPistas(int idCaso, const PistaInfo **pool, int *total) {
    if (idCaso == 1) {
        *pool = pistasCaso1;
        *total = (int)(sizeof(pistasCaso1) / sizeof(pistasCaso1[0]));
        return;
    }
    if (idCaso == 2) {
        *pool = pistasCaso2;
        *total = (int)(sizeof(pistasCaso2) / sizeof(pistasCaso2[0]));
        return;
    }
    *pool = pistasCaso3;
    *total = (int)(sizeof(pistasCaso3) / sizeof(pistasCaso3[0]));
}

static void selecionarPistasAleatorias(int idCaso, int quantidade, int selecionadas[MAX_PISTAS]) {
    const PistaInfo *pool = NULL;
    int total = 0;
    int ordem[MAX_POOL_PISTAS];

    obterPoolPistas(idCaso, &pool, &total);
    if (total > MAX_POOL_PISTAS) {
        total = MAX_POOL_PISTAS;
    }

    for (int i = 0; i < total; i++) {
        ordem[i] = i;
    }

    for (int i = total - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int tmp = ordem[i];
        ordem[i] = ordem[j];
        ordem[j] = tmp;
    }

    for (int i = 0; i < MAX_PISTAS; i++) {
        selecionadas[i] = -1;
    }
    for (int i = 0; i < quantidade && i < total && i < MAX_PISTAS; i++) {
        selecionadas[i] = ordem[i];
    }
}

static void prepararPistasFalsasAtivas(int reputacao, int maxPistas, int pistasFalsas[MAX_PISTAS]) {
    for (int i = 0; i < MAX_PISTAS; i++) {
        pistasFalsas[i] = 0;
    }
    for (int i = 0; i < maxPistas && i < MAX_PISTAS; i++) {
        pistasFalsas[i] = ((rand() % 100) < chancePistaFalsaPorReputacao(reputacao));
    }
}

static int removerUmaPistaFalsaAtiva(int maxPistas, int pistasFalsas[MAX_PISTAS]) {
    for (int i = 0; i < maxPistas && i < MAX_PISTAS; i++) {
        if (pistasFalsas[i]) {
            pistasFalsas[i] = 0;
            return 1;
        }
    }
    return 0;
}

static void prepararItensAtivosCaso(ItensAtivosCaso *itens, int confiancaAtual) {
    int opcao;
    memset(itens, 0, sizeof(*itens));

    limparTela();
    printf("\n");
    uiBanner("LOADOUT PRE-CASO", "Ative itens estrategicos");
    uiSection("INVENTARIO", UI_CYAN);
    uiBoxTop();
    {
        char linha[160];
        snprintf(linha, sizeof(linha), "Segunda Chance: %d", getQuantidadeItem(ITEM_SEGUNDA_CHANCE));
        uiBoxWrap(linha, UI_WHITE);
        snprintf(linha, sizeof(linha), "Analise Extra: %d", getQuantidadeItem(ITEM_ANALISE_EXTRA));
        uiBoxWrap(linha, UI_WHITE);
        snprintf(linha, sizeof(linha), "Scanner Forense: %d", getQuantidadeItem(ITEM_SCANNER_FORENSE));
        uiBoxWrap(linha, UI_WHITE);
        snprintf(linha, sizeof(linha), "Intuicao: %d", getQuantidadeItem(ITEM_INTUICAO));
        uiBoxWrap(linha, UI_WHITE);
        snprintf(linha, sizeof(linha), "+2 Tentativas: %d", getQuantidadeItem(ITEM_MAIS_2_TENTATIVAS));
        uiBoxWrap(linha, UI_WHITE);
        snprintf(linha, sizeof(linha), "+1 Interrogatorio: %d", getQuantidadeItem(ITEM_MAIS_1_INTERROGATORIO));
        uiBoxWrap(linha, UI_WHITE);
    }
    uiBoxBottom();

    if (getQuantidadeItem(ITEM_SEGUNDA_CHANCE) > 0) {
        uiPrompt("Ativar Segunda Chance? [1=sim,2=nao]");
        opcao = lerOpcao(1, 2);
        if (opcao == 1 && consumirItem(ITEM_SEGUNDA_CHANCE)) {
            itens->segundaChance = 1;
        }
    }
    if (getQuantidadeItem(ITEM_ANALISE_EXTRA) > 0) {
        uiPrompt("Ativar Analise Extra? [1=sim,2=nao]");
        opcao = lerOpcao(1, 2);
        if (opcao == 1 && consumirItem(ITEM_ANALISE_EXTRA)) {
            itens->analiseExtra = 1;
        }
    }
    if (getQuantidadeItem(ITEM_SCANNER_FORENSE) > 0) {
        if (confiancaAtual >= CONFIANCA_SCANNER) {
            uiPrompt("Ativar Scanner Forense? [1=sim,2=nao]");
            opcao = lerOpcao(1, 2);
            if (opcao == 1 && consumirItem(ITEM_SCANNER_FORENSE)) {
                itens->scannerForense = 1;
            }
        } else {
            uiAlert("BLOQUEADO", "Scanner Forense exige confianca >= 50.", UI_YELLOW);
        }
    }
    if (getQuantidadeItem(ITEM_INTUICAO) > 0) {
        uiPrompt("Ativar Intuicao? [1=sim,2=nao]");
        opcao = lerOpcao(1, 2);
        if (opcao == 1 && consumirItem(ITEM_INTUICAO)) {
            itens->intuicao = 1;
        }
    }
    if (getQuantidadeItem(ITEM_MAIS_2_TENTATIVAS) > 0) {
        uiPrompt("Ativar +2 Tentativas? [1=sim,2=nao]");
        opcao = lerOpcao(1, 2);
        if (opcao == 1 && consumirItem(ITEM_MAIS_2_TENTATIVAS)) {
            itens->bonusTentativas = 1;
        }
    }
    if (getQuantidadeItem(ITEM_MAIS_1_INTERROGATORIO) > 0) {
        uiPrompt("Ativar +1 Interrogatorio? [1=sim,2=nao]");
        opcao = lerOpcao(1, 2);
        if (opcao == 1 && consumirItem(ITEM_MAIS_1_INTERROGATORIO)) {
            itens->bonusInterrogatorio = 1;
        }
    }
}

static void exibirIlustracaoCena(const CasoInfo *caso) {
    uiSection("ILUSTRACAO DA CENA", UI_CYAN);
    uiBoxTop();
    for (int i = 0; i < 9; i++) {
        uiBoxText(caso->ilustracao[i]);
        uiSleepMs(85);
    }
    uiBoxBottom();
    uiLoading("Renderizando quadro forense", 10, 14);
}

static int somaDigitos(int valor) {
    int soma = 0;
    int numero = valor < 0 ? -valor : valor;

    while (numero > 0) {
        soma += numero % 10;
        numero /= 10;
    }
    return soma;
}

static int ehPrimo(int valor) {
    if (valor < 2) {
        return 0;
    }
    for (int i = 2; i * i <= valor; i++) {
        if (valor % i == 0) {
            return 0;
        }
    }
    return 1;
}

static int segredoPossui(int secreto, PropriedadeAlvo propriedade) {
    switch (propriedade) {
        case PROP_PAR:
            return secreto % 2 == 0;
        case PROP_MULT3:
            return secreto % 3 == 0;
        case PROP_MULT5:
            return secreto % 5 == 0;
        case PROP_MULT7:
            return secreto % 7 == 0;
        case PROP_DIGITO_ALTO:
            return (secreto % 10) >= 5;
        case PROP_SOMA_DIGITOS_PAR:
            return somaDigitos(secreto) % 2 == 0;
        case PROP_PRIMO:
            return ehPrimo(secreto);
    }
    return 0;
}

static const char *calorDaDistancia(int distancia, int max) {
    if (distancia == 0) {
        return "ALVO CONFIRMADO";
    }
    if (distancia <= max / 20 + 1) {
        return "QUENTE";
    }
    if (distancia <= max / 8 + 1) {
        return "MORNO";
    }
    if (distancia <= max / 4 + 1) {
        return "FRIO";
    }
    return "GELADO";
}

static int calcularPontos(const CasoInfo *caso, int tentativasRestantes, int pistasUsadas,
                           int interrogatoriosUsados, int penalidadesRitmo, int venceu) {
    int pontos;

    if (!venceu) {
        return caso->bonus / 5;
    }

    pontos = caso->bonus + (tentativasRestantes * 120) - (pistasUsadas * 45)
             - (interrogatoriosUsados * 30) - (penalidadesRitmo * 40);

    if (pontos < caso->bonus / 4) {
        pontos = caso->bonus / 4;
    }
    return pontos;
}

static int percentualRecompensa(int venceu, int tentativasUsadas) {
    if (!venceu) {
        return 15;
    }
    if (tentativasUsadas <= 1) {
        return 100;
    }
    if (tentativasUsadas == 2) {
        return 65;
    }
    return 40;
}

static int calcularRecompensaMoedas(const CasoInfo *caso, int venceu, int tentativasUsadas,
                                      int *percentual) {
    int pct = percentualRecompensa(venceu, tentativasUsadas);
    if (percentual != NULL) {
        *percentual = pct;
    }
    return (caso->recompensaBase * pct) / 100;
}

static const char *rankInvestigativo(int pontos, int venceu) {
    if (!venceu) {
        return "Arquivo inconclusivo";
    }
    if (pontos >= 950) {
        return "Lenda da Pericia";
    }
    if (pontos >= 650) {
        return "Detetive Cirurgico";
    }
    if (pontos >= 420) {
        return "Investigador Solido";
    }
    return "Aprendiz de Campo";
}

static void exibirRelatorio(const CasoInfo *caso) {
    limparTela();
    printf("\n");

    uiBanner("PREPARANDO INVESTIGACAO", caso->codinome);
    uiStamp(caso->nome, "TRANSICAO TATICA", UI_DIM);
    uiLoading("Carregando mapa de evidencias", 20, 18);
    uiLoading("Calibrando sensores forenses", 20, 18);
    uiAlert("PRONTO", "O terminal forense esta em modo ativo.", UI_GREEN);
    uiPause("Pressione ENTER para comecar...");
}

static void exibirTutorialBasico(const CasoInfo *caso) {
    char linha[120];

    limparTela();
    printf("\n");
    uiBanner("GUIA RAPIDO DE INVESTIGACAO", caso->nome);
    uiStamp(caso->codinome, "LEIA ANTES DE COMECAR", UI_DIM);

    uiSection("OBJETIVO", UI_CYAN);
    uiBoxTop();
    snprintf(linha, sizeof(linha), "Descobrir o %s correto na faixa %d a %d.",
             caso->alvo, caso->min, caso->max);
    uiBoxWrap(linha, UI_WHITE);
    uiBoxWrap("Voce vence ao acertar o valor secreto antes das tentativas acabarem.", UI_WHITE);
    uiBoxBottom();

    uiSection("COMANDOS", UI_YELLOW);
    uiBoxTop();
    uiBoxWrap("Digite um numero da faixa para tentar resolver o caso.", UI_WHITE);
    uiBoxWrap("Digite 0 para pedir pista tecnica (consome pista de campo).", UI_WHITE);
    uiBoxWrap("Digite -1 para interrogar um suspeito (maximo de 2 consultas).", UI_WHITE);
    uiBoxWrap("Digite -2 para revisar pistas e depoimentos ja coletados.", UI_WHITE);
    uiBoxWrap("Digite -3 para abrir o dossie completo da vitima e suspeitos.", UI_WHITE);
    uiBoxBottom();

    uiSection("COMO LER O FEEDBACK", UI_MAGENTA);
    uiBoxTop();
    uiBoxWrap("MAIOR: o alvo secreto esta acima do numero que voce testou.", UI_WHITE);
    uiBoxWrap("MENOR: o alvo secreto esta abaixo do numero que voce testou.", UI_WHITE);
    uiBoxWrap("Leituras GELADO/FRIO/MORNO/QUENTE indicam o quao perto voce esta.", UI_WHITE);
    uiBoxBottom();

    uiSection("REGRAS IMPORTANTES", UI_RED);
    uiBoxTop();
    snprintf(linha, sizeof(linha), "Cada caso tem limite de tentativas (%d neste caso).", caso->tentativas);
    uiBoxWrap(linha, UI_WHITE);
    uiBoxWrap("Entradas rapidas demais ativam anti-spam e podem consumir tentativa.", UI_WHITE);
    uiBoxWrap("Nem todo depoimento e verdadeiro: compare pistas antes de confiar.", UI_WHITE);
    uiBoxBottom();

    uiPause("Pressione ENTER para iniciar a investigacao com este guia em mente...");
}

static void exibirLog(const RegistroPalpite *registros, int total) {
    uiSection("LOG DE VARREDURAS", UI_CYAN);
    if (total == 0) {
        uiAlert("LOG", "Nenhuma tentativa registrada ainda.", UI_DIM);
        return;
    }

    uiBoxTop();
    uiBoxText("N.  Teste  Leitura");
    for (int i = 0; i < total; i++) {
        char linha[80];
        snprintf(linha, sizeof(linha), "%02d  %-5d  %s",
                 i + 1, registros[i].valor, registros[i].leitura);
        uiBoxText(linha);
    }
    uiBoxBottom();
}

static void exibirHistoriaVitima(const CasoInfo *caso, int idxCaso) {
    limparTela();
    printf("\n");
    uiBanner("DOSSIE DA VITIMA", caso->nome);
    uiStamp(caso->codinome, "HISTORIA E PONTOS SUSPEITOS", UI_DIM);
    uiAlert("AVISO", "Atencao: neste caso, um ou mais suspeitos podem fornecer pistas falsas.", UI_YELLOW);
    uiSection("HISTORIA", UI_CYAN);
    uiBoxTop();
    for (int i = 0; i < MAX_HISTORIA; i++) {
        uiBoxWrap(caso->historiaVitima[i], UI_WHITE);
    }
    uiBoxBottom();

    uiSection("CARACTERISTICAS DOS SUSPEITOS", UI_MAGENTA);
    uiBoxTop();
    for (int i = 0; i < MAX_INTERROGADOS; i++) {
        char linha[360];
        snprintf(linha, sizeof(linha), "%d) %s", i + 1, caracteristicasSuspeitos[idxCaso][i][0]);
        uiBoxWrap(linha, UI_WHITE);
        snprintf(linha, sizeof(linha), "   - %s", caracteristicasSuspeitos[idxCaso][i][1]);
        uiBoxWrap(linha, UI_DIM);
        snprintf(linha, sizeof(linha), "   - %s", caracteristicasSuspeitos[idxCaso][i][2]);
        uiBoxWrap(linha, UI_DIM);
        snprintf(linha, sizeof(linha), "   - %s", caracteristicasSuspeitos[idxCaso][i][3]);
        uiBoxWrap(linha, UI_DIM);
        if (i < MAX_INTERROGADOS - 1) {
            uiBoxWrap(" ", UI_DIM);
        }
    }
    uiBoxBottom();

    uiPause("Pressione ENTER para voltar a investigacao...");
}

static void aplicarPista(char *feedback, size_t tamanho, const CasoInfo *caso, int pistaNumero,
                          int secreto, const int pistasSelecionadas[MAX_PISTAS], int reputacao,
                          const int pistasFalsas[MAX_PISTAS], int forcarVerdade) {
    const PistaInfo *pool = NULL;
    int total = 0;
    int idxPool;
    const PistaInfo *pista;

    if (pistaNumero < 1 || pistaNumero > caso->maxPistas) {
        snprintf(feedback, tamanho, "PERITO: Sem novas pistas no momento.");
        return;
    }

    obterPoolPistas(caso->id, &pool, &total);
    idxPool = pistasSelecionadas[pistaNumero - 1];
    if (idxPool < 0 || idxPool >= total) {
        snprintf(feedback, tamanho, "PERITO: A trilha da evidencia esta incompleta.");
        return;
    }

    pista = &pool[idxPool];
    {
        int verdade = segredoPossui(secreto, pista->propriedade);
        int pistaFalsa = 0;
        if (!forcarVerdade) {
            pistaFalsa = pistasFalsas[pistaNumero - 1];
        }
        const char *texto;
        if (pistaFalsa) {
            verdade = !verdade;
        }
        texto = verdade ? pista->textoTrue : pista->textoFalse;
        if (peritoPrecisoPorReputacao(reputacao)) {
            snprintf(feedback, tamanho, "PERITO (alta precisao): %s", texto);
        } else if (reputacao < REP_BAIXA) {
            snprintf(feedback, tamanho, "PERITO (leitura parcial): %s", texto);
        } else {
            snprintf(feedback, tamanho, "PERITO: %s", texto);
        }
    }
}

static void exibirArquivoEvidencias(const CasoInfo *caso, int pistasUsadas,
                                      char pistasRegistradas[MAX_PISTAS][MAX_FEEDBACK],
                                      char depoimentos[MAX_CONSULTAS][MAX_FEEDBACK],
                                      int totalDepoimentos) {
    limparTela();
    printf("\n");
    uiBanner("ARQUIVO DE EVIDENCIAS", caso->nome);
    uiStamp(caso->codinome, "REVISAO TATICA", UI_DIM);

    uiSection("PISTAS DESBLOQUEADAS", UI_CYAN);
    if (pistasUsadas == 0) {
        uiAlert("PISTAS", "Nenhuma pista coletada ate agora.", UI_DIM);
    } else {
        uiBoxTop();
        for (int i = 0; i < pistasUsadas && i < MAX_PISTAS; i++) {
            char linha[340];
            snprintf(linha, sizeof(linha), "%d) %s", i + 1, pistasRegistradas[i]);
            uiBoxWrap(linha, UI_YELLOW);
        }
        uiBoxBottom();
    }

    uiSection("INTERROGATORIOS REGISTRADOS", UI_MAGENTA);
    if (totalDepoimentos == 0) {
        uiAlert("DEPOIMENTO", "Nenhum interrogado consultado ate agora.", UI_DIM);
    } else {
        uiBoxTop();
        for (int i = 0; i < totalDepoimentos; i++) {
            char linha[340];
            snprintf(linha, sizeof(linha), "%d) %s", i + 1, depoimentos[i]);
            uiBoxWrap(linha, UI_WHITE);
        }
        uiBoxBottom();
    }

    uiPause("Pressione ENTER para voltar a investigacao...");
}

static void consultarInterrogado(char *feedback, size_t tamanho, const CasoInfo *caso, int secreto,
                                  int idxCaso, int ouvidos[MAX_INTERROGADOS], int *consultasUsadas,
                                  char depoimentos[MAX_CONSULTAS][MAX_FEEDBACK], int *totalDepoimentos,
                                  int reputacao, int *deltaReputacao) {
    int opcao;
    const InterrogadoInfo *escolhido;
    int propriedadeReal;
    int afirmacao;
    int mente;
    (void)caso;

    uiSection("REDE DE INTERROGADOS", UI_MAGENTA);
    uiBoxTop();
    for (int i = 0; i < MAX_INTERROGADOS; i++) {
        char linha[220];
        snprintf(linha, sizeof(linha), "%d) %s - %s %s",
                 i + 1,
                 interrogados[idxCaso][i].nome,
                 interrogados[idxCaso][i].vinculo,
                 ouvidos[i] ? "[JA CONSULTADO]" : "");
        uiBoxWrap(linha, UI_WHITE);
    }
    uiBoxBottom();
    uiPrompt("INTERROGADO");

    opcao = lerOpcao(1, MAX_INTERROGADOS);
    if (ouvidos[opcao - 1]) {
        snprintf(feedback, tamanho, "CENTRAL: %s ja foi ouvido. Escolha outro contato.",
                 interrogados[idxCaso][opcao - 1].nome);
        return;
    }

    ouvidos[opcao - 1] = 1;
    (*consultasUsadas)++;

    escolhido = &interrogados[idxCaso][opcao - 1];
    propriedadeReal = segredoPossui(secreto, escolhido->propriedade);
    mente = (rand() % 100) < chanceMentiraPorReputacao(reputacao);
    afirmacao = mente ? !propriedadeReal : propriedadeReal;
    if (mente) {
        *deltaReputacao -= 5;
    }

    snprintf(feedback, tamanho, "DEPOIMENTO (%s): %s",
             escolhido->nome, afirmacao ? escolhido->falaTrue : escolhido->falaFalse);

    if (*totalDepoimentos < MAX_CONSULTAS) {
        strncpy(depoimentos[*totalDepoimentos], feedback, MAX_FEEDBACK - 1);
        depoimentos[*totalDepoimentos][MAX_FEEDBACK - 1] = '\0';
        (*totalDepoimentos)++;
    }
}

int confirmarCaso(int idCaso) {
    const CasoInfo *caso = obterCaso(idCaso);
    char valor[40];

    limparTela();
    printf("\n");
    uiBanner("SELECAO DE CASO", caso->nome);
    uiStamp(caso->codinome, "CONFIRMACAO TATICA", UI_DIM);
    uiSection("RESUMO OPERACIONAL", UI_YELLOW);
    uiBoxTop();
    uiBoxMid("Dificuldade", caso->dificuldade, UI_YELLOW);
    uiBoxMid("Alvo", caso->alvo, UI_CYAN);
    snprintf(valor, sizeof(valor), "%d a %d", caso->min, caso->max);
    uiBoxMid("Faixa", valor, UI_WHITE);
    snprintf(valor, sizeof(valor), "%d varreduras", caso->tentativas);
    uiBoxMid("Tentativas", valor, UI_GREEN);
    snprintf(valor, sizeof(valor), "%d pistas", caso->maxPistas);
    uiBoxMid("Pistas de campo", valor, UI_CYAN);
    uiBoxMid("Interrogatorios", "maximo de 2 consultas", UI_MAGENTA);
    snprintf(valor, sizeof(valor), "%d moedas", caso->recompensaBase);
    uiBoxMid("Recompensa maxima", valor, UI_GREEN);
    uiBoxBottom();

    exibirIlustracaoCena(caso);

    uiSection("COMANDO", UI_MAGENTA);
    printf("  %s[1]%s Iniciar investigacao\n", UI_GREEN, UI_RESET);
    printf("  %s[2]%s Voltar ao mural de casos\n", UI_YELLOW, UI_RESET);
    uiPrompt("ACAO");

    return lerOpcao(1, 2) == 1;
}

void jogarPartida(int idCaso) {
    const CasoInfo *caso = obterCaso(idCaso);
    int idxCaso = indiceCaso(idCaso);
    int pistasSorteadas[MAX_PISTAS];
    int pistasFalsasAtivas[MAX_PISTAS];
    int secreto = caso->min + (rand() % (caso->max - caso->min + 1));
    int confiancaAtual = getConfiancaDelegacia();
    int tentativas = calcularTentativasPorConfianca(confiancaAtual, caso->tentativas);
    int maxInterrogatorios = MAX_CONSULTAS;
    int segundaChanceDisponivel = 0;
    int pistasUsadas = 0;
    int interrogatoriosUsados = 0;
    int penalidadesRitmo = 0;
    int interrogadosOuvidos[MAX_INTERROGADOS] = {0};
    int sequenciaRapida = 0;
    time_t ultimoPalpite = time(NULL) - 3;
    char pistasRegistradas[MAX_PISTAS][MAX_FEEDBACK] = {{0}};
    char depoimentos[MAX_CONSULTAS][MAX_FEEDBACK] = {{0}};
    int totalDepoimentos = 0;
    RegistroPalpite registros[MAX_REGISTROS];
    int contPalpites = 0;
    char feedback[MAX_FEEDBACK] = "Central online. Cruze pistas e depoimentos antes de arriscar.";
    int reputacaoAtual = getScore();
    int deltaReputacao = 0;
    ItensAtivosCaso itens;
    Sessao s;

    prepararItensAtivosCaso(&itens, confiancaAtual);
    if (itens.bonusTentativas) {
        tentativas += 2;
    }
    if (itens.bonusInterrogatorio) {
        maxInterrogatorios += 1;
    }
    segundaChanceDisponivel = itens.segundaChance;

    exibirRelatorio(caso);
    exibirTutorialBasico(caso);

    strncpy(s.casoNome, caso->nome, sizeof(s.casoNome) - 1);
    s.casoNome[sizeof(s.casoNome) - 1] = '\0';
    strncpy(s.dificuldade, caso->dificuldade, sizeof(s.dificuldade) - 1);
    s.dificuldade[sizeof(s.dificuldade) - 1] = '\0';
    s.secreto = secreto;
    s.venceu = 0;

    selecionarPistasAleatorias(caso->id, caso->maxPistas, pistasSorteadas);
    prepararPistasFalsasAtivas(reputacaoAtual, caso->maxPistas, pistasFalsasAtivas);
    if (itens.scannerForense) {
        if (removerUmaPistaFalsaAtiva(caso->maxPistas, pistasFalsasAtivas)) {
            strcpy(feedback, "SCANNER FORENSE: uma pista falsa foi removida do conjunto ativo.");
        } else {
            strcpy(feedback, "SCANNER FORENSE: nao havia pista falsa ativa para remover.");
        }
    }
    if (itens.intuicao) {
        char intuicaoLinha[120];
        int minInt = secreto - 10;
        int maxInt = secreto + 10;
        if (minInt < caso->min) {
            minInt = caso->min;
        }
        if (maxInt > caso->max) {
            maxInt = caso->max;
        }
        snprintf(intuicaoLinha, sizeof(intuicaoLinha),
                 "INTUICAO: alvo entre %d e %d.", minInt, maxInt);
        strncpy(feedback, intuicaoLinha, sizeof(feedback) - 1);
        feedback[sizeof(feedback) - 1] = '\0';
    }
    if (itens.analiseExtra && pistasUsadas < caso->maxPistas) {
        pistasUsadas++;
        aplicarPista(feedback, sizeof(feedback), caso, pistasUsadas, secreto, pistasSorteadas, reputacaoAtual,
                     pistasFalsasAtivas, 1);
        strncpy(pistasRegistradas[pistasUsadas - 1], feedback, MAX_FEEDBACK - 1);
        pistasRegistradas[pistasUsadas - 1][MAX_FEEDBACK - 1] = '\0';
    }

    while (tentativas > 0) {
        int palpite;

        limparTela();
        printf("\n");
        uiBanner("INVESTIGACAO ATIVA", caso->nome);
        uiStamp(caso->codinome, tentativas <= 2 ? "RISCO ALTO" : "SISTEMA ESTAVEL",
                 tentativas <= 2 ? UI_RED : UI_GREEN);
        uiSection("PAINEL DE CONTROLE", UI_CYAN);
        uiMeter("Integridade", tentativas, tentativas > caso->tentativas ? tentativas : caso->tentativas,
                tentativas <= 2 ? UI_RED : UI_GREEN);
        uiMeter("Pistas de campo", caso->maxPistas - pistasUsadas, caso->maxPistas,
                 pistasUsadas == caso->maxPistas ? UI_RED : UI_CYAN);
        uiMeter("Interrogatorios", maxInterrogatorios - interrogatoriosUsados, maxInterrogatorios,
                 interrogatoriosUsados == maxInterrogatorios ? UI_RED : UI_MAGENTA);
        uiMeter("Reputacao", reputacaoAtual, 100,
                 reputacaoAtual >= REP_ALTA ? UI_GREEN : (reputacaoAtual < REP_BAIXA ? UI_RED : UI_YELLOW));
        uiMeter("Confianca", confiancaAtual, 100,
                 confiancaAtual >= CONFIANCA_ALTA ? UI_GREEN : (confiancaAtual < CONFIANCA_BAIXA ? UI_RED : UI_YELLOW));

        uiSection("LEITURA ATUAL", UI_YELLOW);
        uiBoxTop();
        uiBoxMid("Alvo", caso->alvo, UI_CYAN);
        char faixa[40];
        snprintf(faixa, sizeof(faixa), "%d a %d", caso->min, caso->max);
        uiBoxMid("Faixa valida", faixa, UI_WHITE);
        {
            char reputacaoLinha[48];
            snprintf(reputacaoLinha, sizeof(reputacaoLinha), "%d [%+d]", reputacaoAtual, deltaReputacao);
            uiBoxMid("Reputacao", reputacaoLinha,
                     reputacaoAtual >= REP_ALTA ? UI_GREEN : (reputacaoAtual < REP_BAIXA ? UI_RED : UI_YELLOW));
        }
        {
            char confLinha[48];
            snprintf(confLinha, sizeof(confLinha), "%d", confiancaAtual);
            uiBoxMid("Confianca", confLinha,
                     confiancaAtual >= CONFIANCA_ALTA ? UI_GREEN : (confiancaAtual < CONFIANCA_BAIXA ? UI_RED : UI_YELLOW));
        }
        uiBoxMid("Feedback", "analise abaixo", UI_YELLOW);
        uiBoxWrap(feedback, UI_YELLOW);
        uiBoxBottom();

        exibirLog(registros, contPalpites);

        uiSection("COMANDO DO DETETIVE", UI_MAGENTA);
        printf("  %sNumero valido: %d a %d | 0 pista | -1 interrogado | -2 revisar evidencias | -3 dossie da vitima.%s\n",
               UI_DIM, caso->min, caso->max, UI_RESET);
        uiPrompt("ALVO");

        palpite = lerOpcao(-3, caso->max);

        if (palpite == -3) {
            exibirHistoriaVitima(caso, idxCaso);
            continue;
        }

        if (palpite == -2) {
            exibirArquivoEvidencias(caso, pistasUsadas, pistasRegistradas,
                                      depoimentos, totalDepoimentos);
            continue;
        }

        if (palpite == -1) {
            if (interrogatoriosUsados < maxInterrogatorios) {
                int deltaAntes = deltaReputacao;
                consultarInterrogado(feedback, sizeof(feedback), caso, secreto, idxCaso,
                                      interrogadosOuvidos, &interrogatoriosUsados,
                                      depoimentos, &totalDepoimentos, reputacaoAtual, &deltaReputacao);
                if (deltaReputacao < deltaAntes) {
                    decrementar(deltaAntes - deltaReputacao);
                }
            } else {
                strcpy(feedback, "CENTRAL: Limite de interrogatorios atingido.");
            }
            reputacaoAtual = getScore();
            continue;
        }

        if (palpite == 0) {
            if (pistasUsadas < caso->maxPistas) {
                pistasUsadas++;
                aplicarPista(feedback, sizeof(feedback), caso, pistasUsadas, secreto, pistasSorteadas, reputacaoAtual,
                             pistasFalsasAtivas, 0);
                strncpy(pistasRegistradas[pistasUsadas - 1], feedback, MAX_FEEDBACK - 1);
                pistasRegistradas[pistasUsadas - 1][MAX_FEEDBACK - 1] = '\0';
            } else {
                strcpy(feedback, "PERITO: Nao ha novas pistas de campo disponiveis.");
            }
            reputacaoAtual = getScore();
            continue;
        }

        if (palpite < caso->min) {
            snprintf(feedback, sizeof(feedback), "CENTRAL: valor fora da faixa valida (%d a %d).",
                     caso->min, caso->max);
            continue;
        }

        time_t agora = time(NULL);
        if (difftime(agora, ultimoPalpite) <= 1.0) {
            sequenciaRapida++;
        } else {
            sequenciaRapida = 0;
        }
        ultimoPalpite = agora;

        if (sequenciaRapida >= 2) {
            tentativas--;
            penalidadesRitmo++;
            snprintf(feedback, sizeof(feedback),
                     "CENTRAL: excesso de entradas rapidas detectado. Uma varredura foi perdida.");
            uiLoading("Estabilizando terminal anti-spam", 12, 18);
            uiAlert("PROTOCOLO", feedback, UI_RED);
            pausar();
            continue;
        }

        if (contPalpites < MAX_REGISTROS) {
            registros[contPalpites].valor = palpite;
            strncpy(registros[contPalpites].leitura,
                    calorDaDistancia(abs(palpite - secreto), caso->max),
                    sizeof(registros[contPalpites].leitura) - 1);
            registros[contPalpites].leitura[sizeof(registros[contPalpites].leitura) - 1] = '\0';
            contPalpites++;
        }

        if (palpite == secreto) {
            s.venceu = 1;
            break;
        }

        if (palpite < secreto) {
            snprintf(feedback, sizeof(feedback), "%s Leitura: %s.",
                     caso->maior, registros[contPalpites - 1].leitura);
        } else {
            snprintf(feedback, sizeof(feedback), "%s Leitura: %s.",
                     caso->menor, registros[contPalpites - 1].leitura);
        }
        if (segundaChanceDisponivel) {
            segundaChanceDisponivel = 0;
            snprintf(feedback, sizeof(feedback),
                     "SEGUNDA CHANCE: tentativa preservada. Ajuste a estrategia e tente de novo.");
        } else {
            tentativas--;
            deltaReputacao -= 2;
            decrementar(2);
            reputacaoAtual = getScore();
        }

        printf("\n");
        uiLoading("Processando leitura", 14, 14);
        uiScanBar(palpite, caso->min, caso->max);
        uiAlert("SISTEMA", feedback, palpite < secreto ? UI_CYAN : UI_YELLOW);
        pausar();
    }

    s.tentativasUsadas = contPalpites;
    if (s.venceu) {
        deltaReputacao += 10;
    } else {
        deltaReputacao -= 8;
    }

    if (s.venceu) {
        incrementar(10);
    } else {
        decrementar(8);
    }
    if (s.venceu && s.tentativasUsadas <= 2) {
        aumentarConfianca(6);
    } else if (s.venceu) {
        aumentarConfianca(3);
    } else {
        reduzirConfianca(6);
    }
    reputacaoAtual = getScore();
    confiancaAtual = getConfiancaDelegacia();

    limparTela();
    int pontos = calcularPontos(caso, tentativas, pistasUsadas, interrogatoriosUsados,
                                 penalidadesRitmo, s.venceu);
    int saldoAntes = getSaldo();
    int multiplicadorPct = 0;
    int recompensa = calcularRecompensaMoedas(caso, s.venceu, s.tentativasUsadas, &multiplicadorPct);
    int saldoDepois;

    printf("\n");
    uiBanner("RESULTADO DA INVESTIGACAO", caso->nome);
    uiStamp("RELATORIO FINAL", s.venceu ? "CASO RESOLVIDO" : "ARQUIVO FRIO",
             s.venceu ? UI_GREEN : UI_RED);

    if (s.venceu == 1) {
        uiAlert("SUCESSO", "Caso resolvido com sucesso.", UI_GREEN);
        printf("  O alvo %d foi confirmado. A cadeia de evidencias foi preservada.\n", secreto);
    } else {
        uiAlert("FALHA", "A investigacao foi encerrada sem confirmacao.", UI_RED);
        printf("  O sistema travou. O alvo verdadeiro era %d.\n", secreto);
    }

    uiSection("DESEMPENHO", UI_CYAN);
    uiBoxTop();
    char valor[64];
    snprintf(valor, sizeof(valor), "%d de %d", s.tentativasUsadas, caso->tentativas);
    uiBoxMid("Tentativas", valor, UI_WHITE);
    snprintf(valor, sizeof(valor), "%d", pistasUsadas);
    uiBoxMid("Pistas usadas", valor, pistasUsadas > 0 ? UI_YELLOW : UI_GREEN);
    snprintf(valor, sizeof(valor), "%d", interrogatoriosUsados);
    uiBoxMid("Interrogatorios", valor, interrogatoriosUsados > 0 ? UI_MAGENTA : UI_GREEN);
    snprintf(valor, sizeof(valor), "%d", penalidadesRitmo);
    uiBoxMid("Penalidades anti-spam", valor, penalidadesRitmo > 0 ? UI_RED : UI_GREEN);
    snprintf(valor, sizeof(valor), "%d", pontos);
    uiBoxMid("Pontuacao", valor, UI_CYAN);
    uiBoxMid("Rank", rankInvestigativo(pontos, s.venceu), UI_MAGENTA);
    snprintf(valor, sizeof(valor), "%d [%+d]", reputacaoAtual, deltaReputacao);
    uiBoxMid("Reputacao final", valor, reputacaoAtual >= REP_ALTA ? UI_GREEN : (reputacaoAtual < REP_BAIXA ? UI_RED : UI_YELLOW));
    snprintf(valor, sizeof(valor), "%d", confiancaAtual);
    uiBoxMid("Confianca final", valor,
             confiancaAtual >= CONFIANCA_ALTA ? UI_GREEN : (confiancaAtual < CONFIANCA_BAIXA ? UI_RED : UI_YELLOW));
    uiBoxBottom();

    creditar(recompensa);
    saldoDepois = getSaldo();

    uiSection("RECOMPENSA", UI_YELLOW);
    uiBoxTop();
    snprintf(valor, sizeof(valor), "%d moedas", caso->recompensaBase);
    uiBoxMid("Recompensa base", valor, UI_WHITE);
    snprintf(valor, sizeof(valor), "%d%%", multiplicadorPct);
    uiBoxMid("Multiplicador", valor, UI_CYAN);
    snprintf(valor, sizeof(valor), "%d moedas", recompensa);
    uiBoxMid("Valor final", valor, UI_GREEN);
    snprintf(valor, sizeof(valor), "%d -> %d", saldoAntes, saldoDepois);
    uiBoxMid("Saldo", valor, UI_MAGENTA);
    uiBoxBottom();

    exibirLog(registros, contPalpites);

    printf("\n");
    uiAlert("ARQUIVO", "Registro salvo no banco de dados do departamento.", UI_GREEN);

    salvarSessao(s);
    pausar();
}
