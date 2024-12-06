#include <Windows.h>

#include "Screen.h"

#include "common.h"

int hOutIdx;
HANDLE hOut, hOutDouble[2], hIn;
DWORD fdwSaveOldMode;

BOOL ScreenInit()
{
    if (!AllocConsole()) return FALSE;

    if (!SetConsoleTitle(L"TIS-BLOCK")) return FALSE;

    hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    if (!SetConsoleDisplayMode(hOut, CONSOLE_FULLSCREEN_MODE, NULL)) {
        return FALSE;
    }

    if (!SetConsoleScreenBufferSize(hOut, { SCREEN_WIDTH, SCREEN_HEIGHT })) {
        return FALSE;
    }

    hIn = GetStdHandle(STD_INPUT_HANDLE);
    if (hIn == INVALID_HANDLE_VALUE) return FALSE;

    if (!GetConsoleMode(hIn, &fdwSaveOldMode)) return FALSE;

    DWORD fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT | ENABLE_EXTENDED_FLAGS;
    if (!SetConsoleMode(hIn, fdwMode)) return FALSE;

	hOutDouble[0] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    hOutDouble[1] = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);

	if (hOutDouble[0] == INVALID_HANDLE_VALUE || hOutDouble[1] == INVALID_HANDLE_VALUE) {
		return FALSE;
	}

    CONSOLE_CURSOR_INFO cursor = {};
    cursor.bVisible = FALSE;
    cursor.dwSize = 1;
    if (!SetConsoleCursorInfo(hOutDouble[0], &cursor)) {
        return FALSE;
    }
    if (!SetConsoleCursorInfo(hOutDouble[1], &cursor)) {
        return FALSE;
    }

	hOut = hOutDouble[0];
	SetConsoleActiveScreenBuffer(hOut);

    return TRUE;
}

void ScreenShutdown()
{
    SetConsoleMode(hIn, fdwSaveOldMode);
    FreeConsole();
}

void FlipBuffer()
{
    SetConsoleActiveScreenBuffer(hOut);
    hOutIdx = !hOutIdx;
    hOut = hOutDouble[hOutIdx];
}

void ClearBuffer(HANDLE hConsoleOutputBuffer)
{
    DWORD numOfWritten;
    FillConsoleOutputCharacter(hConsoleOutputBuffer, L' ', SCREEN_WIDTH * SCREEN_HEIGHT, { 0, 0 }, &numOfWritten);
}