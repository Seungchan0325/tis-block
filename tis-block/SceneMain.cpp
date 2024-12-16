#include "SceneMain.h"

#include <Windows.h>
#include <wchar.h>
#include <stdio.h>
#include <filesystem>

#include "common.h"
#include "Screen.h"
#include "Scenes.h"
#include "FileSystem.h"
#include "SceneInGame.h"

namespace fs = std::filesystem;

#define PROGRAM_TEMPLATE	\
	L"╔═════════════════════════════════╗" \
	L"║                                 ║" \
	L"║                                 ║" \
	L"╚═════════════════════════════════╝" \

#define PROGRAM_TEMPLATE_WIDTH 35
#define PROGRAM_TEMPLATE_HEIGHT 4

#define PROGRAM_COPY	\
	L"╔════════╗" \
	L"║ DELETE ║" \
	L"║        ║" \
	L"╚════════╝" \

#define PROGRAM_COPY_WIDTH 10
#define PROGRAM_COPY_HEIGHT 4

#define PUZZLE \
	L"╔═════════════════╗" \
	L"║                 ║" \
	L"║                 ║" \
	L"║                 ║" \
	L"║                 ║" \
	L"║                 ║" \
	L"╚═════════════════╝" \

#define PUZZLE_WIDTH 19
#define PUZZLE_HEIGHT 7

#define PROGRAM_NUM 10
#define PUZZLE_NUM 25

#define PROGRAM_NODES 12

struct ProgramBox {
	bool isExist;
	std::string name;
	std::string path;
	SMALL_RECT rect;
	SMALL_RECT templateRect;
	SMALL_RECT copyRect;
};

struct PuzzleBox {
	bool isExist;
	std::string name;
	std::string path;
	SMALL_RECT rect;
	SMALL_RECT btnRect;
};

static long long enterTick;
static CHAR_INFO programTemplateBuffer[PROGRAM_TEMPLATE_WIDTH * PROGRAM_TEMPLATE_HEIGHT];
static CHAR_INFO programCopyEnabledBuffer[PROGRAM_COPY_WIDTH * PROGRAM_COPY_HEIGHT];
static CHAR_INFO programCopyDisabledBuffer[PROGRAM_COPY_WIDTH * PROGRAM_COPY_HEIGHT];
static CHAR_INFO puzzleBuffer[PUZZLE_WIDTH * PUZZLE_HEIGHT];
static CHAR_INFO selectedPuzzleBuffer[PUZZLE_WIDTH * PUZZLE_HEIGHT];
static ProgramBox programs[PROGRAM_NUM];
static PuzzleBox puzzles[PUZZLE_NUM];
static int selectedPuzzle = 0;

static SaveData saveData;

static void DrawProgram(ProgramBox program)
{
	WriteConsoleOutput(hOut, programTemplateBuffer, { PROGRAM_TEMPLATE_WIDTH, PROGRAM_TEMPLATE_HEIGHT }, { 0, 0 }, &program.templateRect);
	WriteText(program.name, program.templateRect, Align::Center);

	if (program.isExist) {
		WriteConsoleOutput(hOut, programCopyEnabledBuffer, { PROGRAM_COPY_WIDTH, PROGRAM_COPY_HEIGHT }, { 0, 0 }, &program.copyRect);
	}
	else {
		WriteConsoleOutput(hOut, programCopyDisabledBuffer, { PROGRAM_COPY_WIDTH, PROGRAM_COPY_HEIGHT }, { 0, 0 }, &program.copyRect);
	}
}

static void DrawPuzzle(PuzzleBox puzzle, bool selected)
{
	WriteText(puzzle.name, puzzle.rect, Align::CenterTop);
	if(selected) WriteConsoleOutput(hOut, selectedPuzzleBuffer, { PUZZLE_WIDTH, PUZZLE_HEIGHT }, { 0, 0 }, &puzzle.btnRect);
	else WriteConsoleOutput(hOut, puzzleBuffer, { PUZZLE_WIDTH, PUZZLE_HEIGHT }, { 0, 0 }, &puzzle.btnRect);

	if (saveData.isSolved[puzzle.path]) {
		WriteText("SOLVED", puzzle.btnRect, Align::Center);
	}
	else {
		WriteText("NOT SOLVED", puzzle.btnRect, Align::Center);
	}
}

static void DrawSidebar()
{
	short left = 3;
	short right = 3 + PROGRAM_TEMPLATE_WIDTH + PROGRAM_COPY_WIDTH - 1;

	SMALL_RECT rect;
	rect.Left = left;
	rect.Right = right;
	rect.Top = rect.Bottom = 2;
	WriteText("- SEQUENCE COUNTER -", rect, Align::Center);

	for (short i = 0; i < PROGRAM_NUM; i++) {
		DrawProgram(programs[i]);
	}
}

static void LoadPrograms()
{
	for (short i = 0; i < PROGRAM_NUM; i++) {
		std::string path = "./programs/" + std::to_string(selectedPuzzle) + "/" + std::to_string(i) + ".txt";
		if (fs::exists(path)) {
			Program program = LoadProgram(path);
			programs[i].name = program.name;
			programs[i].isExist = true;
		}
		else {
			programs[i].name = "CREATE NEW PROGRAM";
			programs[i].isExist = false;
		}

		programs[i].path = path;
	}
}

void SceneMainEnter()
{
	enterTick = GetTicks();

	// Initialize buffers
	for (int i = 0; i < PROGRAM_TEMPLATE_WIDTH * PROGRAM_TEMPLATE_HEIGHT; i++) {
		programTemplateBuffer[i].Char.UnicodeChar = PROGRAM_TEMPLATE[i];
		programTemplateBuffer[i].Attributes = FOREGROUND_WHITE;
	}

	for (int i = 0; i < PROGRAM_COPY_WIDTH * PROGRAM_COPY_HEIGHT; i++) {
		programCopyEnabledBuffer[i].Char.UnicodeChar = PROGRAM_COPY[i];
		programCopyEnabledBuffer[i].Attributes = FOREGROUND_WHITE;

		programCopyDisabledBuffer[i].Char.UnicodeChar = PROGRAM_COPY[i];
		programCopyDisabledBuffer[i].Attributes = FOREGROUND_INTENSITY;
	}

	for (int i = 0; i < PUZZLE_WIDTH * PUZZLE_HEIGHT; i++) {
		puzzleBuffer[i].Char.UnicodeChar = PUZZLE[i];
		puzzleBuffer[i].Attributes = FOREGROUND_WHITE;

		selectedPuzzleBuffer[i].Char.UnicodeChar = PUZZLE[i];
		selectedPuzzleBuffer[i].Attributes = FOREGROUND_INTENSITY;
	}

	// load puzzles
	for (int i = 0; i < PUZZLE_NUM; i++) {
		std::string path = "./puzzles/" + std::to_string(i) + ".txt";
		if (fs::exists(path)) {
			Puzzle puzzle = LoadPuzzle(path);

			puzzles[i].isExist = true;
			puzzles[i].name = puzzle.name;
			puzzles[i].path = path;
		}
		else {
			puzzles[i].isExist = false;
			puzzles[i].name = "LOCKED";
		}
		

		int width = PUZZLE_WIDTH;
		int height = PUZZLE_HEIGHT + 1;
		int x = i % 5;
		int y = i / 5;

		int top = 4 + y * height;
		int left = 51 + x * (width + 2);

		puzzles[i].rect.Top = top;
		puzzles[i].rect.Bottom = puzzles[i].rect.Top + PUZZLE_HEIGHT;
		puzzles[i].rect.Left = left;
		puzzles[i].rect.Right = puzzles[i].rect.Left + PUZZLE_WIDTH - 1;

		puzzles[i].btnRect.Top = top + 1;
		puzzles[i].btnRect.Bottom = puzzles[i].btnRect.Top + PUZZLE_HEIGHT - 1;
		puzzles[i].btnRect.Left = puzzles[i].rect.Left;
		puzzles[i].btnRect.Right = puzzles[i].rect.Right;
	}

	SMALL_RECT rect;
	rect.Left = 3;
	rect.Right = rect.Left + PROGRAM_TEMPLATE_WIDTH + PROGRAM_COPY_WIDTH - 1;

	SMALL_RECT programTemplateRect;
	programTemplateRect.Left = rect.Left;
	programTemplateRect.Right = rect.Left + PROGRAM_TEMPLATE_WIDTH - 1;

	SMALL_RECT copyRect;
	copyRect.Left = rect.Left + PROGRAM_TEMPLATE_WIDTH;
	copyRect.Right = copyRect.Left + PROGRAM_COPY_WIDTH - 1;
	for (short i = 0; i < PROGRAM_NUM; i++) {
		rect.Top = rect.Bottom = 3 + i * PROGRAM_TEMPLATE_HEIGHT;
		rect.Bottom = rect.Top + PROGRAM_TEMPLATE_HEIGHT - 1;
		programTemplateRect.Top = copyRect.Top = rect.Top;
		programTemplateRect.Bottom = copyRect.Bottom = rect.Bottom;

		programs[i].rect = rect;
		programs[i].templateRect = programTemplateRect;
		programs[i].copyRect = copyRect;
	}

	LoadPrograms();

	if (!fs::exists("./save.txt")) {
		saveData = SaveData();
		SaveSaveData("./save.txt", saveData);
	}
	saveData = LoadSaveData("./save.txt");
}

void SceneMainUpdate()
{
}

void SceneMainRender()
{
	DrawSidebar();
	for (int i = 0; i < PUZZLE_NUM; i++) {
		DrawPuzzle(puzzles[i], i == selectedPuzzle);
	}
}

void SceneMainExit()
{
}

void SceneMainKeyEventProc(KEY_EVENT_RECORD ker)
{
	if (ker.bKeyDown) {
		if (ker.wVirtualKeyCode == VK_ESCAPE) {
			ChangeScene(SceneName::Shutdown);
		}
	}
}

void SceneMainMouseEventProc(MOUSE_EVENT_RECORD mer)
{
	for (int i = 0; i < PROGRAM_NUM; i++) {
		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && mer.dwEventFlags == 0) {
			if (InRect(mer.dwMousePosition, programs[i].templateRect) && programs[i].isExist && puzzles[selectedPuzzle].isExist) {
				programPath = programs[i].path;
				puzzlePath = puzzles[selectedPuzzle].path;
				ChangeScene(SceneName::InGame);
			}
			else if (InRect(mer.dwMousePosition, programs[i].templateRect) && !programs[i].isExist && puzzles[selectedPuzzle].isExist) {
				Program program;
				program.nodes.resize(PROGRAM_NODES);
				SaveProgram(programs[i].path, program);
				LoadPrograms(); // reload
			}
			else if (InRect(mer.dwMousePosition, programs[i].copyRect) && programs[i].isExist) {
				fs::remove(programs[i].path);
				LoadPrograms(); // reload
			}
		}
	}

	for (int i = 0; i < PUZZLE_NUM; i++) {
		if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && mer.dwEventFlags == 0) {
			if (InRect(mer.dwMousePosition, puzzles[i].btnRect)) {
				selectedPuzzle = i;
				puzzlePath = puzzles[i].path;
				LoadPrograms();
			}
		}
	}
}