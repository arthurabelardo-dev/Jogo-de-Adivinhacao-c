#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"
#include "jogo.h"
#include "historico.h"

static void telaLogin(void) {
    const char usuarioValido[] = "detetive";
    const char senhaValida[] = "1234";
    char usuario[50];
    char senha[50];

    limparTela();
    printf("\n");
    printf(CIANO "  ============================================================\n" RESET);
    printf("               SISTEMA DE INVESTIGACAO CRIMINAL\n");
    printf("         DEPARTAMENTO DE INVESTIGACAO - ACESSO RESTRITO\n");
    printf(CIANO "  ============================================================\n\n" RESET);

    while (1) {
        printf("  Usuario: ");
        if (fgets(usuario, sizeof(usuario), stdin) == NULL) {
            continue;
        }
        usuario[strcspn(usuario, "\n")] = '\0';

        printf(VERDE "  Senha: " RESET);
        if (fgets(senha, sizeof(senha), stdin) == NULL) {
            continue;
        }
        senha[strcspn(senha, "\n")] = '\0';

        if (strcmp(usuario, usuarioValido) == 0 && strcmp(senha, senhaValida) == 0) {
            printf(VERDE "\n  Acesso liberado. Bem-vindo(a), Detetive.\n" RESET);
            pausar();
            return;
        }

        printf(VERMELHO "\n  Credenciais invalidas. Tente novamente.\n\n" RESET);
    }
}

static void exibirMenu(void) {
    limparTela();
    printf("\n");
    printf("  ============================================================\n");
    printf(CIANO "             ||||| DETETIVE DO TERMINAL |||||\n" RESET);
    printf(AMARELO "          *** SISTEMA DE INVESTIGACAO CRIMINAL ***\n\n" RESET);
    printf("  Novos casos aguardam sua atencao.\n\n");
    printf("  Selecione uma investigacao:\n\n");
    
    printf("  [1] O Ultimo Suspiro do Magnata    " VERDE "[Facil]\n" RESET);
    printf("  [2] Frequencia de Fuga no Cassino  " AMARELO "[Medio]\n" RESET);
    printf("  [3] Protocolo Apocalipse           " VERMELHO "[Dificil]\n\n" RESET);
    printf("  [4] Ver meu dossie\n");
    printf("  [5] Sair\n\n");
    printf("  > ");
}

int main(void) {
    srand(time(NULL)); 
    telaLogin();

    int opcao;
    do {
        exibirMenu();
        opcao = lerOpcao(1, 5);

        switch (opcao) {
            case 1: jogarPartida(1); break;
            case 2: jogarPartida(2); break;
            case 3: jogarPartida(3); break;
            case 4: exibirHistorico(); break;
            case 5:
                limparTela();
                printf("\n  Encerrando o sistema...\n\n");
                printf("  Ate a proxima investigacao, Detetive.\n\n");
                break;
        }
    } while (opcao != 5);

    return 0;
}
