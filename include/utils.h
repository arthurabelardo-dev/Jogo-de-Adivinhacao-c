#ifndef UTILS_H
#define UTILS_H

#define VERMELHO  "\033[1;31m"
#define AMARELO   "\033[1;33m"
#define VERDE     "\033[1;32m"
#define CIANO     "\033[1;36m"
#define RESET     "\033[0m"

#ifdef _WIN32
  #define CLEAR "cls"
#else
  #define CLEAR "clear"
#endif

void limparTela(void);
void pausar(void);
int  lerOpcao(int min, int max);
int  lerInteiroIntervalo(int min, int max, const char *prompt);

#endif
