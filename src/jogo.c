#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "jogo.h"
#include "historico.h"
#include "utils.h"
#include "pistas.h"

typedef enum {
    DICA_PAR,
    DICA_IMPAR,
    DICA_PRIMO,
    DICA_MULT5,
    DICA_MULT3,
    DICA_MULT10,
    DICA_MULT25,
    DICA_QUADRADO,
    DICA_SOMA_DIGITOS
} TipoDica;

static int ehPrimo(int n) {
    if (n <= 1) {
        return 0;
    }
    if (n % 2 == 0) {
        return n == 2;
    }
    for (int i = 3; i * i <= n; i += 2) {
        if (n % i == 0) {
            return 0;
        }
    }
    return 1;
}

static int ehQuadradoPerfeito(int n) {
    for (int i = 1; i * i <= n; i++) {
        if (i * i == n) {
            return 1;
        }
    }
    return 0;
}

static int somaDigitos(int n) {
    int sum = 0;
    while (n > 0) {
        sum += n % 10;
        n /= 10;
    }
    return sum;
}

static int dicaCombina(TipoDica dica, int secreto) {
    switch (dica) {
        case DICA_PAR:
            return (secreto % 2) == 0;
        case DICA_IMPAR:
            return (secreto % 2) != 0;
        case DICA_PRIMO:
            return ehPrimo(secreto);
        case DICA_MULT5:
            return (secreto % 5) == 0;
        case DICA_MULT3:
            return (secreto % 3) == 0;
        case DICA_MULT10:
            return (secreto % 10) == 0;
        case DICA_MULT25:
            return (secreto % 25) == 0;
        case DICA_QUADRADO:
            return ehQuadradoPerfeito(secreto);
        case DICA_SOMA_DIGITOS:
            return 1;
        default:
            return 0;
    }
}

static void gerarDica(int idCaso, int secreto, char *saida, size_t tamanho) {
    TipoDica dicas[10];
    int qtdDicas = 0;
    int dicasValidas[10];
    int qtdValidas = 0;

    if (idCaso == 1) {
        dicas[qtdDicas++] = DICA_PAR;
        dicas[qtdDicas++] = DICA_IMPAR;
        dicas[qtdDicas++] = DICA_PRIMO;
        dicas[qtdDicas++] = DICA_MULT5;
        dicas[qtdDicas++] = DICA_MULT3;
        dicas[qtdDicas++] = DICA_QUADRADO;
        dicas[qtdDicas++] = DICA_SOMA_DIGITOS;
    } else if (idCaso == 2) {
        dicas[qtdDicas++] = DICA_PAR;
        dicas[qtdDicas++] = DICA_IMPAR;
        dicas[qtdDicas++] = DICA_PRIMO;
        dicas[qtdDicas++] = DICA_MULT5;
        dicas[qtdDicas++] = DICA_MULT10;
        dicas[qtdDicas++] = DICA_QUADRADO;
        dicas[qtdDicas++] = DICA_SOMA_DIGITOS;
    } else {
        dicas[qtdDicas++] = DICA_PAR;
        dicas[qtdDicas++] = DICA_IMPAR;
        dicas[qtdDicas++] = DICA_PRIMO;
        dicas[qtdDicas++] = DICA_MULT10;
        dicas[qtdDicas++] = DICA_MULT25;
        dicas[qtdDicas++] = DICA_QUADRADO;
        dicas[qtdDicas++] = DICA_SOMA_DIGITOS;
    }

    for (int i = 0; i < qtdDicas; i++) {
        if (dicaCombina(dicas[i], secreto)) {
            dicasValidas[qtdValidas++] = dicas[i];
        }
    }

    if (qtdValidas == 0) {
        snprintf(saida, tamanho, "DICA DO PERITO: Sem pistas confiaveis no momento.");
        return;
    }

    TipoDica escolhida = (TipoDica)dicasValidas[rand() % qtdValidas];

    if (idCaso == 1) {
        switch (escolhida) {
            case DICA_PAR:
                snprintf(saida, tamanho, "PERITO: Ajustei os oculos termicos... A assinatura se divide em dois grupos exatos. E um numero PAR.");
                break;
            case DICA_IMPAR:
                snprintf(saida, tamanho, "PERITO: O laudo indica algo incomum na digital: o peso nos botoes nao se divide em grupos iguais. E um numero IMPAR.");
                break;
            case DICA_PRIMO:
                snprintf(saida, tamanho, "PERITO: Dado do laboratorio: a pressao no botao e solitaria. O codigo so e divisivel por ele mesmo e por 1. Temos um numero PRIMO.");
                break;
            case DICA_MULT5:
                snprintf(saida, tamanho, "PERITO: O fotografo forense anotou um padrao: as marcas de calor do Alinho saltam de cinco em cinco no teclado.");
                break;
            case DICA_MULT3:
                snprintf(saida, tamanho, "PERITO: O laudo indica: as manchas formam um triangulo, a matematica do codigo se divide perfeitamente em tres setores da cena.");
                break;
            case DICA_QUADRADO:
                snprintf(saida, tamanho, "PERITO: Analise geometrica do impacto: o calor nos botoes forma um padrao de quadrado perfeito matematico.");
                break;
            case DICA_SOMA_DIGITOS:
                snprintf(saida, tamanho, "PERITO: Calculei em voz alta aqui no laboratorio: a soma dos algarismos desse codigo da exatamente %d.", somaDigitos(secreto));
                break;
            default:
                snprintf(saida, tamanho, "DICA DO PERITO: Sem pistas confiaveis no momento.");
                break;
        }
    } else if (idCaso == 2) {
        switch (escolhida) {
            case DICA_PAR:
                snprintf(saida, tamanho, "PERITO: O sinal do radio do mopretu aparece no osciloscopio em pares simetricos. E uma frequencia PAR.");
                break;
            case DICA_IMPAR:
                snprintf(saida, tamanho, "PERITO: O sinal nao forma pares na antena. A onda opera numa frequencia IMPAR.");
                break;
            case DICA_PRIMO:
                snprintf(saida, tamanho, "PERITO: Dado incomum na analise espectral: a onda de transmissao e um numero primo. O sinal nao se divide, nao sofre interferencia.");
                break;
            case DICA_MULT5:
                snprintf(saida, tamanho, "PERITO: Os picos de radio saltam em blocos de 5 MHz ao longo do espectro. Padrao identificado.");
                break;
            case DICA_MULT10:
                snprintf(saida, tamanho, "PERITO: Sinal limpo demais para ser coincidencia: a frequencia de fuga deles fecha em grupos exatos de dez.");
                break;
            case DICA_QUADRADO:
                snprintf(saida, tamanho, "PERITO: Padrao de onda detectado no software: a distribuicao da frequencia forma um quadrado perfeito matematico.");
                break;
            case DICA_SOMA_DIGITOS:
                snprintf(saida, tamanho, "PERITO: Analise concluida. A soma dos digitos dessa frequencia do mopretu e %d.", somaDigitos(secreto));
                break;
            default:
                snprintf(saida, tamanho, "DICA DO PERITO: Sem pistas confiaveis no momento.");
                break;
        }
    } else {
        switch (escolhida) {
            case DICA_PAR:
                snprintf(saida, tamanho, "PERITO: Tracei a rota do virus nos logs: o trafego de rede se divide em dois trechos exatamente iguais. A porta e PAR.");
                break;
            case DICA_IMPAR:
                snprintf(saida, tamanho, "PERITO: A trilha de dados nao e simetrica - o protocolo de rede esta operando em um numero IMPAR.");
                break;
            case DICA_PRIMO:
                snprintf(saida, tamanho, "PERITO: Detalhe tecnico identificado na criptografia do CH do Pina: o endereco da porta e um numero primo. Indivisivel e letal.");
                break;
            case DICA_MULT10:
                snprintf(saida, tamanho, "PERITO: Confirmado nos logs do data center: os pacotes maliciosos pulam em multiplos exatos de dez.");
                break;
            case DICA_MULT25:
                snprintf(saida, tamanho, "PERITO: Rastreio preciso concluido: o trafego do virus se encaixa perfeitamente nos blocos de 25 portas do servidor.");
                break;
            case DICA_QUADRADO:
                snprintf(saida, tamanho, "PERITO: Curiosidade encontrada no codigo-fonte: o numero da porta infectada pelo CH e um quadrado perfeito.");
                break;
            case DICA_SOMA_DIGITOS:
                snprintf(saida, tamanho, "PERITO: Anotado no terminal seguro: a soma dos algarismos dessa maldita porta e %d.", somaDigitos(secreto));
                break;
            default:
                snprintf(saida, tamanho, "DICA DO PERITO: Sem pistas confiaveis no momento.");
                break;
        }
    }
}

static void exibirRelatorio(int idCaso) {
    limparTela();
    printf("\n");
    printf(VERMELHO "  CASO %02d: ", idCaso);
    
    if (idCaso == 1) {
        printf("O ULTIMO SUSPIRO DO MAGNATA\n\n" RESET);
        printf("  Dificuldade: Facil\n");
        printf("  Tentativas disponiveis: 7\n\n");
        printf("  RELATORIO DE CAMPO: \"Detetive, a cena e caotica. O bilionario Gaga de\n");
        printf("  Big Field foi encontrado morto em sua biblioteca particular na sua\n");
        printf("  Mansao. O cofre da parede esta aberto e vazio, mas ha um detalhe: o\n");
        printf("  assassino, conhecido como Alinho do Goita, nao levou as barras de\n");
        printf("  ouro, apenas um pendrive contendo as chaves de acesso das Industrias\n");
        printf("  Gaga. O sistema de seguranca da mansao registrou um acesso forcado no\n");
        printf("  teclado numerico do cofre momentos antes do crime. Usamos luz\n");
        printf("  ultravioleta e detectamos uma assinatura de calor residual em um\n");
        printf("  codigo especifico. Infelizmente, o teclado e de tecnologia antiga e\n");
        printf("  so aceita codigos entre 1 e 50.\n\n");
        printf("  A MISSAO: A digital esta desaparecendo conforme o teclado esfria.\n");
        printf("  Voce precisa descobrir o codigo exato que o assassino usou para abrir\n");
        printf("  o cofre. Se falhar apos 7 tentativas, o sistema travara\n");
        printf("  permanentemente e a trilha do assassino ficara fria para sempre.\"\n");
    } 
    else if (idCaso == 2) {
        printf("FREQUENCIA DE FUGA NO CASSINO\n\n" RESET);
        printf("  Dificuldade: Medio\n");
        printf("  Tentativas disponiveis: 6 | Intervalo: 1 a 100\n\n");
        printf("  RELATORIO DE CAMPO: \"Estamos no meio de um assalto em andamento em\n");
        printf("  Santo Amaro. Uma gangue de alta tecnologia, liderada por mopretu,\n");
        printf("  invadiu a central de seguranca e instalou um bloqueador de sinal\n");
        printf("  potente. Neste exato momento, eles estao no heliponto do predio,\n");
        printf("  esperando o helicoptero de resgate que chegara em poucos minutos.\n");
        printf("  Nossas comunicacoes estao cortadas. Para derrubar o bloqueador e\n");
        printf("  pedir reforcos aereos, voce deve usar o scanner de radio portatil\n");
        printf("  da policia e sintonizar na frequencia exata que a gangue esta\n");
        printf("  usando para coordenar a fuga. Nossos tecnicos confirmaram que o\n");
        printf("  sinal opera em uma faixa de 1 a 100 MHz.\n\n");
        printf("  A MISSAO: O tempo e o seu pior inimigo. O helicoptero de fuga\n");
        printf("  pousara em breve. Se voce nao encontrar a frequencia correta em\n");
        printf("  ate 6 tentativas, o sinal sera criptografado novamente e os\n");
        printf("  criminosos escaparao com milhoes em diamantes.\"\n");
    } 
    else {
        printf("PROTOCOLO APOCALIPSE\n\n" RESET);
        printf("  Dificuldade: Dificil\n");
        printf("  Tentativas disponiveis: 5 | Intervalo: 1 a 200\n\n");
        printf("  RELATORIO DE CAMPO: \"Situacao de emergencia maxima, detetive. O\n");
        printf("  centro de controle da rede eletrica do Pina foi invadido por um\n");
        printf("  cyber-terrorista conhecido como 'CH do Pina'. Ele injetou um\n");
        printf("  malware de autodestruicao que causara um apagao total em todo o\n");
        printf("  estado em menos de 10 minutos. O virus esta escondido em uma\n");
        printf("  porta de rede (Port) camuflada dentro do servidor central. O\n");
        printf("  firewall do sistema foi corrompido, mas o agente Abedalama\n");
        printf("  conseguiu isolar a ameaca em um range de portas entre 1 e 200.\n");
        printf("  Cada tentativa errada dispara um pulso de sobrecarga que queima\n");
        printf("  os fusveis da central.\n\n");
        printf("  MISSAO: Voce tem apenas 5 tentativas antes que o hardware do\n");
        printf("  servidor derreta por completo, mergulhando a cidade na escuridao\n");
        printf("  e causando o caos total. O destino da infraestrutura do estado\n");
        printf("  esta nas suas maos. Encontre a porta correta e neutralize o\n");
        printf("  protocolo.\"\n");
    }
    
    printf("\n  " VERDE "[Pressione ENTER para iniciar a investigacao]" RESET);
    getchar();
    
    limparTela();
    printf("\n  Caso carregado. Prepare-se, Detetive.\n");
    printf("\n  " VERDE "[Pressione ENTER para comecar]" RESET);
    getchar();
}

void jogarPartida(int idCaso) {
    if (idCaso < 1 || idCaso > 3) {
        printf(VERMELHO "  Erro: id de caso invalido (%d).\n" RESET, idCaso);
        pausar();
        return;
    }

    exibirRelatorio(idCaso);
    
    int maxVal = (idCaso == 1) ? 50 : (idCaso == 2) ? 100 : 200;
    int maxTentativas = (idCaso == 1) ? 7 : (idCaso == 2) ? 6 : 5;
    int dicasUsadas = 0;
    
    int secreto = 1 + (rand() % maxVal);
    int tentativas = maxTentativas;
    
    int palpitesDados[10]; 
    int contPalpites = 0;
    char feedback[200] = "Colete pistas para construir sua investigacao antes de fazer um palpite final.";
    
    // Inicializar o sistema de pistas
    BancoPistas banco;
    inicializarBancoPistas(idCaso, secreto, &banco);
    
    Sessao s;
    if (idCaso == 1) {
        strcpy(s.casoNome, "O Ultimo Suspiro do Magnata");
        strcpy(s.dificuldade, "Facil");
    } else if (idCaso == 2) {
        strcpy(s.casoNome, "Frequencia de Fuga no Cassino");
        strcpy(s.dificuldade, "Medio");
    } else {
        strcpy(s.casoNome, "Protocolo Apocalipse");
        strcpy(s.dificuldade, "Dificil");
    }
    s.secreto = secreto;
    s.venceu = 0;

    int palpite = 0;

    while (tentativas > 0) {
        limparTela();
        printf("\n");
        printf(VERMELHO "  INVESTIGACAO ATIVA: CASO 0%d\n" RESET, idCaso);
        printf(AMARELO "  ! TENTATIVAS RESTANTES: %d/%d\n" RESET, tentativas, maxTentativas);
        printf("  ============================================================\n\n");
        
        printf(CIANO "  >> Digite o codigo (1 a %d)\n" RESET, maxVal);
        printf(AMARELO "  (Digite 0 para coletar uma pista)\n");
        printf(AMARELO "  (Digite 9 para ver historico de pistas)\n\n" RESET);
        
        // Mostrar aviso se nao coletou minimo de pistas
        if (!verificarMinimoAceitacao(&banco)) {
            printf(VERMELHO "  *** AVISO CRITICO ***\n");
            printf("  Voce PRECISA coletar pelo menos %d pistas antes de fazer um palpite!\n", banco.minimoRequired);
            printf("  Pistas coletadas: %d/%d\n" RESET, banco.pistasColetadas, banco.minimoRequired);
            printf(AMARELO "  Pressione 0 para coletar uma pista agora.\n\n" RESET);
        }
        
        printf("  FEEDBACK DO SISTEMA:\n");
        printf("  \"%s\"\n\n", feedback);
        printf("  > ");
        
        palpite = lerOpcao(0, maxVal);

        // Opção para coletar pista
        if (palpite == 0) {
            apresentarPista(&banco, secreto);
            
            // Ajustar reputação geral quando coleta pista
            if (banco.pistasColetadas == banco.minimoRequired) {
                strcpy(feedback, "Excelente! Voce coletou evidencia suficiente para prosseguir.");
            } else {
                strcpy(feedback, "Uma nova pista foi adicionada ao seu relatorio...");
            }
            pausar();
            continue;
        }
        
        // Opção para ver histórico de pistas
        if (palpite == 9) {
            exibirHistoricoPistas(&banco);
            strcpy(feedback, "Revisto o historico de pistas.");
            pausar();
            continue;
        }

        // VALIDACAO CRITICA: Rejeitar palpite se nao tiver minimo de pistas
        if (!verificarMinimoAceitacao(&banco)) {
            strcpy(feedback, "SISTEMA BLOQUEADO: Nao posso permitir um palpite sem evidencia suficiente!");
            pausar();
            continue;
        }

        palpitesDados[contPalpites] = palpite;
        contPalpites++;

        if (palpite == secreto) {
            s.venceu = 1;
            break;
        } else {
            if (palpite < secreto) {
                strcpy(feedback, "A assinatura termica aponta para cima. O codigo e MAIOR.");
            } else {
                strcpy(feedback, "A assinatura termica esfriou. O codigo e MENOR.");
            }
            tentativas--;
        }
        
        printf("  ============================================================\n\n");
        printf("  LOG DE TENTATIVAS:\n");
        for(int i = 0; i < contPalpites; i++) {
            printf("  [%d] %d\n", i+1, palpitesDados[i]);
        }
        pausar();
    }

    s.tentativasUsadas = contPalpites;

    limparTela();
    printf("\n  ============================================================\n");
    printf(VERMELHO "                  RESULTADO DA INVESTIGACAO\n\n" RESET);
    
    if (s.venceu == 1) {
        printf(VERDE "              [V] *** CASO RESOLVIDO COM SUCESSO ***\n\n" RESET);
        printf("  SUCESSO! O codigo %d foi confirmado. Excelente trabalho!\n\n", secreto);
    } else {
        printf(VERMELHO "              [X] *** INVESTIGACAO FALHOU ***\n\n" RESET);
        printf("  O sistema travou. O codigo verdadeiro era %d. O assassino escapou.\n\n", secreto);
    }

    printf(CIANO "  --- RELATORIO DE DESEMPENHO ---\n" RESET);
    printf("  Caso: 0%d\n", idCaso);
    printf("  Tentativas usadas: %d de %d\n", s.tentativasUsadas, maxTentativas);
    printf("  Pistas coletadas: %d pistas\n", banco.pistasColetadas);
    
    // Exibir sumário de pistas no final
    exibirHistoricoPistas(&banco);
    
    printf(VERDE "\n  [V] Registro salvo no banco de dados do departamento.\n" RESET);
    
    salvarSessao(s);
    pausar();
}
