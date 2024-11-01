#pragma once

#define SCREEN_WIDTH 157
#define SCREEN_HEIGHT 45

extern HANDLE hOut;
extern HANDLE OutDouble[2];
extern HANDLE hIn;
extern DWORD fdwSaveOldMode;

BOOL ScreenInit();
void FlipBuffer();
void ClearBuffer(HANDLE hConsoleOutputBuffer);
void ScreenShutdown();