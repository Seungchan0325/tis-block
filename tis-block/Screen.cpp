#include <Windows.h>

#include "Screen.h"

#include "common.h"

int hOutIdx;
HANDLE hOut, hOutDouble[2], hIn;
DWORD fdwSaveOldMode;

BOOL ScreenInit()
{
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

    CONSOLE_CURSOR_INFO cursor = {};
    cursor.bVisible = FALSE;
    cursor.dwSize = 1;
    if (!SetConsoleCursorInfo(hOutDouble[0], &cursor)) {
        return FALSE;
    }


    hOutDouble[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

    if (hOutDouble[1] == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (!SetConsoleCursorInfo(hOutDouble[1], &cursor)) {
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

    return TRUE;
}

void ScreenShutdown()
{
    SetConsoleMode(hIn, fdwSaveOldMode);
    FreeConsole();
}

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