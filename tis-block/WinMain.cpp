#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>

#include "common.h"
#include "Screen.h"
#include "SceneStartup.h"
#include "SceneMain.h"
#include "SceneInGame.h"
#include "SceneShutdown.h"

using namespace std;

bool isRunning;
wstring s;

BOOL Startup()
{
    if (!ScreenInit()) {
        ErrorExit(L"Screen initialization failed");
    }

    isRunning = true;

    // Scene Init
    scene = SceneName::Startup;
    nextScene = SceneName::None;
    SceneStartupEnter();

    return TRUE;
}

void Update()
{
    // Read inputs
    DWORD numEvents;
    GetNumberOfConsoleInputEvents(hIn, &numEvents);
    if (numEvents) {
        INPUT_RECORD inBuffer[128];
        DWORD inBufferSz;
        if (!ReadConsoleInput(hIn, inBuffer, ARRAYSIZE(inBuffer), &inBufferSz)) {
            return;
        }

        for (int i = 0; i < (int)inBufferSz; i++) {
            if (inBuffer[i].EventType == KEY_EVENT) {
                s.push_back((wchar_t)inBuffer[i].Event.KeyEvent.uChar.UnicodeChar);
            }
        }
    }

    switch (scene) {
    case SceneName::Startup: {
        SceneStartupUpdate();
        break;
    }
    case SceneName::Main: {
        SceneMainUpdate();
        break;
    }
    case SceneName::InGame: {
        SceneInGameUpdate();
        break;
    }
    case SceneName::Shutdown: {
        SceneShutdownUpdate();
        break;
    }
    default: {
        ErrorExit(L"switch exception");
        break;
    }
    }
}

void Render()
{
    ClearBuffer(hOut);

    SetConsoleCursorPosition(hOut, {1, 1});
    wchar_t ss[256];
    wsprintf(ss, L"안녕하세요 %d\r\n", fps);
    int len = (int)wcslen(ss);
    WriteConsole(hOut, ss, len, NULL, NULL);

    SetConsoleCursorPosition(hOut, { 10, 10 });
    WriteConsole(hOut, s.c_str(), s.size(), NULL, NULL);

    switch (scene) {
    case SceneName::Startup: {
        SceneStartupRender();
        break;
    }
    case SceneName::Main: {
        SceneMainRender();
        break;
    }
    case SceneName::InGame: {
        SceneInGameRender();
        break;
    }
    case SceneName::Shutdown: {
        SceneShutdownEnter();
        break;
    }
    default: {
        ErrorExit(L"switch exception");
        break;
    }
    }

    FlipBuffer();
}

void Shutdown()
{
    isRunning = false;
    switch (scene)
    {
    case SceneName::Startup: {
        SceneStartupExit();
        break;
    }
    case SceneName::Main: {
        SceneMainExit();
        break;
    }
    case SceneName::InGame: {
        SceneInGameExit();
        break;
    }
    case SceneName::Shutdown: {
        SceneShutdownExit();
        break;
    }
    default: {
        ErrorExit(L"switch exception");
        break;
    }
    }
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

    long long prevTick = GetTicks();
    long long prevSec = GetTicks();
    while (isRunning) {
        if (nextScene != SceneName::None) {
            switch (scene)
            {
            case SceneName::Startup: {
                SceneStartupExit();
                break;
            }
            case SceneName::Main: {
                SceneMainExit();
                break;
            }
            case SceneName::InGame: {
                SceneInGameExit();
                break;
            }
            case SceneName::Shutdown: {
                SceneShutdownExit();
                break;
            }
            default: {
                ErrorExit(L"switch exception");
                break;
            }
            }
            scene = nextScene;
            nextScene = SceneName::None;
        }
        Update();
        Render();

        long long curTick = GetTicks();
        while (curTick - prevTick <= ticksPerSec / 30) {
            curTick = GetTicks();
        }

        prevTick = curTick;

        if (curTick - prevSec >= ticksPerSec) {
            fps = 0;
            prevSec = curTick;
        }
        fps++;
    }
    Shutdown();
    return 0;
}