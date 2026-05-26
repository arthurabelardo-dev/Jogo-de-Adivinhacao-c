#ifndef TUI_H
#define TUI_H

#define UI_RESET      "\033[0m"
#define UI_BOLD       "\033[1m"
#define UI_DIM        "\033[2m"
#define UI_RED        "\033[1;31m"
#define UI_GREEN      "\033[1;32m"
#define UI_YELLOW     "\033[1;33m"
#define UI_BLUE       "\033[1;34m"
#define UI_MAGENTA    "\033[1;35m"
#define UI_CYAN       "\033[1;36m"
#define UI_WHITE      "\033[1;37m"
#define UI_BG_BLUE    "\033[44m"
#define UI_BG_RED     "\033[41m"

void uiInit(void);
void uiClear(void);
void uiPause(const char *message);
void uiSleepMs(int ms);
void uiTypeLine(const char *text, int msPerChar);
void uiRule(const char *color);
void uiSection(const char *title, const char *color);
void uiStamp(const char *left, const char *right, const char *color);
void uiLogo(void);
void uiBanner(const char *title, const char *subtitle);
void uiLoading(const char *label, int steps, int ms);
void uiBoxTop(void);
void uiBoxMid(const char *label, const char *value, const char *color);
void uiBoxText(const char *text);
void uiBoxWrap(const char *text, const char *color);
void uiBoxBottom(void);
void uiMenuItem(int number, const char *title, const char *meta, const char *status, const char *color);
void uiMeter(const char *label, int value, int max, const char *color);
void uiScanBar(int value, int min, int max);
void uiPrompt(const char *label);
void uiAlert(const char *tag, const char *message, const char *color);

#endif
