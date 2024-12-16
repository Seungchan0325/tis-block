#include "Scenes.h"

#include "common.h"
#include "SceneStartup.h"
#include "SceneMain.h"
#include "SceneInGame.h"
#include "SceneShutdown.h"

SceneName scene, nextScene;


static void _StartupScene(SceneName scene)
{
	switch (scene)
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
		assert(0);
		break;
	}
	}
}

static void _UpdateScene(SceneName scene)
{
	switch (scene)
	{
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
		assert(0);
		break;
	}
	}
}

static void _RenderScene(SceneName scene)
{
	switch (scene)
	{
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
		SceneShutdownRender();
		break;
	}
	default: {
		assert(0);
		break;
	}
	}
}

static void _ExitScene(SceneName scene)
{
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
		assert(0);
		break;
	}
	}
}

static void _SceneMouseEventProc(SceneName scene, MOUSE_EVENT_RECORD mer)
{
	switch (scene)
	{
	case SceneName::Startup: {
		SceneStartupMouseEventProc(mer);
		break;
	}
	case SceneName::Main: {
		SceneMainMouseEventProc(mer);
		break;
	}
	case SceneName::InGame: {
		SceneInGameMouseEventProc(mer);
		break;
	}
	case SceneName::Shutdown: {
		SceneShutdownMouseEventProc(mer);
		break;
	}
	default: {
		assert(0);
		break;
	}
	}
}

static void _SceneKeyEventProc(SceneName scene, KEY_EVENT_RECORD ker)
{
	switch (scene)
	{
	case SceneName::Startup: {
		SceneStartupKeyEventProc(ker);
		break;
	}
	case SceneName::Main: {
		SceneMainKeyEventProc(ker);
		break;
	}
	case SceneName::InGame: {
		SceneInGameKeyEventProc(ker);
		break;
	}
	case SceneName::Shutdown: {
		SceneShutdownKeyEventProc(ker);
		break;
	}
	default: {
		assert(0);
		break;
	}
	}
}

void SceneInit()
{
	scene = SceneName::Startup;
	StartupScene();
	nextScene = SceneName::None;
}

void ChangeScene(SceneName _nextScene)
{
	nextScene = _nextScene;
	StartupScene();
}

void SwapScene()
{
	if (nextScene == SceneName::None) return;
	ExitScene();
	scene = nextScene;
	StartupScene();
	nextScene = SceneName::None;
}

void StartupScene()
{
	_StartupScene(scene);
}

void UpdateScene()
{
	_UpdateScene(scene);
}

void RenderScene()
{
	_RenderScene(scene);
}

void ExitScene()
{
	_ExitScene(scene);
}

void SceneMouseEventProc(MOUSE_EVENT_RECORD mer)
{
	_SceneMouseEventProc(scene, mer);
}

void SceneKeyEventProc(KEY_EVENT_RECORD ker)
{
	_SceneKeyEventProc(scene, ker);
}
