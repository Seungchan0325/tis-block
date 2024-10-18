#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

void Update();
void Render();

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    if (!AllocConsole()) {
        return GetLastError();
    }

    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!SetConsoleDisplayMode(hOut, CONSOLE_FULLSCREEN_MODE, NULL)) {
        return GetLastError();
    }

    WriteConsole(hOut, L"Hello, world", 12, NULL, NULL);
    while (true);
    if (!FreeConsole()) {
        return GetLastError();
    }
    return 0;
}