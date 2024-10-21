#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#define ARRAY_LENGTH(arr) (sizeof(arr) / sizeof(arr[0]))
#define SCREEN_WIDTH 157
#define SCREEN_HEIGHT 45

int fps, hOutIdx;
bool isRunning;
HANDLE hOut, hOutDouble[2], hIn;
DWORD fdwSaveOldMode;

void FlipBuffer()
{
    SetConsoleActiveScreenBuffer(hOutDouble[hOutIdx]);
    hOutIdx = !hOutIdx;
    hOut = hOutDouble[hOutIdx];
}

void ClearBuffer(HANDLE hConsoleOutputBuffer)
{
    DWORD numOfWritten;
    FillConsoleOutputCharacter(hConsoleOutputBuffer, L' ', SCREEN_WIDTH * SCREEN_HEIGHT, { 0, 0 }, &numOfWritten);
}

BOOL Startup()
{
    // Set Console
    if (!AllocConsole()) {
        return FALSE;
    }

    hOutDouble[0] = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOutDouble[0] == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    hOutIdx = 0;
    if (!SetConsoleActiveScreenBuffer(hOutDouble[hOutIdx])) {
        return FALSE;
    }

    if (!SetConsoleDisplayMode(hOutDouble[0], CONSOLE_FULLSCREEN_MODE, NULL)) {
        return FALSE;
    }

    if (!SetConsoleScreenBufferSize(hOutDouble[0], { SCREEN_WIDTH, SCREEN_HEIGHT })) {
        return FALSE;
    }

    hOutDouble[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    if (hOutDouble[1] == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (!GetConsoleMode(hIn, &fdwSaveOldMode)) {
        return FALSE;
    }

    if (!SetConsoleMode(hIn, ENABLE_EXTENDED_FLAGS | ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT)) {
        return FALSE;
    }

    if (!SetConsoleTitle(L"TIS-BLOCK")) {
        return FALSE;
    }

    isRunning = true;

    return TRUE;
}

void Update()
{
    DWORD NumEvents;
    GetNumberOfConsoleInputEvents(hIn, &NumEvents);

    if (NumEvents) {
        INPUT_RECORD InBuffer[128];
        DWORD InBufferSz;
        if (!ReadConsoleInput(hIn, InBuffer, ARRAYSIZE(InBuffer), &InBufferSz)) {
            return;
        }

        for (int i = 0; i < InBufferSz; i++) {
            if (InBuffer[i].EventType == MOUSE_EVENT) {
            }
        }
    }
}

void Render()
{
    ClearBuffer(hOut);

    SetConsoleCursorPosition(hOut, {0, 0});
    wchar_t s[256];
    wsprintf(s, L"¾È³çÇÏ¼¼¿ä %d\r\n", fps);
    int len = wcslen(s);
    WriteConsole(hOut, s, len, NULL, NULL);

    FlipBuffer();
}

void Shutdown()
{
    isRunning = false;
    SetConsoleMode(hIn, fdwSaveOldMode);
}

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
    if (!Startup()) {
        return GetLastError();
    }

    long long prevTick = GetTickCount64();
    long long prevSec = GetTickCount64();
    while (isRunning) {
        Update();
        Render();

        long long curTick = GetTickCount64();
        while (curTick - prevTick <= 33) {
            curTick = GetTickCount64();
        }

        if (curTick - prevSec >= 1000) {
            fps = 0;
            prevSec = curTick;
        }
        fps++;

        prevTick = curTick;
    }
    Shutdown();

    if (!FreeConsole()) {
        return GetLastError();
    }
    return 0;
}