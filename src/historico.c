#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "historico.h"
#include "utils.h"
#include "tui.h"

static int saldoCache = 0;
static int saldoCarregado = 0;

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
    int somaTentativas = 0;
    int melhorTentativas = 999;
    int piorTentativas = 0;
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
        char buffer[50];

        uiSection("SESSOES REGISTRADAS", UI_CYAN);
        uiBoxTop();
        uiBoxText("Caso                           Nivel    Alvo   Tent.  Status");
        uiBoxBottom();

        while (fgets(s.casoNome, sizeof(s.casoNome), f) != NULL) {
            
            s.casoNome[strcspn(s.casoNome, "\n")] = '\0';
            
            fgets(s.dificuldade, sizeof(s.dificuldade), f);
            s.dificuldade[strcspn(s.dificuldade, "\n")] = '\0';

            fgets(buffer, sizeof(buffer), f);
            s.secreto = atoi(buffer);

            fgets(buffer, sizeof(buffer), f);
            s.tentativasUsadas = atoi(buffer);

            fgets(buffer, sizeof(buffer), f);
            s.venceu = atoi(buffer);

            printf("  %-30s %-8s %5d %7d   ",
                   s.casoNome, s.dificuldade, s.secreto, s.tentativasUsadas);

            if (s.venceu == 1) {
                printf(VERDE "RESOLVIDO%s\n", RESET);
                vitorias++;
            } else {
                printf(VERMELHO "FALHOU%s\n", RESET);
            }

            total++;
            somaTentativas += s.tentativasUsadas;
            if (s.tentativasUsadas < melhorTentativas) {
                melhorTentativas = s.tentativasUsadas;
                strncpy(melhorCaso, s.casoNome, sizeof(melhorCaso) - 1);
                melhorCaso[sizeof(melhorCaso) - 1] = '\0';
            }
            if (s.tentativasUsadas > piorTentativas) {
                piorTentativas = s.tentativasUsadas;
                strncpy(piorCaso, s.casoNome, sizeof(piorCaso) - 1);
                piorCaso[sizeof(piorCaso) - 1] = '\0';
            }
        }
        fclose(f);

        uiSection("PAINEL ESTATISTICO", UI_MAGENTA);
        uiBoxTop();
        char valor[80];
        if (total > 0) {
            snprintf(valor, sizeof(valor), "%d sessoes", total);
            uiBoxMid("Amostra", valor, UI_CYAN);
            snprintf(valor, sizeof(valor), "%d/%d (%d%%)", vitorias, total, (vitorias * 100) / total);
            uiBoxMid("Taxa de sucesso", valor, vitorias == total ? UI_GREEN : UI_YELLOW);
            snprintf(valor, sizeof(valor), "%.1f", (double)somaTentativas / total);
            uiBoxMid("Media tent.", valor, UI_WHITE);
            snprintf(valor, sizeof(valor), "%s (%d)", melhorCaso, melhorTentativas);
            uiBoxMid("Melhor caso", valor, UI_GREEN);
            snprintf(valor, sizeof(valor), "%s (%d)", piorCaso, piorTentativas);
            uiBoxMid("Maior custo", valor, UI_RED);
        }
        snprintf(valor, sizeof(valor), "%d moedas", getSaldo());
        uiBoxMid("Saldo atual", valor, UI_MAGENTA);
        uiBoxBottom();

        uiSection("MENTORIA DINAMICA", UI_YELLOW);
        if (total > 0 && ((double)somaTentativas / total) > 4.0) {
            uiBoxTop();
            uiBoxWrap("Use busca binaria: teste o meio da faixa e corte metade das opcoes a cada feedback.", UI_YELLOW);
            uiBoxWrap("Exemplo em 1-100: 50, depois 25 ou 75, depois ajuste pelo retorno maior/menor.", UI_YELLOW);
            uiBoxBottom();
        } else if (total > 0) {
            uiBoxTop();
            uiBoxWrap("Seu metodo esta consistente. Continue usando palpites centrais e lendo o historico de desvio.", UI_GREEN);
            uiBoxBottom();
        }
    }
    pausar();
}
