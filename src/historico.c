#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "historico.h"
#include "utils.h"
#include "tui.h"

static int saldoCache = 0;
static int reputacaoCache = 50;
static int confiancaCache = 60;
static int itensCache[TOTAL_ITENS_LOJA] = {0};
static int saldoCarregado = 0;
static const char *itensNome[TOTAL_ITENS_LOJA] = {
    "Segunda Chance",
    "Analise Extra",
    "Scanner Forense",
    "Intuicao",
    "+2 Tentativas",
    "+1 Interrogatorio"
};
static const char *itensDescricao[TOTAL_ITENS_LOJA] = {
    "Ignora uma tentativa errada (uso unico por caso).",
    "Revela uma pista adicional confiavel (nunca falsa).",
    "Remove uma pista falsa ativa no caso (requer confianca >=50).",
    "Mostra intervalo de +-10 ao redor do alvo.",
    "Aumenta o limite de tentativas do caso em +2.",
    "Adiciona +1 slot de interrogatorio no caso."
};
static const int itensPreco[TOTAL_ITENS_LOJA] = {50, 30, 60, 80, 40, 25};

static int precoAjustadoPorConfianca(ItemLoja item) {
    int preco = itensPreco[item];
    if (confiancaCache >= 70) {
        return (preco * 80) / 100;
    }
    if (confiancaCache < 30) {
        return (preco * 120) / 100;
    }
    return preco;
}

static int somaTentativasRec(const Sessao *sessoes, int total) {
    if (total <= 0) {
        return 0;
    }
    return sessoes[total - 1].tentativasUsadas + somaTentativasRec(sessoes, total - 1);
}

static int minTentativasRec(const Sessao *sessoes, int total) {
    int anterior;

    if (total <= 1) {
        return sessoes[0].tentativasUsadas;
    }

    anterior = minTentativasRec(sessoes, total - 1);
    if (sessoes[total - 1].tentativasUsadas < anterior) {
        return sessoes[total - 1].tentativasUsadas;
    }
    return anterior;
}

static int maxTentativasRec(const Sessao *sessoes, int total) {
    int anterior;

    if (total <= 1) {
        return sessoes[0].tentativasUsadas;
    }

    anterior = maxTentativasRec(sessoes, total - 1);
    if (sessoes[total - 1].tentativasUsadas > anterior) {
        return sessoes[total - 1].tentativasUsadas;
    }
    return anterior;
}

static long long somaQuadradosTentativasRec(const Sessao *sessoes, int total) {
    long long valor;

    if (total <= 0) {
        return 0;
    }

    valor = sessoes[total - 1].tentativasUsadas;
    return (valor * valor) + somaQuadradosTentativasRec(sessoes, total - 1);
}

static double raizQuadradaNewton(double valor) {
    double aproximacao;

    if (valor <= 0.0) {
        return 0.0;
    }

    aproximacao = valor > 1.0 ? valor : 1.0;
    for (int i = 0; i < 24; i++) {
        aproximacao = 0.5 * (aproximacao + (valor / aproximacao));
    }

    return aproximacao;
}

static int lerSessao(FILE *f, Sessao *s) {
    char buffer[64];

    if (fgets(s->casoNome, sizeof(s->casoNome), f) == NULL) {
        return 0;
    }
    s->casoNome[strcspn(s->casoNome, "\n")] = '\0';

    if (fgets(s->dificuldade, sizeof(s->dificuldade), f) == NULL) {
        return 0;
    }
    s->dificuldade[strcspn(s->dificuldade, "\n")] = '\0';

    if (fgets(buffer, sizeof(buffer), f) == NULL) {
        return 0;
    }
    s->secreto = atoi(buffer);

    if (fgets(buffer, sizeof(buffer), f) == NULL) {
        return 0;
    }
    s->tentativasUsadas = atoi(buffer);

    if (fgets(buffer, sizeof(buffer), f) == NULL) {
        return 0;
    }
    s->venceu = atoi(buffer);

    return 1;
}

static void exibirMentoriaDinamica(int total, int vitorias, double media, double desvio,
                                   int melhorTentativas, int piorTentativas) {
    uiSection("MENTORIA DINAMICA", UI_YELLOW);
    if (total <= 0) {
        return;
    }

    uiBoxTop();
    if (total < 3) {
        uiBoxWrap("Amostra pequena: jogue mais casos para consolidar o padrao do seu estilo.", UI_WHITE);
    }
    if (media > 4.0) {
        uiBoxWrap("Sua media esta alta. Use busca binaria: palpite no centro e corte metade da faixa.", UI_YELLOW);
    }
    if (desvio > 1.5) {
        uiBoxWrap("Seu ritmo oscila bastante. Defina abertura fixa (centro) e ajuste por intervalos.", UI_MAGENTA);
    } else {
        uiBoxWrap("Boa consistencia: o desvio baixo indica execucao estavel entre partidas.", UI_GREEN);
    }
    if ((vitorias * 100) / total < 50) {
        uiBoxWrap("Taxa de sucesso abaixo de 50%: priorize reduzir erros cedo antes de gastar pistas.", UI_RED);
    } else if (melhorTentativas <= 2 && piorTentativas >= 5) {
        uiBoxWrap("Voce ja resolve rapido em alguns cenarios. Busque repetir essa abertura vencedora.", UI_CYAN);
    } else {
        uiBoxWrap("Estrategia madura: mantenha leitura de feedback e controle de risco por tentativa.", UI_GREEN);
    }
    uiBoxBottom();
}

static void carregarPerfil(void) {
    FILE *f;
    char buffer[64];

    if (saldoCarregado) {
        return;
    }

    f = fopen("perfil.txt", "r");
    if (f == NULL) {
        saldoCache = 0;
        saldoCarregado = 1;
        return;
    }

    if (fgets(buffer, sizeof(buffer), f) != NULL) {
        saldoCache = atoi(buffer);
        if (saldoCache < 0) {
            saldoCache = 0;
        }
    }
    if (fgets(buffer, sizeof(buffer), f) != NULL) {
        reputacaoCache = atoi(buffer);
    } else {
        reputacaoCache = 50;
    }
    if (fgets(buffer, sizeof(buffer), f) != NULL) {
        confiancaCache = atoi(buffer);
    } else {
        confiancaCache = 60;
    }
    for (int i = 0; i < TOTAL_ITENS_LOJA; i++) {
        if (fgets(buffer, sizeof(buffer), f) != NULL) {
            itensCache[i] = atoi(buffer);
        } else {
            itensCache[i] = 0;
        }
        if (itensCache[i] < 0) {
            itensCache[i] = 0;
        }
    }
    if (reputacaoCache < 0) {
        reputacaoCache = 0;
    }
    if (reputacaoCache > 100) {
        reputacaoCache = 100;
    }
    if (confiancaCache < 0) {
        confiancaCache = 0;
    }
    if (confiancaCache > 100) {
        confiancaCache = 100;
    }

    fclose(f);
    saldoCarregado = 1;
}

static void salvarPerfil(void) {
    FILE *f = fopen("perfil.txt", "w");
    if (f == NULL) {
        return;
    }
    fprintf(f, "%d\n%d\n%d\n", saldoCache, reputacaoCache, confiancaCache);
    for (int i = 0; i < TOTAL_ITENS_LOJA; i++) {
        fprintf(f, "%d\n", itensCache[i]);
    }
    fclose(f);
}

void creditar(int valor) {
    if (valor <= 0) {
        return;
    }
    carregarPerfil();
    saldoCache += valor;
    salvarPerfil();
}

int debitar(int valor) {
    if (valor <= 0) {
        return 1;
    }

    carregarPerfil();
    if (valor > saldoCache) {
        return 0;
    }

    saldoCache -= valor;
    salvarPerfil();
    return 1;
}

int getSaldo(void) {
    carregarPerfil();
    return saldoCache;
}

int getScore(void) {
    carregarPerfil();
    return reputacaoCache;
}

void incrementar(int valor) {
    if (valor <= 0) {
        return;
    }
    carregarPerfil();
    reputacaoCache += valor;
    if (reputacaoCache > 100) {
        reputacaoCache = 100;
    }
    salvarPerfil();
}

void decrementar(int valor) {
    if (valor <= 0) {
        return;
    }
    carregarPerfil();
    reputacaoCache -= valor;
    if (reputacaoCache < 0) {
        reputacaoCache = 0;
    }
    salvarPerfil();
}

int getConfiancaDelegacia(void) {
    carregarPerfil();
    return confiancaCache;
}

void aumentarConfianca(int valor) {
    if (valor <= 0) {
        return;
    }
    carregarPerfil();
    confiancaCache += valor;
    if (confiancaCache > 100) {
        confiancaCache = 100;
    }
    salvarPerfil();
}

void reduzirConfianca(int valor) {
    if (valor <= 0) {
        return;
    }
    carregarPerfil();
    confiancaCache -= valor;
    if (confiancaCache < 0) {
        confiancaCache = 0;
    }
    salvarPerfil();
}

int getQuantidadeItem(ItemLoja item) {
    if (item < 0 || item >= TOTAL_ITENS_LOJA) {
        return 0;
    }
    carregarPerfil();
    return itensCache[item];
}

int consumirItem(ItemLoja item) {
    if (item < 0 || item >= TOTAL_ITENS_LOJA) {
        return 0;
    }
    carregarPerfil();
    if (itensCache[item] <= 0) {
        return 0;
    }
    itensCache[item]--;
    salvarPerfil();
    return 1;
}

int comprarItem(ItemLoja item) {
    int preco;
    if (item < 0 || item >= TOTAL_ITENS_LOJA) {
        return 0;
    }
    carregarPerfil();
    preco = precoAjustadoPorConfianca(item);
    if (saldoCache < preco) {
        return 0;
    }
    saldoCache -= preco;
    itensCache[item]++;
    salvarPerfil();
    return 1;
}

void exibirLoja(void) {
    int opcao = 0;

    while (opcao != 7) {
        limparTela();
        printf("\n");
        uiBanner("LOJA ESTRATEGICA", "Suprimentos entre casos");
        uiStamp("COMPRAS PRE-CASO", "Sem acerto automatico", UI_DIM);
        uiSection("CATALOGO", UI_CYAN);
        uiBoxTop();
        for (int i = 0; i < TOTAL_ITENS_LOJA; i++) {
            char linha[220];
            const char *status = "Disponivel";
            int preco = precoAjustadoPorConfianca((ItemLoja)i);
            if (i == ITEM_SCANNER_FORENSE && getConfiancaDelegacia() < 50) {
                status = "Bloqueado: confianca < 50";
            } else if (getSaldo() < preco) {
                status = "Indisponivel: saldo baixo";
            }
            snprintf(linha, sizeof(linha), "[%d] %s | custo: %d | estoque: %d",
                     i + 1, itensNome[i], preco, getQuantidadeItem((ItemLoja)i));
            uiBoxWrap(linha, UI_WHITE);
            uiBoxWrap(itensDescricao[i], UI_DIM);
            uiBoxWrap(status, strcmp(status, "Disponivel") == 0 ? UI_GREEN : UI_YELLOW);
            if (i < TOTAL_ITENS_LOJA - 1) {
                uiBoxWrap(" ", UI_DIM);
            }
        }
        uiBoxBottom();
        uiSection("RECURSOS", UI_MAGENTA);
        uiBoxTop();
        {
            char linha[80];
            snprintf(linha, sizeof(linha), "%d moedas", getSaldo());
            uiBoxMid("Saldo", linha, UI_CYAN);
            snprintf(linha, sizeof(linha), "%d", getConfiancaDelegacia());
            uiBoxMid("Confianca", linha, UI_GREEN);
        }
        uiBoxBottom();
        uiSection("COMANDO", UI_YELLOW);
        printf("  %s[1..6]%s Comprar item | %s[7]%s Voltar\n", UI_WHITE, UI_RESET, UI_YELLOW, UI_RESET);
        uiPrompt("LOJA");
        opcao = lerOpcao(1, 7);
        if (opcao == 7) {
            break;
        }
        if (opcao == 3 && getConfiancaDelegacia() < 50) {
            uiAlert("LOJA", "Scanner Forense bloqueado: confianca da delegacia abaixo de 50.", UI_RED);
            pausar();
            continue;
        }
        if (!comprarItem((ItemLoja)(opcao - 1))) {
            uiAlert("LOJA", "Compra recusada: saldo insuficiente.", UI_RED);
            pausar();
            continue;
        }
        uiAlert("LOJA", "Item adquirido e armazenado no inventario.", UI_GREEN);
        pausar();
    }
}

void salvarSessao(Sessao s) {
    FILE *f = fopen("historico.txt", "a");
    if (f != NULL) {
        fprintf(f, "%s\n%s\n%d\n%d\n%d\n", s.casoNome, s.dificuldade, s.secreto, s.tentativasUsadas, s.venceu);
        fclose(f);
    }
}

void exibirHistorico(void) {
    FILE *f = fopen("historico.txt", "r");
    int total = 0;
    int vitorias = 0;
    int capacidade = 0;
    Sessao *sessoes = NULL;
    char melhorCaso[50] = "-";
    char piorCaso[50] = "-";
    
    limparTela();
    printf("\n");
    uiBanner("DOSSIE COMPLETO", "Historico e analise forense do detetive");
    uiStamp("ARQUIVO LOCAL", "ANALISE DE PERFIL", UI_DIM);
    
    if (f == NULL) {
        uiSection("BANCO DE SESSOES", UI_YELLOW);
        uiBoxTop();
        uiBoxText("Nenhum historico encontrado.");
        uiBoxText("Resolva ou tente um caso para gerar dados de perfil.");
        uiBoxBottom();
    } else {
        Sessao s;
        int melhorTentativas = 0;
        int piorTentativas = 0;
        double mediaTentativas = 0.0;
        double desvioPadrao = 0.0;
        int temAnalise = 0;

        uiSection("SESSOES REGISTRADAS", UI_CYAN);
        uiBoxTop();
        uiBoxText("Caso                           Nivel    Alvo   Tent.  Status");
        uiBoxBottom();

        while (lerSessao(f, &s)) {
            if (total >= capacidade) {
                int novaCapacidade = capacidade == 0 ? 16 : capacidade * 2;
                Sessao *novoBuffer = realloc(sessoes, (size_t)novaCapacidade * sizeof(Sessao));
                if (novoBuffer == NULL) {
                    uiAlert("MEMORIA", "Falha ao expandir analise. Relatorio parcial exibido.", UI_RED);
                    break;
                }
                sessoes = novoBuffer;
                capacidade = novaCapacidade;
            }

            sessoes[total] = s;

            printf("  %-30s %-8s %5d %7d   ",
                   s.casoNome, s.dificuldade, s.secreto, s.tentativasUsadas);

            if (s.venceu == 1) {
                printf(VERDE "RESOLVIDO%s\n", RESET);
                vitorias++;
            } else {
                printf(VERMELHO "FALHOU%s\n", RESET);
            }

            total++;
        }
        fclose(f);

        uiSection("PAINEL ESTATISTICO", UI_MAGENTA);
        uiBoxTop();
        char valor[80];
        if (total > 0) {
            int somaTentativas = somaTentativasRec(sessoes, total);
            melhorTentativas = minTentativasRec(sessoes, total);
            piorTentativas = maxTentativasRec(sessoes, total);
            long long somaQuadrados = somaQuadradosTentativasRec(sessoes, total);
            double variancia;

            mediaTentativas = (double)somaTentativas / total;
            variancia = ((double)somaQuadrados / total) - (mediaTentativas * mediaTentativas);
            if (variancia < 0.0) {
                variancia = 0.0;
            }
            desvioPadrao = raizQuadradaNewton(variancia);

            for (int i = 0; i < total; i++) {
                if (sessoes[i].tentativasUsadas == melhorTentativas) {
                    strncpy(melhorCaso, sessoes[i].casoNome, sizeof(melhorCaso) - 1);
                    melhorCaso[sizeof(melhorCaso) - 1] = '\0';
                    break;
                }
            }
            for (int i = 0; i < total; i++) {
                if (sessoes[i].tentativasUsadas == piorTentativas) {
                    strncpy(piorCaso, sessoes[i].casoNome, sizeof(piorCaso) - 1);
                    piorCaso[sizeof(piorCaso) - 1] = '\0';
                    break;
                }
            }

            snprintf(valor, sizeof(valor), "%d sessoes", total);
            uiBoxMid("Amostra", valor, UI_CYAN);
            snprintf(valor, sizeof(valor), "%d/%d (%d%%)", vitorias, total, (vitorias * 100) / total);
            uiBoxMid("Taxa de sucesso", valor, vitorias == total ? UI_GREEN : UI_YELLOW);
            snprintf(valor, sizeof(valor), "%.2f", mediaTentativas);
            uiBoxMid("Media tent.", valor, UI_WHITE);
            snprintf(valor, sizeof(valor), "%.2f", desvioPadrao);
            uiBoxMid("Desvio padrao", valor, UI_MAGENTA);
            snprintf(valor, sizeof(valor), "%s (%d)", melhorCaso, melhorTentativas);
            uiBoxMid("Melhor caso", valor, UI_GREEN);
            snprintf(valor, sizeof(valor), "%s (%d)", piorCaso, piorTentativas);
            uiBoxMid("Maior custo", valor, UI_RED);
            temAnalise = 1;
        }
        snprintf(valor, sizeof(valor), "%d moedas", getSaldo());
        uiBoxMid("Saldo atual", valor, UI_MAGENTA);
        snprintf(valor, sizeof(valor), "%d", getConfiancaDelegacia());
        uiBoxMid("Confianca delegacia", valor, UI_CYAN);
        uiBoxBottom();

        if (temAnalise) {
            exibirMentoriaDinamica(total, vitorias, mediaTentativas, desvioPadrao, melhorTentativas, piorTentativas);
        }
    }
    free(sessoes);
    pausar();
}
