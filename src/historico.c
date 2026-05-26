#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "historico.h"
#include "utils.h"
#include "tui.h"

static int saldoCache = 0;
static int saldoCarregado = 0;

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

    fclose(f);
    saldoCarregado = 1;
}

static void salvarPerfil(void) {
    FILE *f = fopen("perfil.txt", "w");
    if (f == NULL) {
        return;
    }
    fprintf(f, "%d\n", saldoCache);
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
        uiBoxBottom();

        if (temAnalise) {
            exibirMentoriaDinamica(total, vitorias, mediaTentativas, desvioPadrao, melhorTentativas, piorTentativas);
        }
    }
    free(sessoes);
    pausar();
}
