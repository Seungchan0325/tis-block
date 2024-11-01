#include <Windows.h>

#include "common.h"

#include "Screen.h"
#include "SceneStartup.h"
#include "SceneMain.h"
#include "SceneInGame.h"
#include "SceneShutdown.h"

bool popup;
int fps;
long long ticksPerSec;
SceneName scene, nextScene;

void ErrorExit(const wchar_t* msg)
{
	ScreenShutdown();
	MessageBox(NULL, msg, L"Error", NULL);
	ExitProcess(0);
}

long long GetTicks()
{
    LARGE_INTEGER ticks;
	QueryPerformanceCounter(&ticks);
    return ticks.QuadPart;
}

void ChangeScene(SceneName _nextScene)
{
	nextScene = _nextScene;
	switch (nextScene)
	{
	case SceneName::Startup: {
		SceneStartupEnter();
		break;
	}
	case SceneName::Main: {
		SceneMainEnter();
		break;
	}
	case SceneName::InGame: {
		SceneInGameEnter();
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
}