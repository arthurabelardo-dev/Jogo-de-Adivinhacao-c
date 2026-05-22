#ifndef _WIN32
#define _POSIX_C_SOURCE 199309L
#endif

#include <stdio.h>
#include <string.h>
#include <locale.h>
#include "tui.h"

#ifdef _WIN32
#include <windows.h>
#include <winnls.h>
#ifndef ENABLE_VIRTUAL_TERMINAL_PROCESSING
#define ENABLE_VIRTUAL_TERMINAL_PROCESSING 0x0004
#endif
#else
#include <time.h>
#endif

#define UI_WIDTH 72

static void print_padded(const char *text) {
    char visible[UI_WIDTH];
    int max_len = UI_WIDTH - 4;
    int len = (int)strlen(text);
    int pad;

    if (len > max_len) {
        strncpy(visible, text, (size_t)max_len - 2);
        visible[max_len - 2] = '.';
        visible[max_len - 1] = '.';
        visible[max_len] = '\0';
        text = visible;
        len = max_len;
    }

    pad = UI_WIDTH - 4 - len;

    if (pad < 0) {
        pad = 0;
    }

    printf("  | %s%*s |\n", text, pad, "");
}

void ui_init(void) {
    setlocale(LC_ALL, "");

#ifdef _WIN32
    HANDLE out = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD mode = 0;

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    if (out != INVALID_HANDLE_VALUE && GetConsoleMode(out, &mode)) {
        SetConsoleMode(out, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#endif
}

void ui_clear(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void ui_pause(const char *message) {
    printf("\n  %s%s%s", UI_DIM, message ? message : "Pressione ENTER para continuar...", UI_RESET);
    fflush(stdout);
    getchar();
}

void ui_sleep_ms(int ms) {
    if (ms <= 0) {
        return;
    }

#ifdef _WIN32
    Sleep((DWORD)ms);
#else
    struct timespec ts;
    ts.tv_sec = ms / 1000;
    ts.tv_nsec = (long)(ms % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#endif
}

void ui_type_line(const char *text, int ms_per_char) {
    const char *p = text;

    while (*p != '\0') {
        putchar(*p);
        fflush(stdout);
        ui_sleep_ms(ms_per_char);
        p++;
    }
    putchar('\n');
}

void ui_rule(const char *color) {
    printf("  %s========================================================================%s\n", color ? color : "", UI_RESET);
}

void ui_section(const char *title, const char *color) {
    printf("\n  %s-- %s ", color ? color : UI_CYAN, title);
    for (int i = (int)strlen(title); i < 62; i++) {
        putchar('-');
    }
    printf("%s\n", UI_RESET);
}

void ui_stamp(const char *left, const char *right, const char *color) {
    int left_len = (int)strlen(left);
    int right_len = (int)strlen(right);
    int pad = UI_WIDTH - 4 - left_len - right_len;

    if (pad < 1) {
        pad = 1;
    }

    printf("  %s[%s]%*s[%s]%s\n", color ? color : UI_DIM, left, pad, "", right, UI_RESET);
}

void ui_logo(void) {
    printf("  %s   ____      ____ ____  ___ __  __ ___ _   _    _    _     %s\n", UI_CYAN, UI_RESET);
    printf("  %s  / ___|    / ___|  _ \\|_ _|  \\/  |_ _| \\ | |  / \\  | |    %s\n", UI_CYAN, UI_RESET);
    printf("  %s | |   _____| |   | |_) || || |\\/| || ||  \\| | / _ \\ | |    %s\n", UI_CYAN, UI_RESET);
    printf("  %s | |__|_____| |___|  _ < | || |  | || || |\\  |/ ___ \\| |___ %s\n", UI_CYAN, UI_RESET);
    printf("  %s  \\____|     \\____|_| \\_\\___|_|  |_|___|_| \\_/_/   \\_\\_____|%s\n", UI_CYAN, UI_RESET);
}

void ui_banner(const char *title, const char *subtitle) {
    ui_rule(UI_CYAN);
    printf("  %s>> %-64s%s\n", UI_BOLD, title, UI_RESET);
    if (subtitle != NULL && subtitle[0] != '\0') {
        printf("  %s   %s%s\n", UI_DIM, subtitle, UI_RESET);
    }
    ui_rule(UI_CYAN);
}

void ui_loading(const char *label, int steps, int ms) {
    printf("  %s%s%s [", UI_DIM, label, UI_RESET);
    fflush(stdout);
    for (int i = 0; i < steps; i++) {
        putchar('#');
        fflush(stdout);
        ui_sleep_ms(ms);
    }
    printf("] %sOK%s\n", UI_GREEN, UI_RESET);
}

void ui_box_top(void) {
    printf("  +--------------------------------------------------------------------+\n");
}

void ui_box_mid(const char *label, const char *value, const char *color) {
    char line[96];
    snprintf(line, sizeof(line), "%-18s %s", label, value);
    printf("%s", color ? color : "");
    print_padded(line);
    printf("%s", UI_RESET);
}

void ui_box_text(const char *text) {
    print_padded(text);
}

void ui_box_wrap(const char *text, const char *color) {
    char line[UI_WIDTH];
    int line_len = 0;
    int max_len = UI_WIDTH - 4;
    const char *p = text;

    printf("%s", color ? color : "");
    while (*p != '\0') {
        char word[48];
        int word_len = 0;

        while (*p == ' ') {
            p++;
        }
        while (*p != '\0' && *p != ' ' && word_len < (int)sizeof(word) - 1) {
            word[word_len++] = *p++;
        }
        word[word_len] = '\0';

        if (word_len == 0) {
            break;
        }

        if (line_len > 0 && line_len + word_len + 1 > max_len) {
            line[line_len] = '\0';
            print_padded(line);
            line_len = 0;
        }

        if (line_len > 0) {
            line[line_len++] = ' ';
        }

        strncpy(line + line_len, word, (size_t)(max_len - line_len));
        line_len += word_len;
        if (line_len > max_len) {
            line_len = max_len;
        }
    }

    if (line_len > 0) {
        line[line_len] = '\0';
        print_padded(line);
    }
    printf("%s", UI_RESET);
}

void ui_box_bottom(void) {
    printf("  +--------------------------------------------------------------------+\n");
}

void ui_menu_item(int number, const char *title, const char *meta, const char *status, const char *color) {
    char line[96];

    snprintf(line, sizeof(line), "[%d] %-19s %-28s %s", number, title, meta, status);
    printf("%s", color ? color : "");
    print_padded(line);
    printf("%s", UI_RESET);
}

void ui_meter(const char *label, int value, int max, const char *color) {
    int filled;
    int width = 30;

    if (max <= 0) {
        max = 1;
    }
    if (value < 0) {
        value = 0;
    }
    if (value > max) {
        value = max;
    }

    filled = (value * width) / max;
    printf("  %-18s %s[", label, color ? color : "");
    for (int i = 0; i < width; i++) {
        putchar(i < filled ? '#' : '.');
    }
    printf("] %d/%d%s\n", value, max, UI_RESET);
}

void ui_scan_bar(int value, int min, int max) {
    int width = 52;
    int position;

    if (max <= min) {
        max = min + 1;
    }
    if (value < min) {
        value = min;
    }
    if (value > max) {
        value = max;
    }

    position = ((value - min) * (width - 1)) / (max - min);

    printf("  %sSCAN FORENSE%s  %d ", UI_CYAN, UI_RESET, min);
    printf("[");
    for (int i = 0; i < width; i++) {
        putchar(i == position ? 'X' : '-');
    }
    printf("] %d\n", max);
}

void ui_prompt(const char *label) {
    printf("\n  %s[%s]%s > ", UI_CYAN, label, UI_RESET);
    fflush(stdout);
}

void ui_alert(const char *tag, const char *message, const char *color) {
    printf("  %s[%s]%s %s\n", color ? color : UI_YELLOW, tag, UI_RESET, message);
}
