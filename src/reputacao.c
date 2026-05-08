#include <stdio.h>
#include <limits.h>
#include "reputacao.h"

static int clampReputacao(int reputacao) {
    if (reputacao < 0) return 0;
    if (reputacao > 100) return 100;
    return reputacao;
}

int carregarReputacao(void) {
    FILE *f = fopen("reputacao.txt", "r");
    int reputacao = 0;

    if (f == NULL) {
        return 0;
    }

    if (fscanf(f, "%d", &reputacao) != 1) {
        reputacao = 0;
    }

    fclose(f);
    return clampReputacao(reputacao);
}

void salvarReputacao(int reputacao) {
    FILE *f = fopen("reputacao.txt", "w");

    if (f == NULL) {
        return;
    }

    fprintf(f, "%d\n", clampReputacao(reputacao));
    fclose(f);
}

int atualizarReputacaoPorCaso(int reputacaoAtual, int idCaso, int venceu) {
    int delta = 0;

    if (venceu) {
        delta = (idCaso == 1) ? 5 : (idCaso == 2) ? 8 : 12;
    } else {
        delta = (idCaso == 1) ? -2 : (idCaso == 2) ? -3 : -4;
    }

    return clampReputacao(reputacaoAtual + delta);
}
