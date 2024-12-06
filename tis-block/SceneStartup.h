#pragma once

#include <Windows.h>

void SceneStartupEnter();

void SceneStartupUpdate();

void SceneStartupRender();

void SceneStartupExit();

void SceneStartupKeyEventProc(KEY_EVENT_RECORD ker);

void SceneStartupMouseEventProc(MOUSE_EVENT_RECORD mer);