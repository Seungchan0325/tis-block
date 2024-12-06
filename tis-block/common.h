#pragma once

#include <Windows.h>
#include <string>
#include <cassert>

#define FOREGROUND_WHITE (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)

enum class Align {
    LeftTop,
    CenterTop,
    RightTop,
    LeftCenter,
    Center,
    RightCenter,
    LeftBottom,
    CenterBottom,
    RightBottom
};

extern bool isRunning;
extern int fps;
extern long long ticksPerSec;

void ltrim(std::string& str);

void rtrim(std::string& str);

void trim(std::string& str);

void ErrorExit(std::string message);

long long GetTicks();

bool InRect(COORD pos, SMALL_RECT rect);

void WriteText(std::string text, SMALL_RECT rect, Align align);