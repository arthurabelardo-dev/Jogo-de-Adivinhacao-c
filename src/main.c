#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"
#include "jogo.h"
#include "historico.h"
#include "tui.h"

static void limpar_quebra(char *texto) {
    texto[strcspn(texto, "\r\n")] = '\0';
}

static int tela_login(void) {
    const char *usuario_valido = "detetive";
    const char *senha_valida = "1234";
    int tentativas = 3;

    while (tentativas > 0) {
        char usuario[64];
        char senha[64];

        limpar_tela();
        printf("\n");
        ui_logo();
        printf("\n");
        ui_banner("C-Criminal // TERMINAL FORENSE", "Departamento de Investigacao - acesso restrito");
        ui_stamp("CANAL CRIPTOGRAFADO", "VERSAO 2.0", UI_DIM);
        printf("\n");
        ui_box_top();
        ui_box_mid("Terminal", "conectado ao servidor de evidencias", UI_CYAN);
        ui_box_mid("Usuario padrao", "detetive", UI_WHITE);
        ui_box_mid("Tentativas", tentativas == 1 ? "ultima tentativa" : "restantes", UI_YELLOW);
        ui_box_bottom();

        ui_prompt("USUARIO");
        if (fgets(usuario, sizeof(usuario), stdin) == NULL) {
            clearerr(stdin);
            continue;
        }
        limpar_quebra(usuario);

        ui_prompt("SENHA");
        if (fgets(senha, sizeof(senha), stdin) == NULL) {
            clearerr(stdin);
            continue;
        }
        limpar_quebra(senha);

        printf("\n");
        ui_loading("Validando credencial", 18, 20);
        ui_loading("Sincronizando banco de evidencias", 18, 16);

        if (strcmp(usuario, usuario_valido) == 0 && strcmp(senha, senha_valida) == 0) {
            ui_alert("ACESSO", "Credencial liberada. Boa cacada, Detetive.", UI_GREEN);
            ui_pause("Pressione ENTER para continuar...");
            return 1;
        }

        tentativas--;
        ui_alert("FALHA", "Usuario ou senha invalidos.", UI_RED);
        if (tentativas > 0) {
            ui_pause("Pressione ENTER para tentar novamente...");
        }
    }

    limpar_tela();
    printf("\n");
    ui_banner("ACESSO BLOQUEADO", "Muitas tentativas invalidas");
    ui_alert("SISTEMA", "Terminal bloqueado por seguranca.", UI_RED);
    ui_pause("Pressione ENTER para encerrar...");
    return 0;
}

static void exibir_menu(void) {
    limpar_tela();
    printf("\n");
    ui_logo();
    printf("\n");
    ui_banner("DETETIVE DO TERMINAL", "Central de casos ativos");
    ui_stamp("PLANTAO FORENSE", "3 CASOS ABERTOS", UI_DIM);
    ui_section("MURAL DE INVESTIGACOES", UI_CYAN);

    ui_box_top();
    ui_menu_item(1, "Magnata", "Cofre termico", "FACIL", UI_GREEN);
    ui_menu_item(2, "Cassino", "Frequencia de fuga", "MEDIO", UI_YELLOW);
    ui_menu_item(3, "Apocalipse", "Porta infectada", "DIFICIL", UI_RED);
    ui_menu_item(4, "Dossie", "Historico e mentoria", "ARQUIVO", UI_CYAN);
    ui_menu_item(5, "Sair", "Encerrar terminal", "OFFLINE", UI_DIM);
    ui_box_bottom();
    ui_prompt("COMANDO");
}

int main(void) {
    int opcao;

    ui_init();
    srand((unsigned int)time(NULL));

    if (!tela_login()) {
        return 1;
    }

    do {
        exibir_menu();
        opcao = ler_opcao(1, 5);

        switch (opcao) {
            case 1:
                if (confirmar_caso(1)) {
                    jogar_partida(1);
                }
                break;
            case 2:
                if (confirmar_caso(2)) {
                    jogar_partida(2);
                }
                break;
            case 3:
                if (confirmar_caso(3)) {
                    jogar_partida(3);
                }
                break;
            case 4:
                exibir_historico();
                break;
            case 5:
                limpar_tela();
                printf("\n");
                ui_banner("ENCERRANDO SISTEMA", "Arquivando sessoes locais");
                ui_loading("Desconectando terminal", 20, 18);
                printf("\n  Ate a proxima investigacao, Detetive.\n\n");
                break;
        }
    } while (opcao != 5);

    return 0;
}
