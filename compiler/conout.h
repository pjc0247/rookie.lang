#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#ifdef _WIN32
#define CON_BLACK           0
#define CON_BLUE            1
#define CON_GREEN           2
#define CON_CYAN            3
#define CON_RED             4
#define CON_MAGENTA         5
#define CON_BROWN           6
#define CON_LIGHTGRAY       7
#define CON_DARKGRAY        8
#define CON_LIGHTBLUE       9
#define CON_LIGHTGREEN      10
#define CON_LIGHTCYAN       11
#define CON_LIGHTRED        12
#define CON_LIGHTMAGENTA    13
#define CON_YELLOW          14
#define CON_WHITE           15
#else
#define CON_BLACK           30
#define CON_BLUE            34
#define CON_GREEN           2
#define CON_CYAN            36
#define CON_RED             31
#define CON_MAGENTA         35
#define CON_BROWN           6
#define CON_LIGHTGRAY       30
#define CON_DARKGRAY        30
#define CON_LIGHTBLUE       34
#define CON_LIGHTGREEN      32
#define CON_LIGHTCYAN       36
#define CON_LIGHTRED        31
#define CON_LIGHTMAGENTA    35
#define CON_YELLOW          33
#define CON_WHITE           37
#endif

class con {
public:
    static void setColor(int color) {
#if _WIN32
        SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
#else
        printf("\033[0;%dm", color);
#endif
    }
};
