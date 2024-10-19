#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define FPS 30

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))

int fps;
bool isRunning;
HANDLE hOut, hIn;

void Startup()
{
    isRunning = true;
}

void Update()
{
    DWORD NumEvents;
    GetNumberOfConsoleInputEvents(hIn, &NumEvents);

    if (NumEvents) {
        INPUT_RECORD InBuffer[256];
        DWORD InBufferSz;
        if (!ReadConsoleInput(hIn, InBuffer, ARRAYSIZE(InBuffer), &InBufferSz)) {
            return;
        }
    }
}

void Render()
{
    wchar_t s[256];
    wsprintf(s, L"%d", fps);
    int len = wcslen(s);
    WriteConsole(hOut, s, len, NULL, NULL);


    wsprintf(s, L"\r\nHello, world");
    len = wcslen(s);
    WriteConsole(hOut, s, len, NULL, NULL);
}

void Shutdown()
{
    isRunning = false;
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    if (!AllocConsole()) {
        return GetLastError();
    }

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (!SetConsoleDisplayMode(hOut, CONSOLE_FULLSCREEN_MODE, NULL)) {
        return GetLastError();
    }

    hIn = GetStdHandle(STD_INPUT_HANDLE);

    SetConsoleTitle(L"TIS-BLOCK");

    long long prevTick = GetTickCount64();
    long long prevSec = GetTickCount64();

    Startup();
    while (isRunning) {
        Update();
        Render();

        long long curTick = GetTickCount64();
        while (curTick - prevTick <= 33) {
            curTick = GetTickCount64();
        }
        prevTick = curTick;

        if (curTick - prevSec >= 1000) {
            fps = 0;
        }
        fps++;
    }
    Shutdown();

    if (!FreeConsole()) {
        return GetLastError();
    }
    return 0;
}