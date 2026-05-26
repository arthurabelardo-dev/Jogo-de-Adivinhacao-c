#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#include "utils.h"
#include "tui.h"

void limparTela(void) {
    uiClear();
}

void pausar(void) {
    uiPause("Pressione ENTER para continuar...");
}

int lerOpcao(int min, int max) {
    return lerInteiroIntervalo(min, max, NULL);
}

int lerInteiroIntervalo(int min, int max, const char *prompt) {
    char linha[80];

    while (1) {
        char *fim = NULL;
        long valor;

        if (prompt != NULL) {
            printf("%s", prompt);
        }

        if (fgets(linha, sizeof(linha), stdin) == NULL) {
            clearerr(stdin);
            continue;
        }

        linha[strcspn(linha, "\n")] = '\0';
        errno = 0;
        valor = strtol(linha, &fim, 10);

        while (fim != NULL && *fim == ' ') {
            fim++;
        }

        if (linha[0] != '\0' && errno != ERANGE && fim != NULL && *fim == '\0' &&
            valor >= min && valor <= max && valor >= INT_MIN && valor <= INT_MAX) {
            return (int)valor;
        }

        printf(VERMELHO "  Entrada invalida. Digite um numero entre %d e %d: " RESET, min, max);
    }
}
