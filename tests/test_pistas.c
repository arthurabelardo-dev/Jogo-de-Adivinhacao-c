#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/pistas.h"
#include "../include/utils.h"

// ============================================================
// TESTES DO SISTEMA DE PISTAS
// ============================================================

// Teste 1: Verificar que pistas diretas sempre existem
void teste_pistas_diretas_existem() {
    printf("\n[TESTE 1] Verificar que pistas DIRETAS sempre existem por caso\n");
    printf("==============================================================\n");
    
    for (int caso = 1; caso <= 3; caso++) {
        BancoPistas banco;
        inicializarBancoPistas(caso, 25 + caso * 10, &banco);
        
        int contarDiretas = 0;
        for (int i = 0; i < banco.totalPistas; i++) {
            if (banco.pistas[i].tipo == PISTA_DIRETA) {
                contarDiretas++;
            }
        }
        
        printf("  Caso %d: %d pistas diretas encontradas ", caso, contarDiretas);
        if (contarDiretas >= 2) {
            printf("✓ PASSOU\n");
        } else {
            printf("✗ FALHOU (minimo 2 necessarias)\n");
        }
    }
}

// Teste 2: Verificar que pistas falsas nao sao geradas sem validacao
void teste_pistas_falsas_validacao() {
    printf("\n[TESTE 2] Verificar logica de pistas FALSAS (validacao)\n");
    printf("==============================================================\n");
    
    for (int caso = 1; caso <= 3; caso++) {
        BancoPistas banco;
        int numeroSecreto = 30;
        inicializarBancoPistas(caso, numeroSecreto, &banco);
        
        int contarFalsas = 0;
        int contarFalsasValidas = 0; // Falsas que apontam para número certo (erro!)
        
        for (int i = 0; i < banco.totalPistas; i++) {
            if (banco.pistas[i].tipo == PISTA_FALSA) {
                contarFalsas++;
                // Pista falsa NUNCA deve ter vinculoNumero = 1
                if (banco.pistas[i].vinculoNumero == 1) {
                    contarFalsasValidas++;
                }
            }
        }
        
        printf("  Caso %d: %d pistas falsas, %d incorretas (validacao)\n", 
               caso, contarFalsas, contarFalsasValidas);
        
        if (contarFalsasValidas == 0) {
            printf("        ✓ PASSOU - Nenhuma falsa aponta para número certo\n");
        } else {
            printf("        ✗ FALHOU - Falsas estao apontando para numero!\n");
        }
    }
}

// Teste 3: Verificar minimo de pistas por caso
void teste_minimo_pistas() {
    printf("\n[TESTE 3] Verificar minimo de pistas exigidas por caso\n");
    printf("==============================================================\n");
    
    int minimosPorCaso[] = {1, 1, 1};
    
    for (int caso = 1; caso <= 3; caso++) {
        BancoPistas banco;
        inicializarBancoPistas(caso, 40, &banco);
        
        printf("  Caso %d: Minimo requerido = %d pistas ", caso, banco.minimoRequired);
        if (banco.minimoRequired == minimosPorCaso[caso-1]) {
            printf("✓ PASSOU\n");
        } else {
            printf("✗ FALHOU (esperado %d, recebido %d)\n", 
                   minimosPorCaso[caso-1], banco.minimoRequired);
        }
    }
}

// Teste 4: Verificar sistema de reputação de suspeitos
void teste_reputacao_suspeitos() {
    printf("\n[TESTE 4] Verificar sistema de reputacao de suspeitos\n");
    printf("==============================================================\n");
    
    BancoPistas banco;
    inicializarBancoPistas(1, 25, &banco);
    
    printf("  Reputacao inicial do suspeito 1: %.2f\n", banco.suspeitos[0].reputacao);
    
    // Aumentar reputação
    ajustarReputacaoSuspeito(&banco, 1, 0.2f);
    printf("  Apos +0.2: %.2f ", banco.suspeitos[0].reputacao);
    if (banco.suspeitos[0].reputacao == 0.4f) {
        printf("✓ PASSOU\n");
    } else {
        printf("✗ FALHOU\n");
    }
    
    // Testar limite máximo
    ajustarReputacaoSuspeito(&banco, 1, 1.0f);
    printf("  Apos +1.0: %.2f (limite 1.0) ", banco.suspeitos[0].reputacao);
    if (banco.suspeitos[0].reputacao == 1.0f) {
        printf("✓ PASSOU\n");
    } else {
        printf("✗ FALHOU\n");
    }
}

// Teste 5: Verificar confiabilidade em escala 0.0-1.0
void teste_confiabilidade_escala() {
    printf("\n[TESTE 5] Verificar confiabilidade em escala 0.0-1.0\n");
    printf("==============================================================\n");
    
    BancoPistas banco;
    inicializarBancoPistas(1, 25, &banco);
    
    int todosValidos = 1;
    for (int i = 0; i < banco.totalPistas; i++) {
        if (banco.pistas[i].confiabilidade < 0.0f || banco.pistas[i].confiabilidade > 1.0f) {
            printf("  Pista %d confiabilidade INVALIDA: %.2f\n", i+1, banco.pistas[i].confiabilidade);
            todosValidos = 0;
        }
    }
    
    if (todosValidos) {
        printf("  Todas as pistas tem confiabilidade valida (0.0-1.0) ✓ PASSOU\n");
    } else {
        printf("  FALHOU - Confiabilidades fora da escala\n");
    }
}

// Teste 6: Verificar que sistema de suspeitos existe
void teste_sistema_suspeitos() {
    printf("\n[TESTE 6] Verificar existencia de sistema de suspeitos\n");
    printf("==============================================================\n");
    
    for (int caso = 1; caso <= 3; caso++) {
        BancoPistas banco;
        inicializarBancoPistas(caso, 30, &banco);
        
        printf("  Caso %d: %d suspeitos cadastrados ", caso, banco.totalSuspeitos);
        if (banco.totalSuspeitos >= 2) {
            printf("✓ PASSOU\n");
            for (int i = 0; i < banco.totalSuspeitos; i++) {
                printf("           - %s (rep: %.2f)\n", banco.suspeitos[i].nome, 
                       banco.suspeitos[i].reputacao);
            }
        } else {
            printf("✗ FALHOU (minimo 2 suspeitos)\n");
        }
    }
}

// Teste 7: Verificar tipos de pista sao válidos
void teste_tipos_pista() {
    printf("\n[TESTE 7] Verificar que todos os tipos de pista existem\n");
    printf("==============================================================\n");
    
    BancoPistas banco;
    inicializarBancoPistas(2, 50, &banco);
    
    int temDireta = 0, temNarrativa = 0, temFalsa = 0;
    
    for (int i = 0; i < banco.totalPistas; i++) {
        switch (banco.pistas[i].tipo) {
            case PISTA_DIRETA: temDireta = 1; break;
            case PISTA_NARRATIVA: temNarrativa = 1; break;
            case PISTA_FALSA: temFalsa = 1; break;
        }
    }
    
    printf("  Pistas diretas: %s\n", temDireta ? "✓ SIM" : "✗ NAO");
    printf("  Pistas narrativas: %s\n", temNarrativa ? "✓ SIM" : "✗ NAO");
    printf("  Pistas falsas: %s\n", temFalsa ? "✓ SIM" : "✗ NAO");
    
    if (temDireta && temNarrativa && temFalsa) {
        printf("  ✓ PASSOU - Todos os tipos presentes\n");
    } else {
        printf("  ✗ FALHOU - Faltam tipos de pista\n");
    }
}

// Teste 8: Verificar preenchimento de casoId no modelo de dados
void teste_caso_id_modelo() {
    printf("\n[TESTE 8] Verificar campo casoId em todas as pistas\n");
    printf("==============================================================\n");

    for (int caso = 1; caso <= 3; caso++) {
        BancoPistas banco;
        int ok = 1;
        inicializarBancoPistas(caso, 30, &banco);
        for (int i = 0; i < banco.totalPistas; i++) {
            if (banco.pistas[i].casoId != caso) {
                ok = 0;
                break;
            }
        }
        printf("  Caso %d: %s\n", caso, ok ? "✓ PASSOU" : "✗ FALHOU");
    }
}

// Teste 9: Verificar verificarMinimoAceitacao
void teste_minimo_aceitacao() {
    printf("\n[TESTE 8] Verificar validacao de minimo de pistas\n");
    printf("==============================================================\n");
    
    BancoPistas banco;
    inicializarBancoPistas(1, 25, &banco);
    
    printf("  Minimo requerido: %d pistas\n", banco.minimoRequired);
    printf("  Coletadas inicialmente: %d ", banco.pistasColetadas);
    printf("(%s)\n", verificarMinimoAceitacao(&banco) ? "OK" : "NAO OK");
    
    // Simular coleta
    for (int i = 0; i < banco.minimoRequired && i < banco.totalPistas; i++) {
        banco.pistas[i].jaApresentada = 1;
        banco.pistasColetadas++;
    }
    
    printf("  Apos coletar %d: ", banco.pistasColetadas);
    if (verificarMinimoAceitacao(&banco)) {
        printf("✓ PASSOU - Minimo atingido\n");
    } else {
        printf("✗ FALHOU - Minimo nao reconhecido\n");
    }
}

// Teste 10: Validar que pistas falsas podem ser ativadas por trigger de reputacao
void teste_trigger_pistas_falsas() {
    printf("\n[TESTE 10] Verificar trigger de pistas falsas por reputacao\n");
    printf("==============================================================\n");

    int houveFalsa = 0;
    srand(42);

    for (int rodada = 0; rodada < 20; rodada++) {
        BancoPistas banco;
        inicializarBancoPistas(1, 37, &banco);
        banco.reputacaoGeral = 0.1f;
        banco.suspeitos[0].reputacao = 0.1f;
        banco.suspeitos[1].reputacao = 0.1f;

        apresentarPista(&banco, 37);
        apresentarPista(&banco, 37);
        apresentarPista(&banco, 37);

        for (int i = 0; i < banco.totalPistas; i++) {
            if (banco.pistas[i].jaApresentada && banco.pistas[i].tipo == PISTA_FALSA) {
                houveFalsa = 1;
                break;
            }
        }
        if (houveFalsa) break;
    }

    printf("  Trigger de falsa com reputacao baixa: %s\n", houveFalsa ? "✓ PASSOU" : "✗ FALHOU");
}

// ============================================================
// EXECUÇÃO DOS TESTES
// ============================================================

int main(void) {
    printf("\n");
    printf(CIANO "╔════════════════════════════════════════════════════════╗\n" RESET);
    printf(CIANO "║       SUITE DE TESTES - SISTEMA DE PISTAS             ║\n" RESET);
    printf(CIANO "║       Criminal Investigation Game                      ║\n" RESET);
    printf(CIANO "╚════════════════════════════════════════════════════════╝\n" RESET);
    
    teste_pistas_diretas_existem();
    teste_pistas_falsas_validacao();
    teste_minimo_pistas();
    teste_reputacao_suspeitos();
    teste_confiabilidade_escala();
    teste_sistema_suspeitos();
    teste_tipos_pista();
    teste_caso_id_modelo();
    teste_minimo_aceitacao();
    teste_trigger_pistas_falsas();
    
    printf("\n");
    printf(CIANO "════════════════════════════════════════════════════════\n" RESET);
    printf(VERDE "  Testes concluidos! Verifique os resultados acima.\n" RESET);
    printf(CIANO "════════════════════════════════════════════════════════\n\n" RESET);
    
    return 0;
}
