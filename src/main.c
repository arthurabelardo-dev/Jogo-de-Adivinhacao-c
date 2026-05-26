#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"
#include "jogo.h"
#include "historico.h"
#include "tui.h"

static void limparQuebra(char *texto) {
    texto[strcspn(texto, "\r\n")] = '\0';
}

static int telaLogin(void) {
    const char *usuarioValido = "detetive";
    const char *senhaValida = "1234";
    int tentativas = 3;

    while (tentativas > 0) {
        char usuario[64];
        char senha[64];

        limparTela();
        printf("\n");
        uiLogo();
        printf("\n");
        uiBanner("C-Criminal // TERMINAL FORENSE", "Departamento de Investigacao - acesso restrito");
        uiStamp("CANAL CRIPTOGRAFADO", "VERSAO 2.0", UI_DIM);
        printf("\n");
        uiBoxTop();
        uiBoxMid("Terminal", "conectado ao servidor de evidencias", UI_CYAN);
        uiBoxMid("Usuario padrao", "detetive", UI_WHITE);
        uiBoxMid("Tentativas", tentativas == 1 ? "ultima tentativa" : "restantes", UI_YELLOW);
        uiBoxBottom();

        uiPrompt("USUARIO");
        if (fgets(usuario, sizeof(usuario), stdin) == NULL) {
            clearerr(stdin);
            continue;
        }
        limparQuebra(usuario);

        uiPrompt("SENHA");
        if (fgets(senha, sizeof(senha), stdin) == NULL) {
            clearerr(stdin);
            continue;
        }
        limparQuebra(senha);

        printf("\n");
        uiLoading("Validando credencial", 18, 20);
        uiLoading("Sincronizando banco de evidencias", 18, 16);

        if (strcmp(usuario, usuarioValido) == 0 && strcmp(senha, senhaValida) == 0) {
            uiAlert("ACESSO", "Credencial liberada. Boa cacada, Detetive.", UI_GREEN);
            uiPause("Pressione ENTER para continuar...");
            return 1;
        }

        tentativas--;
        uiAlert("FALHA", "Usuario ou senha invalidos.", UI_RED);
        if (tentativas > 0) {
            uiPause("Pressione ENTER para tentar novamente...");
        }
    }

    limparTela();
    printf("\n");
    uiBanner("ACESSO BLOQUEADO", "Muitas tentativas invalidas");
    uiAlert("SISTEMA", "Terminal bloqueado por seguranca.", UI_RED);
    uiPause("Pressione ENTER para encerrar...");
    return 0;
}

static void exibirMenu(void) {
    limparTela();
    printf("\n");
    uiLogo();
    printf("\n");
    uiBanner("DETETIVE DO TERMINAL", "Central de casos ativos");
    uiStamp("PLANTAO FORENSE", "3 CASOS ABERTOS", UI_DIM);
    uiSection("MURAL DE INVESTIGACOES", UI_CYAN);

    uiBoxTop();
    uiMenuItem(1, "Magnata", "Cofre termico", "FACIL", UI_GREEN);
    uiMenuItem(2, "Cassino", "Frequencia de fuga", "MEDIO", UI_YELLOW);
    uiMenuItem(3, "Apocalipse", "Porta infectada", "DIFICIL", UI_RED);
    uiMenuItem(4, "Dossie", "Historico e mentoria", "ARQUIVO", UI_CYAN);
    uiMenuItem(5, "Sair", "Encerrar terminal", "OFFLINE", UI_DIM);
    uiBoxBottom();
    uiPrompt("COMANDO");
}

int main(void) {
    int opcao;

    uiInit();
    srand((unsigned int)time(NULL));

    if (!telaLogin()) {
        return 1;
    }

    do {
        exibirMenu();
        opcao = lerOpcao(1, 5);

        switch (opcao) {
            case 1:
                if (confirmarCaso(1)) {
                    jogarPartida(1);
                }
                break;
            case 2:
                if (confirmarCaso(2)) {
                    jogarPartida(2);
                }
                break;
            case 3:
                if (confirmarCaso(3)) {
                    jogarPartida(3);
                }
                break;
            case 4:
                exibirHistorico();
                break;
            case 5:
                limparTela();
                printf("\n");
                uiBanner("ENCERRANDO SISTEMA", "Arquivando sessoes locais");
                uiLoading("Desconectando terminal", 20, 18);
                printf("\n  Ate a proxima investigacao, Detetive.\n\n");
                break;
        }
    } while (opcao != 5);

    return 0;
}
