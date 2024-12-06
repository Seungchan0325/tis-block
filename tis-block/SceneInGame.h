#pragma once

#include <Windows.h>
#include <string>

#define MAX_PATH_LENGTH 1024

extern std::string puzzlePath;
extern std::string programPath;

void SceneInGameEnter();

void SceneInGameUpdate();

void SceneInGameRender();

void SceneInGameExit();

void SceneInGameKeyEventProc(KEY_EVENT_RECORD ker);

void SceneInGameMouseEventProc(MOUSE_EVENT_RECORD mer);