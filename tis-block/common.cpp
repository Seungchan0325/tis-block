#include <Windows.h>

#include "common.h"

#include "Screen.h"
#include "SceneStartup.h"
#include "SceneMain.h"
#include "SceneInGame.h"
#include "SceneShutdown.h"

bool isRunning;
int fps;
long long ticksPerSec;

void ErrorExit(std::string message)
{
	ScreenShutdown();
	MessageBoxA(NULL, message.c_str(), "Error", NULL);
	ExitProcess(0);
}

void ltrim(std::string& str)
{
    str.erase(str.begin(), std::find_if(str.begin(), str.end(), [](unsigned char ch) {
        return !std::isspace(ch);
	}));
}

void rtrim(std::string& str)
{
	str.erase(std::find_if(str.rbegin(), str.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
    }).base(), str.end());
}

void trim(std::string& str)
{
    ltrim(str);
    rtrim(str);
}

long long GetTicks()
{
    LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
    return ticks.QuadPart;
}

bool InRect(COORD pos, SMALL_RECT rect)
{
	return pos.X >= rect.Left && pos.X <= rect.Right && pos.Y >= rect.Top && pos.Y <= rect.Bottom;
}

void WriteText(std::string text, SMALL_RECT rect, Align align) {
    COORD coord = {0, };
	int textLength = text.length();

    switch (align) {
    case Align::LeftTop:
        coord.X = rect.Left;
        coord.Y = rect.Top;
        break;
    case Align::CenterTop:
        coord.X = rect.Left + (rect.Right - rect.Left - textLength) / 2;
        coord.Y = rect.Top;
        break;
    case Align::RightTop:
        coord.X = rect.Right - textLength;
        coord.Y = rect.Top;
        break;
    case Align::LeftCenter:
        coord.X = rect.Left;
        coord.Y = rect.Top + (rect.Bottom - rect.Top) / 2;
        break;
    case Align::Center:
        coord.X = rect.Left + (rect.Right - rect.Left - textLength) / 2;
        coord.Y = rect.Top + (rect.Bottom - rect.Top) / 2;
        break;
    case Align::RightCenter:
        coord.X = rect.Right - textLength;
        coord.Y = rect.Top + (rect.Bottom - rect.Top) / 2;
        break;
    case Align::LeftBottom:
        coord.X = rect.Left;
        coord.Y = rect.Bottom;
        break;
    case Align::CenterBottom:
        coord.X = rect.Left + (rect.Right - rect.Left - textLength) / 2;
        coord.Y = rect.Bottom;
        break;
    case Align::RightBottom:
        coord.X = rect.Right - textLength;
        coord.Y = rect.Bottom;
        break;
    }

    SetConsoleCursorPosition(hOut, coord);
    WriteConsoleA(hOut, text.c_str(), textLength, NULL, NULL);
}
