#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "historico.h"
#include "utils.h"
#include "tui.h"

static int saldo_cache = 0;
static int saldo_carregado = 0;

static void carregar_perfil(void) {
    FILE *f;
    char buffer[64];

    if (saldo_carregado) {
        return;
    }

    f = fopen("perfil.txt", "r");
    if (f == NULL) {
        saldo_cache = 0;
        saldo_carregado = 1;
        return;
    }

    if (fgets(buffer, sizeof(buffer), f) != NULL) {
        saldo_cache = atoi(buffer);
        if (saldo_cache < 0) {
            saldo_cache = 0;
        }
    }

    fclose(f);
    saldo_carregado = 1;
}

static void salvar_perfil(void) {
    FILE *f = fopen("perfil.txt", "w");
    if (f == NULL) {
        return;
    }
    fprintf(f, "%d\n", saldo_cache);
    fclose(f);
}

void creditar(int valor) {
    if (valor <= 0) {
        return;
    }
    carregar_perfil();
    saldo_cache += valor;
    salvar_perfil();
}

int debitar(int valor) {
    if (valor <= 0) {
        return 1;
    }

    carregar_perfil();
    if (valor > saldo_cache) {
        return 0;
    }

    saldo_cache -= valor;
    salvar_perfil();
    return 1;
}

int getSaldo(void) {
    carregar_perfil();
    return saldo_cache;
}

void salvar_sessao(Sessao s) {
    FILE *f = fopen("historico.txt", "a");
    if (f != NULL) {
        fprintf(f, "%s\n%s\n%d\n%d\n%d\n", s.caso_nome, s.dificuldade, s.secreto, s.tentativas_usadas, s.venceu);
        fclose(f);
    }
}

void exibir_historico(void) {
    FILE *f = fopen("historico.txt", "r");
    int total = 0;
    int vitorias = 0;
    int soma_tentativas = 0;
    int melhor_tentativas = 999;
    int pior_tentativas = 0;
    char melhor_caso[50] = "-";
    char pior_caso[50] = "-";
    
    limpar_tela();
    printf("\n");
    ui_banner("DOSSIE COMPLETO", "Historico e analise forense do detetive");
    ui_stamp("ARQUIVO LOCAL", "ANALISE DE PERFIL", UI_DIM);
    
    if (f == NULL) {
        ui_section("BANCO DE SESSOES", UI_YELLOW);
        ui_box_top();
        ui_box_text("Nenhum historico encontrado.");
        ui_box_text("Resolva ou tente um caso para gerar dados de perfil.");
        ui_box_bottom();
    } else {
        Sessao s;
        char buffer[50];

        ui_section("SESSOES REGISTRADAS", UI_CYAN);
        ui_box_top();
        ui_box_text("Caso                           Nivel    Alvo   Tent.  Status");
        ui_box_bottom();

        while (fgets(s.caso_nome, sizeof(s.caso_nome), f) != NULL) {
            
            s.caso_nome[strcspn(s.caso_nome, "\n")] = '\0';
            
            fgets(s.dificuldade, sizeof(s.dificuldade), f);
            s.dificuldade[strcspn(s.dificuldade, "\n")] = '\0';

            fgets(buffer, sizeof(buffer), f);
            s.secreto = atoi(buffer);

            fgets(buffer, sizeof(buffer), f);
            s.tentativas_usadas = atoi(buffer);

            fgets(buffer, sizeof(buffer), f);
            s.venceu = atoi(buffer);

            printf("  %-30s %-8s %5d %7d   ",
                   s.caso_nome, s.dificuldade, s.secreto, s.tentativas_usadas);

            if (s.venceu == 1) {
                printf(VERDE "RESOLVIDO%s\n", RESET);
                vitorias++;
            } else {
                printf(VERMELHO "FALHOU%s\n", RESET);
            }

            total++;
            soma_tentativas += s.tentativas_usadas;
            if (s.tentativas_usadas < melhor_tentativas) {
                melhor_tentativas = s.tentativas_usadas;
                strncpy(melhor_caso, s.caso_nome, sizeof(melhor_caso) - 1);
                melhor_caso[sizeof(melhor_caso) - 1] = '\0';
            }
            if (s.tentativas_usadas > pior_tentativas) {
                pior_tentativas = s.tentativas_usadas;
                strncpy(pior_caso, s.caso_nome, sizeof(pior_caso) - 1);
                pior_caso[sizeof(pior_caso) - 1] = '\0';
            }
        }
        fclose(f);

        ui_section("PAINEL ESTATISTICO", UI_MAGENTA);
        ui_box_top();
        char valor[80];
        if (total > 0) {
            snprintf(valor, sizeof(valor), "%d sessoes", total);
            ui_box_mid("Amostra", valor, UI_CYAN);
            snprintf(valor, sizeof(valor), "%d/%d (%d%%)", vitorias, total, (vitorias * 100) / total);
            ui_box_mid("Taxa de sucesso", valor, vitorias == total ? UI_GREEN : UI_YELLOW);
            snprintf(valor, sizeof(valor), "%.1f", (double)soma_tentativas / total);
            ui_box_mid("Media tent.", valor, UI_WHITE);
            snprintf(valor, sizeof(valor), "%s (%d)", melhor_caso, melhor_tentativas);
            ui_box_mid("Melhor caso", valor, UI_GREEN);
            snprintf(valor, sizeof(valor), "%s (%d)", pior_caso, pior_tentativas);
            ui_box_mid("Maior custo", valor, UI_RED);
        }
        snprintf(valor, sizeof(valor), "%d moedas", getSaldo());
        ui_box_mid("Saldo atual", valor, UI_MAGENTA);
        ui_box_bottom();

        ui_section("MENTORIA DINAMICA", UI_YELLOW);
        if (total > 0 && ((double)soma_tentativas / total) > 4.0) {
            ui_box_top();
            ui_box_wrap("Use busca binaria: teste o meio da faixa e corte metade das opcoes a cada feedback.", UI_YELLOW);
            ui_box_wrap("Exemplo em 1-100: 50, depois 25 ou 75, depois ajuste pelo retorno maior/menor.", UI_YELLOW);
            ui_box_bottom();
        } else if (total > 0) {
            ui_box_top();
            ui_box_wrap("Seu metodo esta consistente. Continue usando palpites centrais e lendo o historico de desvio.", UI_GREEN);
            ui_box_bottom();
        }
    }
    pausar();
}
