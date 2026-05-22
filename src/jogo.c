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
    const char *historia_vitima[MAX_HISTORIA];
    const char *maior;
    const char *menor;
    int max_pistas;
    int min;
    int max;
    int tentativas;
    int bonus;
    int recompensa_base;
} CasoInfo;

typedef struct {
    int valor;
    char leitura[24];
} RegistroPalpite;

typedef struct {
    const char *nome;
    const char *vinculo;
    PropriedadeAlvo propriedade;
    const char *fala_true;
    const char *fala_false;
} InterrogadoInfo;

typedef struct {
    PropriedadeAlvo propriedade;
    const char *texto_true;
    const char *texto_false;
} PistaInfo;

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

/* 1 mentiroso no facil, 2 no medio, 3 no dificil */
static const int mentirosos[][MAX_INTERROGADOS] = {
    {0, 1, 0, 0, 0},
    {0, 0, 1, 0, 1},
    {0, 0, 1, 1, 1}
};

static const char *caracteristicas_suspeitos[][MAX_INTERROGADOS][MAX_CARACTERISTICAS] = {
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

static const PistaInfo pistas_caso_1[] = {
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

static const PistaInfo pistas_caso_2[] = {
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

static const PistaInfo pistas_caso_3[] = {
    {PROP_PRIMO, "SOC limpo: porta infectada prima.", "SOC limpo: porta infectada nao prima."},
    {PROP_PAR, "Telemetria da subestacao: porta PAR.", "Telemetria da subestacao: porta IMPAR."},
    {PROP_MULT5, "Firewall legado: porta divisivel por 5.", "Firewall legado: porta nao divisivel por 5."},
    {PROP_MULT7, "Plano de contingencia: porta multiplo de 7.", "Plano de contingencia: porta nao multiplo de 7."},
    {PROP_SOMA_DIGITOS_PAR, "Relatorio da operadora: soma dos algarismos da porta e PAR.",
     "Relatorio da operadora: soma dos algarismos da porta e IMPAR."},
    {PROP_DIGITO_ALTO, "Sonda de rede: ultimo digito da porta e 5 ou maior.",
     "Sonda de rede: ultimo digito da porta e menor que 5."}
};

static const CasoInfo *obter_caso(int id_caso) {
    for (size_t i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
        if (casos[i].id == id_caso) {
            return &casos[i];
        }
    }
    return &casos[0];
}

static int indice_caso(int id_caso) {
    for (size_t i = 0; i < sizeof(casos) / sizeof(casos[0]); i++) {
        if (casos[i].id == id_caso) {
            return (int)i;
        }
    }
    return 0;
}

static void obter_pool_pistas(int id_caso, const PistaInfo **pool, int *total) {
    if (id_caso == 1) {
        *pool = pistas_caso_1;
        *total = (int)(sizeof(pistas_caso_1) / sizeof(pistas_caso_1[0]));
        return;
    }
    if (id_caso == 2) {
        *pool = pistas_caso_2;
        *total = (int)(sizeof(pistas_caso_2) / sizeof(pistas_caso_2[0]));
        return;
    }
    *pool = pistas_caso_3;
    *total = (int)(sizeof(pistas_caso_3) / sizeof(pistas_caso_3[0]));
}

static void selecionar_pistas_aleatorias(int id_caso, int quantidade, int selecionadas[MAX_PISTAS]) {
    const PistaInfo *pool = NULL;
    int total = 0;
    int ordem[MAX_POOL_PISTAS];

    obter_pool_pistas(id_caso, &pool, &total);
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

static void exibir_ilustracao_cena(const CasoInfo *caso) {
    ui_section("ILUSTRACAO DA CENA", UI_CYAN);
    ui_box_top();
    for (int i = 0; i < 9; i++) {
        ui_box_text(caso->ilustracao[i]);
        ui_sleep_ms(85);
    }
    ui_box_bottom();
    ui_loading("Renderizando quadro forense", 10, 14);
}

static int soma_digitos(int valor) {
    int soma = 0;
    int numero = valor < 0 ? -valor : valor;

    while (numero > 0) {
        soma += numero % 10;
        numero /= 10;
    }
    return soma;
}

static int eh_primo(int valor) {
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

static int segredo_possui(int secreto, PropriedadeAlvo propriedade) {
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
            return soma_digitos(secreto) % 2 == 0;
        case PROP_PRIMO:
            return eh_primo(secreto);
    }
    return 0;
}

static const char *calor_da_distancia(int distancia, int max) {
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

static int calcular_pontos(const CasoInfo *caso, int tentativas_restantes, int pistas_usadas,
                           int interrogatorios_usados, int penalidades_ritmo, int venceu) {
    int pontos;

    if (!venceu) {
        return caso->bonus / 5;
    }

    pontos = caso->bonus + (tentativas_restantes * 120) - (pistas_usadas * 45)
             - (interrogatorios_usados * 30) - (penalidades_ritmo * 40);

    if (pontos < caso->bonus / 4) {
        pontos = caso->bonus / 4;
    }
    return pontos;
}

static int percentual_recompensa(int venceu, int tentativas_usadas) {
    if (!venceu) {
        return 15;
    }
    if (tentativas_usadas <= 1) {
        return 100;
    }
    if (tentativas_usadas == 2) {
        return 65;
    }
    return 40;
}

static int calcular_recompensa_moedas(const CasoInfo *caso, int venceu, int tentativas_usadas,
                                      int *percentual) {
    int pct = percentual_recompensa(venceu, tentativas_usadas);
    if (percentual != NULL) {
        *percentual = pct;
    }
    return (caso->recompensa_base * pct) / 100;
}

static const char *rank_investigativo(int pontos, int venceu) {
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

static void exibir_relatorio(const CasoInfo *caso) {
    limpar_tela();
    printf("\n");

    ui_banner("PREPARANDO INVESTIGACAO", caso->codinome);
    ui_stamp(caso->nome, "TRANSICAO TATICA", UI_DIM);
    ui_loading("Carregando mapa de evidencias", 20, 18);
    ui_loading("Calibrando sensores forenses", 20, 18);
    ui_alert("PRONTO", "O terminal forense esta em modo ativo.", UI_GREEN);
    ui_pause("Pressione ENTER para comecar...");
}

static void exibir_tutorial_basico(const CasoInfo *caso) {
    char linha[120];

    limpar_tela();
    printf("\n");
    ui_banner("GUIA RAPIDO DE INVESTIGACAO", caso->nome);
    ui_stamp(caso->codinome, "LEIA ANTES DE COMECAR", UI_DIM);

    ui_section("OBJETIVO", UI_CYAN);
    ui_box_top();
    snprintf(linha, sizeof(linha), "Descobrir o %s correto na faixa %d a %d.",
             caso->alvo, caso->min, caso->max);
    ui_box_wrap(linha, UI_WHITE);
    ui_box_wrap("Voce vence ao acertar o valor secreto antes das tentativas acabarem.", UI_WHITE);
    ui_box_bottom();

    ui_section("COMANDOS", UI_YELLOW);
    ui_box_top();
    ui_box_wrap("Digite um numero da faixa para tentar resolver o caso.", UI_WHITE);
    ui_box_wrap("Digite 0 para pedir pista tecnica (consome pista de campo).", UI_WHITE);
    ui_box_wrap("Digite -1 para interrogar um suspeito (maximo de 2 consultas).", UI_WHITE);
    ui_box_wrap("Digite -2 para revisar pistas e depoimentos ja coletados.", UI_WHITE);
    ui_box_wrap("Digite -3 para abrir o dossie completo da vitima e suspeitos.", UI_WHITE);
    ui_box_bottom();

    ui_section("COMO LER O FEEDBACK", UI_MAGENTA);
    ui_box_top();
    ui_box_wrap("MAIOR: o alvo secreto esta acima do numero que voce testou.", UI_WHITE);
    ui_box_wrap("MENOR: o alvo secreto esta abaixo do numero que voce testou.", UI_WHITE);
    ui_box_wrap("Leituras GELADO/FRIO/MORNO/QUENTE indicam o quao perto voce esta.", UI_WHITE);
    ui_box_bottom();

    ui_section("REGRAS IMPORTANTES", UI_RED);
    ui_box_top();
    snprintf(linha, sizeof(linha), "Cada caso tem limite de tentativas (%d neste caso).", caso->tentativas);
    ui_box_wrap(linha, UI_WHITE);
    ui_box_wrap("Entradas rapidas demais ativam anti-spam e podem consumir tentativa.", UI_WHITE);
    ui_box_wrap("Nem todo depoimento e verdadeiro: compare pistas antes de confiar.", UI_WHITE);
    ui_box_bottom();

    ui_pause("Pressione ENTER para iniciar a investigacao com este guia em mente...");
}

static void exibir_log(const RegistroPalpite *registros, int total) {
    ui_section("LOG DE VARREDURAS", UI_CYAN);
    if (total == 0) {
        ui_alert("LOG", "Nenhuma tentativa registrada ainda.", UI_DIM);
        return;
    }

    ui_box_top();
    ui_box_text("N.  Teste  Leitura");
    for (int i = 0; i < total; i++) {
        char linha[80];
        snprintf(linha, sizeof(linha), "%02d  %-5d  %s",
                 i + 1, registros[i].valor, registros[i].leitura);
        ui_box_text(linha);
    }
    ui_box_bottom();
}

static void exibir_historia_vitima(const CasoInfo *caso, int idx_caso) {
    limpar_tela();
    printf("\n");
    ui_banner("DOSSIE DA VITIMA", caso->nome);
    ui_stamp(caso->codinome, "HISTORIA E PONTOS SUSPEITOS", UI_DIM);
    ui_alert("AVISO", "Atencao: neste caso, um ou mais suspeitos podem fornecer pistas falsas.", UI_YELLOW);
    ui_section("HISTORIA", UI_CYAN);
    ui_box_top();
    for (int i = 0; i < MAX_HISTORIA; i++) {
        ui_box_wrap(caso->historia_vitima[i], UI_WHITE);
    }
    ui_box_bottom();

    ui_section("CARACTERISTICAS DOS SUSPEITOS", UI_MAGENTA);
    ui_box_top();
    for (int i = 0; i < MAX_INTERROGADOS; i++) {
        char linha[360];
        snprintf(linha, sizeof(linha), "%d) %s", i + 1, caracteristicas_suspeitos[idx_caso][i][0]);
        ui_box_wrap(linha, UI_WHITE);
        snprintf(linha, sizeof(linha), "   - %s", caracteristicas_suspeitos[idx_caso][i][1]);
        ui_box_wrap(linha, UI_DIM);
        snprintf(linha, sizeof(linha), "   - %s", caracteristicas_suspeitos[idx_caso][i][2]);
        ui_box_wrap(linha, UI_DIM);
        snprintf(linha, sizeof(linha), "   - %s", caracteristicas_suspeitos[idx_caso][i][3]);
        ui_box_wrap(linha, UI_DIM);
        if (i < MAX_INTERROGADOS - 1) {
            ui_box_wrap(" ", UI_DIM);
        }
    }
    ui_box_bottom();

    ui_pause("Pressione ENTER para voltar a investigacao...");
}

static void aplicar_pista(char *feedback, size_t tamanho, const CasoInfo *caso, int pista_numero,
                          int secreto, const int pistas_selecionadas[MAX_PISTAS]) {
    const PistaInfo *pool = NULL;
    int total = 0;
    int idx_pool;
    const PistaInfo *pista;

    if (pista_numero < 1 || pista_numero > caso->max_pistas) {
        snprintf(feedback, tamanho, "PERITO: Sem novas pistas no momento.");
        return;
    }

    obter_pool_pistas(caso->id, &pool, &total);
    idx_pool = pistas_selecionadas[pista_numero - 1];
    if (idx_pool < 0 || idx_pool >= total) {
        snprintf(feedback, tamanho, "PERITO: A trilha da evidencia esta incompleta.");
        return;
    }

    pista = &pool[idx_pool];
    snprintf(feedback, tamanho, "PERITO: %s",
             segredo_possui(secreto, pista->propriedade) ? pista->texto_true : pista->texto_false);
}

static void exibir_arquivo_evidencias(const CasoInfo *caso, int pistas_usadas,
                                      char pistas_registradas[MAX_PISTAS][MAX_FEEDBACK],
                                      char depoimentos[MAX_CONSULTAS][MAX_FEEDBACK],
                                      int total_depoimentos) {
    limpar_tela();
    printf("\n");
    ui_banner("ARQUIVO DE EVIDENCIAS", caso->nome);
    ui_stamp(caso->codinome, "REVISAO TATICA", UI_DIM);

    ui_section("PISTAS DESBLOQUEADAS", UI_CYAN);
    if (pistas_usadas == 0) {
        ui_alert("PISTAS", "Nenhuma pista coletada ate agora.", UI_DIM);
    } else {
        ui_box_top();
        for (int i = 0; i < pistas_usadas && i < MAX_PISTAS; i++) {
            char linha[340];
            snprintf(linha, sizeof(linha), "%d) %s", i + 1, pistas_registradas[i]);
            ui_box_wrap(linha, UI_YELLOW);
        }
        ui_box_bottom();
    }

    ui_section("INTERROGATORIOS REGISTRADOS", UI_MAGENTA);
    if (total_depoimentos == 0) {
        ui_alert("DEPOIMENTO", "Nenhum interrogado consultado ate agora.", UI_DIM);
    } else {
        ui_box_top();
        for (int i = 0; i < total_depoimentos; i++) {
            char linha[340];
            snprintf(linha, sizeof(linha), "%d) %s", i + 1, depoimentos[i]);
            ui_box_wrap(linha, UI_WHITE);
        }
        ui_box_bottom();
    }

    ui_pause("Pressione ENTER para voltar a investigacao...");
}

static void consultar_interrogado(char *feedback, size_t tamanho, const CasoInfo *caso, int secreto,
                                  int idx_caso, int ouvidos[MAX_INTERROGADOS], int *consultas_usadas,
                                  char depoimentos[MAX_CONSULTAS][MAX_FEEDBACK], int *total_depoimentos) {
    int opcao;
    const InterrogadoInfo *escolhido;
    int propriedade_real;
    int afirmacao;
    int mente;

    ui_section("REDE DE INTERROGADOS", UI_MAGENTA);
    ui_box_top();
    for (int i = 0; i < MAX_INTERROGADOS; i++) {
        char linha[220];
        snprintf(linha, sizeof(linha), "%d) %s - %s %s",
                 i + 1,
                 interrogados[idx_caso][i].nome,
                 interrogados[idx_caso][i].vinculo,
                 ouvidos[i] ? "[JA CONSULTADO]" : "");
        ui_box_wrap(linha, UI_WHITE);
    }
    ui_box_bottom();
    ui_prompt("INTERROGADO");

    opcao = ler_opcao(1, MAX_INTERROGADOS);
    if (ouvidos[opcao - 1]) {
        snprintf(feedback, tamanho, "CENTRAL: %s ja foi ouvido. Escolha outro contato.",
                 interrogados[idx_caso][opcao - 1].nome);
        return;
    }

    ouvidos[opcao - 1] = 1;
    (*consultas_usadas)++;

    escolhido = &interrogados[idx_caso][opcao - 1];
    propriedade_real = segredo_possui(secreto, escolhido->propriedade);
    mente = mentirosos[idx_caso][opcao - 1];
    afirmacao = mente ? !propriedade_real : propriedade_real;

    snprintf(feedback, tamanho, "DEPOIMENTO (%s): %s",
             escolhido->nome, afirmacao ? escolhido->fala_true : escolhido->fala_false);

    if (*total_depoimentos < MAX_CONSULTAS) {
        strncpy(depoimentos[*total_depoimentos], feedback, MAX_FEEDBACK - 1);
        depoimentos[*total_depoimentos][MAX_FEEDBACK - 1] = '\0';
        (*total_depoimentos)++;
    }
}

int confirmar_caso(int id_caso) {
    const CasoInfo *caso = obter_caso(id_caso);
    char valor[40];

    limpar_tela();
    printf("\n");
    ui_banner("SELECAO DE CASO", caso->nome);
    ui_stamp(caso->codinome, "CONFIRMACAO TATICA", UI_DIM);
    ui_section("RESUMO OPERACIONAL", UI_YELLOW);
    ui_box_top();
    ui_box_mid("Dificuldade", caso->dificuldade, UI_YELLOW);
    ui_box_mid("Alvo", caso->alvo, UI_CYAN);
    snprintf(valor, sizeof(valor), "%d a %d", caso->min, caso->max);
    ui_box_mid("Faixa", valor, UI_WHITE);
    snprintf(valor, sizeof(valor), "%d varreduras", caso->tentativas);
    ui_box_mid("Tentativas", valor, UI_GREEN);
    snprintf(valor, sizeof(valor), "%d pistas", caso->max_pistas);
    ui_box_mid("Pistas de campo", valor, UI_CYAN);
    ui_box_mid("Interrogatorios", "maximo de 2 consultas", UI_MAGENTA);
    snprintf(valor, sizeof(valor), "%d moedas", caso->recompensa_base);
    ui_box_mid("Recompensa maxima", valor, UI_GREEN);
    ui_box_bottom();

    exibir_ilustracao_cena(caso);

    ui_section("COMANDO", UI_MAGENTA);
    printf("  %s[1]%s Iniciar investigacao\n", UI_GREEN, UI_RESET);
    printf("  %s[2]%s Voltar ao mural de casos\n", UI_YELLOW, UI_RESET);
    ui_prompt("ACAO");

    return ler_opcao(1, 2) == 1;
}

void jogar_partida(int id_caso) {
    const CasoInfo *caso = obter_caso(id_caso);
    int idx_caso = indice_caso(id_caso);
    int pistas_sorteadas[MAX_PISTAS];
    int secreto = caso->min + (rand() % (caso->max - caso->min + 1));
    int tentativas = caso->tentativas;
    int pistas_usadas = 0;
    int interrogatorios_usados = 0;
    int penalidades_ritmo = 0;
    int interrogados_ouvidos[MAX_INTERROGADOS] = {0};
    int sequencia_rapida = 0;
    time_t ultimo_palpite = time(NULL) - 3;
    char pistas_registradas[MAX_PISTAS][MAX_FEEDBACK] = {{0}};
    char depoimentos[MAX_CONSULTAS][MAX_FEEDBACK] = {{0}};
    int total_depoimentos = 0;
    RegistroPalpite registros[MAX_REGISTROS];
    int cont_palpites = 0;
    char feedback[MAX_FEEDBACK] = "Central online. Cruze pistas e depoimentos antes de arriscar.";
    Sessao s;

    exibir_relatorio(caso);
    exibir_tutorial_basico(caso);

    strncpy(s.caso_nome, caso->nome, sizeof(s.caso_nome) - 1);
    s.caso_nome[sizeof(s.caso_nome) - 1] = '\0';
    strncpy(s.dificuldade, caso->dificuldade, sizeof(s.dificuldade) - 1);
    s.dificuldade[sizeof(s.dificuldade) - 1] = '\0';
    s.secreto = secreto;
    s.venceu = 0;

    selecionar_pistas_aleatorias(caso->id, caso->max_pistas, pistas_sorteadas);

    while (tentativas > 0) {
        int palpite;

        limpar_tela();
        printf("\n");
        ui_banner("INVESTIGACAO ATIVA", caso->nome);
        ui_stamp(caso->codinome, tentativas <= 2 ? "RISCO ALTO" : "SISTEMA ESTAVEL",
                 tentativas <= 2 ? UI_RED : UI_GREEN);
        ui_section("PAINEL DE CONTROLE", UI_CYAN);
        ui_meter("Integridade", tentativas, caso->tentativas, tentativas <= 2 ? UI_RED : UI_GREEN);
        ui_meter("Pistas de campo", caso->max_pistas - pistas_usadas, caso->max_pistas,
                 pistas_usadas == caso->max_pistas ? UI_RED : UI_CYAN);
        ui_meter("Interrogatorios", MAX_CONSULTAS - interrogatorios_usados, MAX_CONSULTAS,
                 interrogatorios_usados == MAX_CONSULTAS ? UI_RED : UI_MAGENTA);

        ui_section("LEITURA ATUAL", UI_YELLOW);
        ui_box_top();
        ui_box_mid("Alvo", caso->alvo, UI_CYAN);
        char faixa[40];
        snprintf(faixa, sizeof(faixa), "%d a %d", caso->min, caso->max);
        ui_box_mid("Faixa valida", faixa, UI_WHITE);
        ui_box_mid("Feedback", "analise abaixo", UI_YELLOW);
        ui_box_wrap(feedback, UI_YELLOW);
        ui_box_bottom();

        exibir_log(registros, cont_palpites);

        ui_section("COMANDO DO DETETIVE", UI_MAGENTA);
        printf("  %sNumero valido: %d a %d | 0 pista | -1 interrogado | -2 revisar evidencias | -3 dossie da vitima.%s\n",
               UI_DIM, caso->min, caso->max, UI_RESET);
        ui_prompt("ALVO");

        palpite = ler_opcao(-3, caso->max);

        if (palpite == -3) {
            exibir_historia_vitima(caso, idx_caso);
            continue;
        }

        if (palpite == -2) {
            exibir_arquivo_evidencias(caso, pistas_usadas, pistas_registradas,
                                      depoimentos, total_depoimentos);
            continue;
        }

        if (palpite == -1) {
            if (interrogatorios_usados < MAX_CONSULTAS) {
                consultar_interrogado(feedback, sizeof(feedback), caso, secreto, idx_caso,
                                      interrogados_ouvidos, &interrogatorios_usados,
                                      depoimentos, &total_depoimentos);
            } else {
                strcpy(feedback, "CENTRAL: Limite de 2 interrogatorios atingido.");
            }
            continue;
        }

        if (palpite == 0) {
            if (pistas_usadas < caso->max_pistas) {
                pistas_usadas++;
                aplicar_pista(feedback, sizeof(feedback), caso, pistas_usadas, secreto, pistas_sorteadas);
                strncpy(pistas_registradas[pistas_usadas - 1], feedback, MAX_FEEDBACK - 1);
                pistas_registradas[pistas_usadas - 1][MAX_FEEDBACK - 1] = '\0';
            } else {
                strcpy(feedback, "PERITO: Nao ha novas pistas de campo disponiveis.");
            }
            continue;
        }

        if (palpite < caso->min) {
            snprintf(feedback, sizeof(feedback), "CENTRAL: valor fora da faixa valida (%d a %d).",
                     caso->min, caso->max);
            continue;
        }

        time_t agora = time(NULL);
        if (difftime(agora, ultimo_palpite) <= 1.0) {
            sequencia_rapida++;
        } else {
            sequencia_rapida = 0;
        }
        ultimo_palpite = agora;

        if (sequencia_rapida >= 2) {
            tentativas--;
            penalidades_ritmo++;
            snprintf(feedback, sizeof(feedback),
                     "CENTRAL: excesso de entradas rapidas detectado. Uma varredura foi perdida.");
            ui_loading("Estabilizando terminal anti-spam", 12, 18);
            ui_alert("PROTOCOLO", feedback, UI_RED);
            pausar();
            continue;
        }

        if (cont_palpites < MAX_REGISTROS) {
            registros[cont_palpites].valor = palpite;
            strncpy(registros[cont_palpites].leitura,
                    calor_da_distancia(abs(palpite - secreto), caso->max),
                    sizeof(registros[cont_palpites].leitura) - 1);
            registros[cont_palpites].leitura[sizeof(registros[cont_palpites].leitura) - 1] = '\0';
            cont_palpites++;
        }

        if (palpite == secreto) {
            s.venceu = 1;
            break;
        }

        if (palpite < secreto) {
            snprintf(feedback, sizeof(feedback), "%s Leitura: %s.",
                     caso->maior, registros[cont_palpites - 1].leitura);
        } else {
            snprintf(feedback, sizeof(feedback), "%s Leitura: %s.",
                     caso->menor, registros[cont_palpites - 1].leitura);
        }
        tentativas--;

        printf("\n");
        ui_loading("Processando leitura", 14, 14);
        ui_scan_bar(palpite, caso->min, caso->max);
        ui_alert("SISTEMA", feedback, palpite < secreto ? UI_CYAN : UI_YELLOW);
        pausar();
    }

    s.tentativas_usadas = cont_palpites;

    limpar_tela();
    int pontos = calcular_pontos(caso, tentativas, pistas_usadas, interrogatorios_usados,
                                 penalidades_ritmo, s.venceu);
    int saldo_antes = getSaldo();
    int multiplicador_pct = 0;
    int recompensa = calcular_recompensa_moedas(caso, s.venceu, s.tentativas_usadas, &multiplicador_pct);
    int saldo_depois;

    printf("\n");
    ui_banner("RESULTADO DA INVESTIGACAO", caso->nome);
    ui_stamp("RELATORIO FINAL", s.venceu ? "CASO RESOLVIDO" : "ARQUIVO FRIO",
             s.venceu ? UI_GREEN : UI_RED);

    if (s.venceu == 1) {
        ui_alert("SUCESSO", "Caso resolvido com sucesso.", UI_GREEN);
        printf("  O alvo %d foi confirmado. A cadeia de evidencias foi preservada.\n", secreto);
    } else {
        ui_alert("FALHA", "A investigacao foi encerrada sem confirmacao.", UI_RED);
        printf("  O sistema travou. O alvo verdadeiro era %d.\n", secreto);
    }

    ui_section("DESEMPENHO", UI_CYAN);
    ui_box_top();
    char valor[64];
    snprintf(valor, sizeof(valor), "%d de %d", s.tentativas_usadas, caso->tentativas);
    ui_box_mid("Tentativas", valor, UI_WHITE);
    snprintf(valor, sizeof(valor), "%d", pistas_usadas);
    ui_box_mid("Pistas usadas", valor, pistas_usadas > 0 ? UI_YELLOW : UI_GREEN);
    snprintf(valor, sizeof(valor), "%d", interrogatorios_usados);
    ui_box_mid("Interrogatorios", valor, interrogatorios_usados > 0 ? UI_MAGENTA : UI_GREEN);
    snprintf(valor, sizeof(valor), "%d", penalidades_ritmo);
    ui_box_mid("Penalidades anti-spam", valor, penalidades_ritmo > 0 ? UI_RED : UI_GREEN);
    snprintf(valor, sizeof(valor), "%d", pontos);
    ui_box_mid("Pontuacao", valor, UI_CYAN);
    ui_box_mid("Rank", rank_investigativo(pontos, s.venceu), UI_MAGENTA);
    ui_box_bottom();

    creditar(recompensa);
    saldo_depois = getSaldo();

    ui_section("RECOMPENSA", UI_YELLOW);
    ui_box_top();
    snprintf(valor, sizeof(valor), "%d moedas", caso->recompensa_base);
    ui_box_mid("Recompensa base", valor, UI_WHITE);
    snprintf(valor, sizeof(valor), "%d%%", multiplicador_pct);
    ui_box_mid("Multiplicador", valor, UI_CYAN);
    snprintf(valor, sizeof(valor), "%d moedas", recompensa);
    ui_box_mid("Valor final", valor, UI_GREEN);
    snprintf(valor, sizeof(valor), "%d -> %d", saldo_antes, saldo_depois);
    ui_box_mid("Saldo", valor, UI_MAGENTA);
    ui_box_bottom();

    exibir_log(registros, cont_palpites);

    printf("\n");
    ui_alert("ARQUIVO", "Registro salvo no banco de dados do departamento.", UI_GREEN);

    salvar_sessao(s);
    pausar();
}
