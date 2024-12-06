#include "SceneStartup.h"

#include <Windows.h>
#include <math.h>
#include <filesystem>

#include "common.h"
#include "Screen.h"
#include "Scenes.h"

namespace fs = std::filesystem;

#define TIS_BLOCK                                                           \
    L"████████╗██╗███████╗      ██████╗ ██╗      ██████╗  ██████╗██╗  ██╗"  \
    L"╚══██╔══╝██║██╔════╝      ██╔══██╗██║     ██╔═══██╗██╔════╝██║ ██╔╝"  \
    L"   ██║   ██║███████╗█████╗██████╔╝██║     ██║   ██║██║     █████╔╝ "  \
    L"   ██║   ██║╚════██║╚════╝██╔══██╗██║     ██║   ██║██║     ██╔═██╗ "  \
    L"   ██║   ██║███████║      ██████╔╝███████╗╚██████╔╝╚██████╗██║  ██╗"  \
    L"   ╚═╝   ╚═╝╚══════╝      ╚═════╝ ╚══════╝ ╚═════╝  ╚═════╝╚═╝  ╚═╝"
#define WIDTH 67
#define HEIGHT 6

static long long enterTick;
static COORD tisBlockPos;
static CHAR_INFO tisBlockBuffer[WIDTH * HEIGHT];

void SceneStartupEnter()
{
    enterTick = GetTicks();
    for (int i = 0; i < WIDTH * HEIGHT; i++) {
        tisBlockBuffer[i].Char.UnicodeChar = TIS_BLOCK[i];
        tisBlockBuffer[i].Attributes = FOREGROUND_WHITE;
    }
    tisBlockPos.X = 45;
    tisBlockPos.Y = 20;

	if (fs::is_directory("./puzzles") == false) {
		fs::create_directory("./puzzles");
	}

	if (fs::is_directory("./programs") == false) {
		fs::create_directory("./programs");
	}
}

void SceneStartupUpdate()
{
    tisBlockPos.Y = max(20 - (GetTicks() - enterTick) / ticksPerSec, 10);

    if ((GetTicks() - enterTick) / ticksPerSec >= 2) {
        //ChangeScene(SceneName::Main);
    }
}

void SceneStartupRender()
{
    SMALL_RECT rect;
    rect.Top = tisBlockPos.Y;
    rect.Bottom = rect.Top + HEIGHT;
    rect.Left = tisBlockPos.X;
    rect.Right = rect.Left + WIDTH;
    WriteConsoleOutput(hOut, tisBlockBuffer, { WIDTH, HEIGHT }, { 0, 0 }, &rect);
}

void SceneStartupExit()
{

}

void SceneStartupKeyEventProc(KEY_EVENT_RECORD ker)
{
	if (ker.bKeyDown == FALSE) {
        if (ker.wVirtualKeyCode == VK_ESCAPE) {
            ChangeScene(SceneName::Shutdown);
        }
        else {
            ChangeScene(SceneName::Main);
        }
	}
}

void SceneStartupMouseEventProc(MOUSE_EVENT_RECORD mer)
{
    if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && mer.dwEventFlags == 0) {
		ChangeScene(SceneName::Main);
	}
}