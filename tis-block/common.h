#pragma once

#define FOREGOUND_WHITE (FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED)

enum class SceneName {
	Startup,
	Main,
	InGame,
	Shutdown,
	None,
};

extern bool popup;
extern int fps;
extern long long ticksPerSec;
extern SceneName scene;
extern SceneName nextScene;

void ErrorExit(const wchar_t* msg);

long long GetTicks();

void ChangeScene(SceneName _nextScene);