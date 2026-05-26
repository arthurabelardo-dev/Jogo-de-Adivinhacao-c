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

static void printPadded(const char *text) {
    char visible[UI_WIDTH];
    int maxLen = UI_WIDTH - 4;
    int len = (int)strlen(text);
    int pad;

    if (len > maxLen) {
        strncpy(visible, text, (size_t)maxLen - 2);
        visible[maxLen - 2] = '.';
        visible[maxLen - 1] = '.';
        visible[maxLen] = '\0';
        text = visible;
        len = maxLen;
    }

    pad = UI_WIDTH - 4 - len;

    if (pad < 0) {
        pad = 0;
    }

    printf("  | %s%*s |\n", text, pad, "");
}

void uiInit(void) {
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

void uiClear(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}

void uiPause(const char *message) {
    printf("\n  %s%s%s", UI_DIM, message ? message : "Pressione ENTER para continuar...", UI_RESET);
    fflush(stdout);
    getchar();
}

void uiSleepMs(int ms) {
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

void uiTypeLine(const char *text, int msPerChar) {
    const char *p = text;

    while (*p != '\0') {
        putchar(*p);
        fflush(stdout);
        uiSleepMs(msPerChar);
        p++;
    }
    putchar('\n');
}

void uiRule(const char *color) {
    printf("  %s========================================================================%s\n", color ? color : "", UI_RESET);
}

void uiSection(const char *title, const char *color) {
    printf("\n  %s-- %s ", color ? color : UI_CYAN, title);
    for (int i = (int)strlen(title); i < 62; i++) {
        putchar('-');
    }
    printf("%s\n", UI_RESET);
}

void uiStamp(const char *left, const char *right, const char *color) {
    int leftLen = (int)strlen(left);
    int rightLen = (int)strlen(right);
    int pad = UI_WIDTH - 4 - leftLen - rightLen;

    if (pad < 1) {
        pad = 1;
    }

    printf("  %s[%s]%*s[%s]%s\n", color ? color : UI_DIM, left, pad, "", right, UI_RESET);
}

void uiLogo(void) {
    printf("  %s   ____      ____ ____  ___ __  __ ___ _   _    _    _     %s\n", UI_CYAN, UI_RESET);
    printf("  %s  / ___|    / ___|  _ \\|_ _|  \\/  |_ _| \\ | |  / \\  | |    %s\n", UI_CYAN, UI_RESET);
    printf("  %s | |   _____| |   | |_) || || |\\/| || ||  \\| | / _ \\ | |    %s\n", UI_CYAN, UI_RESET);
    printf("  %s | |__|_____| |___|  _ < | || |  | || || |\\  |/ ___ \\| |___ %s\n", UI_CYAN, UI_RESET);
    printf("  %s  \\____|     \\____|_| \\_\\___|_|  |_|___|_| \\_/_/   \\_\\_____|%s\n", UI_CYAN, UI_RESET);
}

void uiBanner(const char *title, const char *subtitle) {
    uiRule(UI_CYAN);
    printf("  %s>> %-64s%s\n", UI_BOLD, title, UI_RESET);
    if (subtitle != NULL && subtitle[0] != '\0') {
        printf("  %s   %s%s\n", UI_DIM, subtitle, UI_RESET);
    }
    uiRule(UI_CYAN);
}

void uiLoading(const char *label, int steps, int ms) {
    printf("  %s%s%s [", UI_DIM, label, UI_RESET);
    fflush(stdout);
    for (int i = 0; i < steps; i++) {
        putchar('#');
        fflush(stdout);
        uiSleepMs(ms);
    }
    printf("] %sOK%s\n", UI_GREEN, UI_RESET);
}

void uiBoxTop(void) {
    printf("  +--------------------------------------------------------------------+\n");
}

void uiBoxMid(const char *label, const char *value, const char *color) {
    char line[96];
    snprintf(line, sizeof(line), "%-18s %s", label, value);
    printf("%s", color ? color : "");
    printPadded(line);
    printf("%s", UI_RESET);
}

void uiBoxText(const char *text) {
    printPadded(text);
}

void uiBoxWrap(const char *text, const char *color) {
    char line[UI_WIDTH];
    int lineLen = 0;
    int maxLen = UI_WIDTH - 4;
    const char *p = text;

    printf("%s", color ? color : "");
    while (*p != '\0') {
        char word[48];
        int wordLen = 0;

        while (*p == ' ') {
            p++;
        }
        while (*p != '\0' && *p != ' ' && wordLen < (int)sizeof(word) - 1) {
            word[wordLen++] = *p++;
        }
        word[wordLen] = '\0';

        if (wordLen == 0) {
            break;
        }

        if (lineLen > 0 && lineLen + wordLen + 1 > maxLen) {
            line[lineLen] = '\0';
            printPadded(line);
            lineLen = 0;
        }

        if (lineLen > 0) {
            line[lineLen++] = ' ';
        }

        strncpy(line + lineLen, word, (size_t)(maxLen - lineLen));
        lineLen += wordLen;
        if (lineLen > maxLen) {
            lineLen = maxLen;
        }
    }

    if (lineLen > 0) {
        line[lineLen] = '\0';
        printPadded(line);
    }
    printf("%s", UI_RESET);
}

void uiBoxBottom(void) {
    printf("  +--------------------------------------------------------------------+\n");
}

void uiMenuItem(int number, const char *title, const char *meta, const char *status, const char *color) {
    char line[96];

    snprintf(line, sizeof(line), "[%d] %-19s %-28s %s", number, title, meta, status);
    printf("%s", color ? color : "");
    printPadded(line);
    printf("%s", UI_RESET);
}

void uiMeter(const char *label, int value, int max, const char *color) {
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

void uiScanBar(int value, int min, int max) {
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

void uiPrompt(const char *label) {
    printf("\n  %s[%s]%s > ", UI_CYAN, label, UI_RESET);
    fflush(stdout);
}

void uiAlert(const char *tag, const char *message, const char *color) {
    printf("  %s[%s]%s %s\n", color ? color : UI_YELLOW, tag, UI_RESET, message);
}
