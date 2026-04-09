# Jogo de Adivinhação – Investigação Criminal

## Descrição

Este projeto consiste em um jogo de adivinhação desenvolvido em linguagem C, com temática de investigação criminal. O jogador assume o papel de um detetive e deve descobrir um número secreto por meio de tentativas, utilizando pistas fornecidas pelo sistema.

O jogo vai além da mecânica tradicional, incorporando análise de desempenho e sugestões estratégicas, tornando a experiência mais interativa e educativa.

---

## Objetivo

* Desenvolver um jogo interativo em C
* Aplicar conceitos de lógica de programação
* Utilizar arquivos para persistência de dados
* Implementar análise estatística básica
* Explorar o uso de recursão

---

## Funcionalidades

### Jogabilidade

* Geração de número aleatório
* Sistema de tentativas com feedback (maior/menor)
* Níveis de dificuldade (casos)
* Interface no terminal com cores
* Sistema de pontuação

### Análise de Dados

* Registro de partidas em arquivo
* Cálculo de média de tentativas
* Melhor e pior desempenho
* Cálculo de desvio padrão (recursivo)
* Sugestões estratégicas ao jogador

---

## Temática

O jogo é baseado em investigação criminal, onde:

* Cada partida é um “caso”
* Cada tentativa representa uma análise de evidência
* O objetivo é “resolver o caso” com eficiência

---

## Como Executar

### Compilação

bash
gcc main.c -o jogo


### Execução

bash
./jogo


---

## Estrutura do Projeto (sugestão)


/projeto
 ├── main.c
 ├── jogo.c
 ├── jogo.h
 ├── stats.c
 ├── stats.h
 ├── historico.txt
 └── README.md


---

## Conceitos Utilizados

* Estruturas de repetição
* Funções
* Manipulação de arquivos
* Números aleatórios
* Recursão
* Estatística básica

---

## Exemplo de Saída


Caso iniciado...
Digite um número: 50
A evidência indica que o número é menor...

Digite um número: 25
Muito baixo...

Caso resolvido!
Tentativas: 5
Pontuação: 120


---

## Melhorias Futuras

* Ranking de jogadores
* Interface gráfica
* Modo multiplayer
* Inteligência adaptativa

---

## Integrantes

* Nome 1
* Nome 2

---

## Observação

Projeto desenvolvido para fins acadêmicos, com foco no aprendizado de lógica de programação e análise de dados.
