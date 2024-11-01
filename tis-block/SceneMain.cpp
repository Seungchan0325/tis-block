#include <Windows.h>

#include "SceneMain.h"

#include "common.h"
#include "Screen.h"

//L"╔═╗║╚═╝";

#define SIDE_BAR_WITH_SEGMENT	\
	L"                                                  " \
	L"                - SEQUENCE COUNTER -              " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"   ╔════════════════════════════════════════════╗ " \
	L"   ║                                            ║ " \
	L"   ║                                            ║ " \
	L"   ╚════════════════════════════════════════════╝ " \
	L"               CYCLE COUNT STATISTICS             " \
	L"   ╔═════════════════╗ ╔════════════════════════╗ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ╚═════════════════╝ ╚════════════════════════╝ " \
	L"               NODE COUNT STATISTICS              " \
	L"   ╔═════════════════╗ ╔════════════════════════╗ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ╚═════════════════╝ ╚════════════════════════╝ " \
	L"            INSTRUCTION COUNT STATISTICS          " \
	L"   ╔═════════════════╗ ╔════════════════════════╗ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ║                 ║ ║                        ║ " \
	L"   ╚═════════════════╝ ╚════════════════════════╝ " \

#define SIDE_BAR_SANDBOX	\
	L"                                                  " \
	L"                 - SIMPLE SANDBOX -               " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"   ╔═════════════════════════════════╗ ╔════════╗ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ║                                 ║ ║        ║ " \
	L"   ╚═════════════════════════════════╝ ╚════════╝ " \
	L"                                                  " \
	L"   ╔════════════════════════════════════════════╗ " \
	L"   ║                                            ║ " \
	L"   ║                                            ║ " \
	L"   ╚════════════════════════════════════════════╝ " \
	L"                                                  " \
	L"                                                  " \

#define SIDE_BAR_WIDTH 50
#define SIDE_BAR_HEIGHT 45

#define PROGRAM_TEMPLATE	\
	L"╔═════════════════════════════════╗" \
	L"║                                 ║" \
	L"║                                 ║" \
	L"╚═════════════════════════════════╝" \

#define PROGRAM_TEMPLATE_WIDTH 35
#define PROGRAM_TEMPLATE_HEIGHT 4

#define PROGRAM_COPY	\
	L"╔════════╗" \
	L"║  COPY  ║" \
	L"║        ║" \
	L"╚════════╝" \

#define PROGRAM_COPY_WIDTH 10
#define PROGRAM_COPY_HEIGHT 4

struct ProgramInfo {
	bool isExist;
	const wchar_t* name;
	int nameLength;
};

static long long enterTick;
static CHAR_INFO programTemplateBuffer[PROGRAM_TEMPLATE_WIDTH * PROGRAM_TEMPLATE_HEIGHT];
static CHAR_INFO programCopyEnabledBuffer[PROGRAM_COPY_WIDTH * PROGRAM_COPY_HEIGHT];
static CHAR_INFO programCopyDisabledBuffer[PROGRAM_COPY_WIDTH * PROGRAM_COPY_HEIGHT];
static CHAR_INFO SideBarWithSegmentBuffer[SIDE_BAR_WIDTH * SIDE_BAR_HEIGHT];
static CHAR_INFO SideBarSandboxBuffer[SIDE_BAR_WIDTH * SIDE_BAR_HEIGHT];

static void drawProgram(ProgramInfo program, COORD topLeft)
{
	SMALL_RECT rect;
	rect.Left = topLeft.X;
	rect.Top = topLeft.Y;
	rect.Right = rect.Left + PROGRAM_TEMPLATE_WIDTH - 1;
	rect.Bottom = rect.Top + PROGRAM_TEMPLATE_HEIGHT - 1;
	WriteConsoleOutput(hOut, programTemplateBuffer, { PROGRAM_TEMPLATE_WIDTH, PROGRAM_TEMPLATE_HEIGHT }, { 0, 0 }, &rect);

	rect.Left = topLeft.X + PROGRAM_TEMPLATE_WIDTH + 1;
	rect.Top = topLeft.Y;
	rect.Right = rect.Left + PROGRAM_COPY_WIDTH - 1;
	rect.Bottom = rect.Top + PROGRAM_COPY_HEIGHT - 1;
	if (program.isExist) {
		WriteConsoleOutput(hOut, programCopyEnabledBuffer, { PROGRAM_COPY_WIDTH, PROGRAM_COPY_HEIGHT }, { 0, 0 }, &rect);

		int mid = topLeft.X + PROGRAM_TEMPLATE_WIDTH / 2 - program.nameLength / 2;
		SetConsoleCursorPosition(hOut, { (short)mid, (short)(topLeft.Y + 1) });
		WriteConsole(hOut, program.name, program.nameLength, NULL, NULL);
	}
	else {
		WriteConsoleOutput(hOut, programCopyDisabledBuffer, { PROGRAM_COPY_WIDTH, PROGRAM_COPY_HEIGHT }, { 0, 0 }, &rect);
	}
}

static void drawSideBar()
{
	ProgramInfo pro;
	pro.name = L"Hello, world";
	pro.nameLength = lstrlenW(pro.name);
	if ((int)((GetTicks() - enterTick) / ticksPerSec) % 2) pro.isExist = true;
	else pro.isExist = false;
	drawProgram(pro, { 0, 0 });
}

void SceneMainEnter()
{
	enterTick = GetTicks();

	for (int i = 0; i < SIDE_BAR_WIDTH * SIDE_BAR_HEIGHT; i++) {
		SideBarWithSegmentBuffer[i].Char.UnicodeChar = SIDE_BAR_WITH_SEGMENT[i];
		SideBarWithSegmentBuffer[i].Attributes = FOREGOUND_WHITE;

		SideBarSandboxBuffer[i].Char.UnicodeChar = SIDE_BAR_SANDBOX[i];
		SideBarSandboxBuffer[i].Attributes = FOREGOUND_WHITE;
	}

	for (int i = 0; i < PROGRAM_TEMPLATE_WIDTH * PROGRAM_TEMPLATE_HEIGHT; i++) {
		programTemplateBuffer[i].Char.UnicodeChar = PROGRAM_TEMPLATE[i];
		programTemplateBuffer[i].Attributes = FOREGOUND_WHITE;
	}

	for (int i = 0; i < PROGRAM_COPY_WIDTH * PROGRAM_COPY_HEIGHT; i++) {
		programCopyEnabledBuffer[i].Char.UnicodeChar = PROGRAM_COPY[i];
		programCopyEnabledBuffer[i].Attributes = FOREGOUND_WHITE;

		programCopyDisabledBuffer[i].Char.UnicodeChar = PROGRAM_COPY[i];
		programCopyDisabledBuffer[i].Attributes = FOREGROUND_INTENSITY;
	}
}

void SceneMainUpdate()
{
	if ((int)(GetTicks() - enterTick) / ticksPerSec >= 4) {
		ChangeScene(SceneName::InGame);
	}
}

void SceneMainRender()
{
	drawSideBar();
}

void SceneMainExit()
{

}