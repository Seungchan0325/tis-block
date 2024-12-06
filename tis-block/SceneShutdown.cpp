#include <Windows.h>

#include "SceneShutdown.h"
#include "Scenes.h"
#include "common.h"

static long long enterTick;

void SceneShutdownEnter()
{
	enterTick = GetTicks();
}

void SceneShutdownUpdate()
{
	if ((GetTicks() - enterTick) > 1 * ticksPerSec)
	{
		isRunning = false;
	}
}

void SceneShutdownRender()
{

}

void SceneShutdownExit()
{

}

void SceneShutdownKeyEventProc(KEY_EVENT_RECORD ker)
{

}

void SceneShutdownMouseEventProc(MOUSE_EVENT_RECORD mer)
{

}