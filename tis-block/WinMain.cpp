#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "common.h"
#include "Screen.h"
#include "Scenes.h"
#include "SceneStartup.h"
#include "SceneMain.h"
#include "SceneInGame.h"
#include "SceneShutdown.h"

using namespace std;

bool isRunning;

BOOL Startup()
{
    if (!ScreenInit()) {
        ErrorExit("Screen initialization failed");
    }

    isRunning = true;

    SceneInit();

    return TRUE;
}

void Update()
{
    // Read inputs
	DWORD eventCount = 0;
    GetNumberOfConsoleInputEvents(hIn, &eventCount);
    if (eventCount) {
        INPUT_RECORD inBuffer[256];
        DWORD inBufferSz;

        if (!ReadConsoleInput(hIn, inBuffer, ARRAYSIZE(inBuffer), &inBufferSz)) {
            ErrorExit("failed to read console input");
            return;
        }

        for (int i = 0; i < (int)inBufferSz; i++) {
            if (inBuffer[i].EventType == KEY_EVENT) {
				SceneKeyEventProc(inBuffer[i].Event.KeyEvent);
            }
            else if (inBuffer[i].EventType == MOUSE_EVENT) {
				SceneMouseEventProc(inBuffer[i].Event.MouseEvent);
            }
        }
    }

    UpdateScene();
}

void Render()
{
    ClearBuffer(hOut);

	RenderScene();

    FlipBuffer();
}

void Shutdown()
{
    isRunning = false;

	ExitScene();

    ScreenShutdown();
}

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR pCmdLine, _In_ int nCmdShow)
{
    if (!Startup()) {
        return GetLastError();
    }

    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    ticksPerSec = freq.QuadPart;

    while (isRunning) {
		SwapScene();

        Update();
        Render();

        Sleep(33);
    }
    Shutdown();
    return 0;
}