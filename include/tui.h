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

void ui_init(void);
void ui_clear(void);
void ui_pause(const char *message);
void ui_sleep_ms(int ms);
void ui_type_line(const char *text, int ms_per_char);
void ui_rule(const char *color);
void ui_section(const char *title, const char *color);
void ui_stamp(const char *left, const char *right, const char *color);
void ui_logo(void);
void ui_banner(const char *title, const char *subtitle);
void ui_loading(const char *label, int steps, int ms);
void ui_box_top(void);
void ui_box_mid(const char *label, const char *value, const char *color);
void ui_box_text(const char *text);
void ui_box_wrap(const char *text, const char *color);
void ui_box_bottom(void);
void ui_menu_item(int number, const char *title, const char *meta, const char *status, const char *color);
void ui_meter(const char *label, int value, int max, const char *color);
void ui_scan_bar(int value, int min, int max);
void ui_prompt(const char *label);
void ui_alert(const char *tag, const char *message, const char *color);

#endif
