#pragma once

#include <Windows.h>

enum class SceneName {
	Startup,
	Main,
	InGame,
	Shutdown,
	None,
};

void SceneInit();

void ChangeScene(SceneName _nextScene);

void SwapScene();

void StartupScene();

void UpdateScene();

void RenderScene();

void ExitScene();

void SceneMouseEventProc(MOUSE_EVENT_RECORD mer);

void SceneKeyEventProc(KEY_EVENT_RECORD ker);
