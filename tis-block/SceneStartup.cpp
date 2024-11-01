#include <Windows.h>

#include "SceneStartup.h"

#include <math.h>

#include "common.h"
#include "Screen.h"

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
        tisBlockBuffer[i].Attributes = FOREGOUND_WHITE;
    }
    tisBlockPos.X = 45;
    tisBlockPos.Y = 20;
}

void SceneStartupUpdate()
{
    tisBlockPos.Y = 20 - (GetTicks() - enterTick)/ticksPerSec;

    if ((GetTicks() - enterTick) / ticksPerSec >= 2) {
        ChangeScene(SceneName::Main);
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