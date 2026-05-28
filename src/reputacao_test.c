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

static int validarEfeitoTemporario(void) {
    EfeitoTemporario efeito;

    iniciarEfeitoTemporario(&efeito, 2);
    if (!efeitoTemporarioAtivo(&efeito) || efeito.duracao != 2) {
        printf("FALHA efeito temporario nao iniciou corretamente.\n");
        return 0;
    }

    if (consumirEfeitoTemporario(&efeito) != 1 || !efeitoTemporarioAtivo(&efeito)) {
        printf("FALHA efeito temporario nao permaneceu ativo apos 1 consumo.\n");
        return 0;
    }

    if (consumirEfeitoTemporario(&efeito) != 0 || efeitoTemporarioAtivo(&efeito)) {
        printf("FALHA efeito temporario nao expirou ao final.\n");
        return 0;
    }

    if (ajusteConfiabilidadePista(20, 20) != 0 ||
        ajusteConfiabilidadePista(20, 0) != 20 ||
        ajusteConfiabilidadePista(0, 20) != -20) {
        printf("FALHA ajuste de confiabilidade inconsistente.\n");
        return 0;
    }

    printf("OK efeito temporario expira corretamente.\n");
    return 1;
}

static int validarBalanceamentoRiscoRecompensa(void) {
    int cedo = calcularMultiplicadorRiscoRecompensa(1);
    int medio = calcularMultiplicadorRiscoRecompensa(2);
    int tarde = calcularMultiplicadorRiscoRecompensa(4);
    int extenso = aplicarPenalidadeInvestigacaoExtensa(200, 6);

    if (cedo != 150 || medio != 125 || tarde != 100 || extenso != 180) {
        printf("FALHA balanceamento risco/recompensa.\n");
        return 0;
    }

    printf("OK balanceamento risco/recompensa validado.\n");
    return 1;
}

int main(void) {
    int ok = 1;
    ok &= validarFaixa(80, 10, 10, 1);
    ok &= validarFaixa(50, 20, 25, 0);
    ok &= validarFaixa(10, 36, 50, 0);
    ok &= validarEfeitoTemporario();
    ok &= validarBalanceamentoRiscoRecompensa();

    if (!ok) {
        return 1;
    }

    printf("Todos os cenarios de reputacao foram validados.\n");
    return 0;
}
