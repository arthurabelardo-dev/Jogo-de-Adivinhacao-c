#include <stdio.h>
#include "jogo.h"

static int validarFaixa(int reputacao, int pistaEsperada, int mentiraEsperada, int peritoPrecisoEsperado) {
    int pista = chancePistaFalsaPorReputacao(reputacao);
    int mentira = chanceMentiraPorReputacao(reputacao);
    int perito = peritoPrecisoPorReputacao(reputacao);

    if (pista != pistaEsperada || mentira != mentiraEsperada || perito != peritoPrecisoEsperado) {
        printf("FALHA rep=%d -> pista=%d (esp %d), mentira=%d (esp %d), perito=%d (esp %d)\n",
               reputacao, pista, pistaEsperada, mentira, mentiraEsperada, perito, peritoPrecisoEsperado);
        return 0;
    }

    printf("OK rep=%d -> pista=%d, mentira=%d, perito=%d\n", reputacao, pista, mentira, perito);
    return 1;
}

int main(void) {
    int ok = 1;
    ok &= validarFaixa(80, 10, 10, 1);
    ok &= validarFaixa(50, 20, 25, 0);
    ok &= validarFaixa(10, 36, 50, 0);

    if (!ok) {
        return 1;
    }

    printf("Todos os cenarios de reputacao foram validados.\n");
    return 0;
}
