#include "SceneInGame.h"

#include <Windows.h>
#include <stdio.h>
#include <sstream>
#include <string>

#include "common.h"
#include "Screen.h"
#include "Scenes.h"
#include "Filesystem.h"
#include "VM.h"

#define DESCRIPTION_BOX \
	L"╔═════════════════════════════════╗"	\
	L"║                                 ║"	\
	L"║                                 ║"	\
	L"║                                 ║"	\
	L"║                                 ║"	\
	L"╚═════════════════════════════════╝"	\

#define DESCRIPTION_WIDTH 35
#define DESCRIPTION_HEIGHT 6

#define COMPUTE_NODE \
	L"╔═════════════════╦═════╗"	\
	L"║                 ║ ACC ║"	\
	L"║                 ║     ║"	\
	L"║                 ║ BAK ║"	\
	L"║                 ║     ║"	\
	L"║                 ║ LAST║"	\
	L"║                 ║     ║"	\
	L"║                 ║ MODE║"	\
	L"║                 ║     ║"	\
	L"║                 ║ IDLE║"	\
	L"║                 ║     ║"	\
	L"╚═════════════════╩═════╝"	\

#define DAMAGED_NODE \
	L"╔═════════════════╦═════╗"	\
	L"║                 ║     ║"	\
	L"║  █████████████  ║     ║"	\
	L"║  COMMUNICATION  ║     ║"	\
	L"║     FAILURE     ║     ║"	\
	L"║  █████████████  ║     ║"	\
	L"║                 ║     ║"	\
	L"║                 ║     ║"	\
	L"║                 ║     ║"	\
	L"║                 ║     ║"	\
	L"║                 ║     ║"	\
	L"╚═════════════════╩═════╝"	\

#define MEMORY_NODE \
	L"╔═════════════════╦═════╗"	\
	L"║                 ║     ║"	\
	L"║                 ║     ║"	\
	L"║                 ║     ║"	\
	L"║█████████████████║     ║"	\
	L"║STACK MEMORY NODE║     ║"	\
	L"║█████████████████║     ║"	\
	L"║                 ║     ║"	\
	L"║                 ║     ║"	\
	L"║                 ║     ║"	\
	L"║                 ║     ║"	\
	L"╚═════════════════╩═════╝"	\

#define NODE_WIDTH 25
#define NODE_HEIGHT 12

#define STOP_BTN \
	L"╔═══════╗"	\
	L"║ █████ ║"	\
	L"║ █████ ║"	\
	L"║  STOP ║"	\
	L"╚═══════╝"	\


#define STEP_BTN \
	L"╔═══════╗"	\
	L"║       ║"	\
	L"║       ║"	\
	L"║  STEP ║"	\
	L"╚═══════╝"	\


#define RUN_BTN \
	L"╔═══════╗"	\
	L"║       ║"	\
	L"║       ║"	\
	L"║  RUN  ║"	\
	L"╚═══════╝"	\


#define FAST_BTN \
	L"╔═══════╗"	\
	L"║       ║"	\
	L"║       ║"	\
	L"║  FAST ║"	\
	L"╚═══════╝"	\

#define BTN_WIDTH 9
#define BTN_HEIGHT 5

enum class RunState {
	STOP,
	STEP,
	RUN,
	FAST,
};

struct NodeBox {
	int id;
	bool is_selected;
	COORD cursor;
	SMALL_RECT rect;
	SMALL_RECT textRect;
	NodeTileType type;
	std::vector<std::string> lines;
};

std::string puzzlePath;
std::string programPath;

static long long enterTick;
static CHAR_INFO descriptionBuffer[DESCRIPTION_WIDTH * DESCRIPTION_HEIGHT];
static CHAR_INFO nodeBuffer[NODE_WIDTH * NODE_HEIGHT];
static CHAR_INFO errorNodeBuffer[NODE_WIDTH * NODE_HEIGHT];
static CHAR_INFO memoryNodeBuffer[NODE_WIDTH * NODE_HEIGHT];
static CHAR_INFO stopBtnBuffer[BTN_WIDTH * BTN_HEIGHT];
static CHAR_INFO stepBtnBuffer[BTN_WIDTH * BTN_HEIGHT];
static CHAR_INFO runBtnBuffer[BTN_WIDTH * BTN_HEIGHT];
static CHAR_INFO fastBtnBuffer[BTN_WIDTH * BTN_HEIGHT];

static SMALL_RECT stopBtnRect;
static SMALL_RECT stepBtnRect;
static SMALL_RECT runBtnRect;
static SMALL_RECT fastBtnRect;

static bool isProgramRunning = false;
static RunState runState = RunState::STOP;
static Puzzle puzzle;
static Program program;
static NodeBox nodes_box[NODE_NUM];
static int selectedNode = 0;

static void DrawSidebar()
{
	short left = 3;
	short right = left + DESCRIPTION_WIDTH - 1;

	WriteText("- " + puzzle.name + " -", {left, 1, right, 1}, Align::CenterTop);

	// Draw description
	{
		SMALL_RECT rect;
		rect.Left = left;
		rect.Right = left + DESCRIPTION_WIDTH - 1;
		rect.Top = 2;
		rect.Bottom = rect.Top + DESCRIPTION_HEIGHT - 1;
		WriteConsoleOutput(hOut, descriptionBuffer, { DESCRIPTION_WIDTH, DESCRIPTION_HEIGHT }, { 0, 0 }, &rect);

		COORD cursor = { 0, 0 };

		for (auto description : puzzle.description) {
			description = "> " + description;
			std::stringstream ss(description);
			std::string s;
			while (ss >> s) {
				if (cursor.X + s.size() + 1 >= DESCRIPTION_WIDTH - 2) { // 1 for blank, -2 for padding
					cursor.X = 0;
					cursor.Y++;
				}
				else {
					s = " " + s;
				}

				COORD absCurosr = { rect.Left + 1 + cursor.X, rect.Top + 1 + cursor.Y };
				SetConsoleCursorPosition(hOut, absCurosr);
				WriteConsoleA(hOut, s.c_str(), s.size(), NULL, NULL);

				cursor.X += s.size();
			}

			cursor.X = 0;
			cursor.Y++;
		}
	}

	WriteConsoleOutput(hOut, stopBtnBuffer, { BTN_WIDTH, BTN_HEIGHT }, { 0, 0 }, &stopBtnRect);
	WriteConsoleOutput(hOut, stepBtnBuffer, { BTN_WIDTH, BTN_HEIGHT }, { 0, 0 }, &stepBtnRect);
	WriteConsoleOutput(hOut, runBtnBuffer, { BTN_WIDTH, BTN_HEIGHT }, { 0, 0 }, &runBtnRect);
	WriteConsoleOutput(hOut, fastBtnBuffer, { BTN_WIDTH, BTN_HEIGHT }, { 0, 0 }, &fastBtnRect);
}

static void DrawNode(NodeBox node)
{
	if (node.type == NodeTileType::TILE_COMPUTE) {
		WriteConsoleOutput(hOut, nodeBuffer, { NODE_WIDTH, NODE_HEIGHT }, { 0, 0 }, &node.rect);
		for (short i = 0; i < node.lines.size(); i++)
		{
			COORD pos = { node.rect.Left + 1, node.rect.Top + i + 1 };
			SetConsoleCursorPosition(hOut, pos);
			WriteConsoleA(hOut, node.lines[i].c_str(), node.lines[i].size(), NULL, NULL);
		}

		if (runState == RunState::STOP && node.is_selected) {
			COORD absCusor = { node.rect.Left + 1 + node.cursor.X, node.rect.Top + 1 + node.cursor.Y };
			DWORD written;
			FillConsoleOutputAttribute(hOut, BACKGROUND_INTENSITY, 1, absCusor, &written);
		}

		if (runState != RunState::STOP) {
			if (nodes[node.id].compute.mode != ComputeNodeMode::IDLE) {
				int PC = nodes[node.id].compute.PC;
				int line = nodes[node.id].compute.PC2Line[PC];
				COORD absCusor = { node.rect.Left + 1, node.rect.Top + 1 + line };
				DWORD written;
				FillConsoleOutputAttribute(hOut, BACKGROUND_INTENSITY, LINE_WIDTH, absCusor, &written);
			}
			SMALL_RECT infoRect;
			infoRect.Top = 2;
			infoRect.Bottom = 2;
			infoRect.Left = 19;
			infoRect.Right = 25;
			infoRect.Top += node.rect.Top;
			infoRect.Bottom += node.rect.Top;
			infoRect.Left += node.rect.Left;
			infoRect.Right += node.rect.Left;
			WriteText(std::to_string(nodes[node.id].compute.ACC), infoRect, Align::CenterTop);

			infoRect.Top += 2;
			infoRect.Bottom += 2;
			WriteText(std::to_string(nodes[node.id].compute.BAK), infoRect, Align::CenterTop);

			infoRect.Top += 2;
			infoRect.Bottom += 2;
			if (nodes[node.id].LAST == -1) {
				WriteText("N/A", infoRect, Align::CenterTop);
			}
			else {
				WriteText(std::to_string(nodes[node.id].LAST), infoRect, Align::CenterTop);
			}
		}
	}
	else if (node.type == NodeTileType::TILE_DAMAGED) {
		WriteConsoleOutput(hOut, errorNodeBuffer, { NODE_WIDTH, NODE_HEIGHT }, { 0, 0 }, &node.rect);
	}
	else if (node.type == NodeTileType::TILE_MEMORY) {
		WriteConsoleOutput(hOut, memoryNodeBuffer, { NODE_WIDTH, NODE_HEIGHT }, { 0, 0 }, &node.rect);
	}
}

static void NewLineNode(NodeBox& node)
{
	if (node.lines.size() < LINE_NUM) {
		SHORT& x = node.cursor.X;
		SHORT& y = node.cursor.Y;

		std::string& line = node.lines[y];
		std::string newline = node.lines[y].substr(x);
		line.erase(line.begin() + x, line.end());
		node.lines.insert(node.lines.begin() + node.cursor.Y + 1, newline);

		node.cursor.X = 0;
		node.cursor.Y++;
	}
}

static void MoveCusorUpNode(NodeBox& node)
{
	if (node.cursor.Y > 0) {
		node.cursor.Y--;
		node.cursor.X = min(node.cursor.X, (SHORT)node.lines[node.cursor.Y].size());
	}
}

static void MoveCursorDownNode(NodeBox& node)
{
	if (node.cursor.Y < node.lines.size() - 1) {
		node.cursor.Y++;
		node.cursor.X = min(node.cursor.X, (SHORT)node.lines[node.cursor.Y].size());
	}
}

static void MoveCursorLeftNode(NodeBox& node)
{
	if (node.cursor.X > 0) {
		node.cursor.X--;
	}
	else {
		if (node.cursor.Y > 0) {
			node.cursor.Y--;
			node.cursor.X = node.lines[node.cursor.Y].size();
		}
	}
}

static void MoveCursorRightNode(NodeBox& node)
{
	std::string& line = node.lines[node.cursor.Y];
	if (node.cursor.X < line.size()) {
		node.cursor.X++;
	}
	else {
		if (node.cursor.Y < node.lines.size() - 1) {
			node.cursor.Y++;
			node.cursor.X = 0;
		}
	}
}

static void BackspaceNode(NodeBox& node)
{
	SHORT& x = node.cursor.X;
	SHORT& y = node.cursor.Y;
	if (x > 0) {
		node.lines[y].erase(x - 1, 1);
		x--;
	}
	else {
		if (y > 0 && node.lines[y-1].size() + node.lines[y].size() <= LINE_WIDTH) {
			y--;
			x = node.lines[y].size();
			node.lines[y] += node.lines[y + 1];
			node.lines.erase(node.lines.begin() + y + 1);
		}
	}
}

static void DeleteNode(NodeBox& node)
{
	SHORT& x = node.cursor.X;
	SHORT& y = node.cursor.Y;
	if (x < node.lines[y].size()) {
		node.lines[y].erase(x, 1);
	}
	else {
		if (y < node.lines.size() - 1 && node.lines[y].size() + node.lines[y+1].size() <= LINE_WIDTH) {
			node.lines[y] += node.lines[y + 1];
			node.lines.erase(node.lines.begin() + y + 1);
		}
	}
}

static void WriteCharToNode(NodeBox& node, char c)
{
	if (isalpha(c)) c = toupper(c);
	std::string& line = node.lines[node.cursor.Y];
	if (line.length() < LINE_WIDTH) {
		line.insert(line.begin() + node.cursor.X, c);
		node.cursor.X++;
	}
}

void SceneInGameEnter()
{
	enterTick = GetTicks();

	for (int i = 0; i < DESCRIPTION_WIDTH * DESCRIPTION_HEIGHT; i++)
	{
		descriptionBuffer[i].Char.UnicodeChar = DESCRIPTION_BOX[i];
		descriptionBuffer[i].Attributes = FOREGROUND_WHITE;
	}

	for (int i = 0; i < NODE_WIDTH * NODE_HEIGHT; i++)
	{
		nodeBuffer[i].Char.UnicodeChar = COMPUTE_NODE[i];
		nodeBuffer[i].Attributes = FOREGROUND_WHITE;

		errorNodeBuffer[i].Char.UnicodeChar = DAMAGED_NODE[i];
		errorNodeBuffer[i].Attributes = FOREGROUND_RED;

		memoryNodeBuffer[i].Char.UnicodeChar = MEMORY_NODE[i];
		memoryNodeBuffer[i].Attributes = FOREGROUND_WHITE;
	}

	for (int i = 0; i < BTN_WIDTH * BTN_HEIGHT; i++)
	{
		stopBtnBuffer[i].Char.UnicodeChar = STOP_BTN[i];
		stopBtnBuffer[i].Attributes = FOREGROUND_WHITE;

		stepBtnBuffer[i].Char.UnicodeChar = STEP_BTN[i];
		stepBtnBuffer[i].Attributes = FOREGROUND_WHITE;

		runBtnBuffer[i].Char.UnicodeChar = RUN_BTN[i];
		runBtnBuffer[i].Attributes = FOREGROUND_WHITE;

		fastBtnBuffer[i].Char.UnicodeChar = FAST_BTN[i];
		fastBtnBuffer[i].Attributes = FOREGROUND_WHITE;
	}

	int btnTop = 39;
	int btnLeft = 2;

	stopBtnRect.Top = btnTop;
	stopBtnRect.Bottom = stopBtnRect.Top + BTN_HEIGHT - 1;
	stopBtnRect.Left = btnLeft;
	stopBtnRect.Right = stopBtnRect.Left + BTN_WIDTH - 1;

	stepBtnRect.Top = btnTop;
	stepBtnRect.Bottom = stepBtnRect.Top + BTN_HEIGHT - 1;
	stepBtnRect.Left = btnLeft + BTN_WIDTH + 1;
	stepBtnRect.Right = stepBtnRect.Left + BTN_WIDTH - 1;

	runBtnRect.Top = btnTop;
	runBtnRect.Bottom = runBtnRect.Top + BTN_HEIGHT - 1;
	runBtnRect.Left = btnLeft + BTN_WIDTH * 2 + 2;
	runBtnRect.Right = runBtnRect.Left + BTN_WIDTH - 1;
	
	fastBtnRect.Top = btnTop;
	fastBtnRect.Bottom = fastBtnRect.Top + BTN_HEIGHT - 1;
	fastBtnRect.Left = btnLeft + BTN_WIDTH * 3 + 3;
	fastBtnRect.Right = fastBtnRect.Left + BTN_WIDTH - 1;

	puzzle = LoadPuzzle(puzzlePath);
	program = LoadProgram(programPath);

	for (int i = 0; i < NODE_NUM; i++) {
		nodes_box[i].id = i;
		nodes_box[i].type = puzzle.layout[i];
		nodes_box[i].lines = program.nodes[i].lines;
		nodes_box[i].cursor = { 0, 0 };
		nodes_box[i].is_selected = false;

		if (nodes_box[i].type == NodeTileType::TILE_COMPUTE && nodes_box[i].lines.empty())
			nodes_box[i].lines.push_back("");

		int x = i % 4;
		int y = i / 4;
		int left = 41 + x * (NODE_WIDTH + 4);
		int top = 4 + y * (NODE_HEIGHT + 1);
		nodes_box[i].rect.Top = top;
		nodes_box[i].rect.Bottom = top + NODE_HEIGHT - 1;
		nodes_box[i].rect.Left = left;
		nodes_box[i].rect.Right = left + NODE_WIDTH - 1;

		nodes_box[i].textRect.Top = top + 1;
		nodes_box[i].textRect.Bottom = nodes_box[i].textRect.Top + LINE_NUM - 1;
		nodes_box[i].textRect.Left = left + 1;
		nodes_box[i].textRect.Right = nodes_box[i].textRect.Left + LINE_WIDTH;
	}

	selectedNode = 0;
	nodes_box[selectedNode].is_selected = true;
}

static void AdaptNodeLines()
{
	for (int i = 0; i < NODE_NUM; i++) {
		program.nodes[i].lines = nodes_box[i].lines;
	}
}

static void Stop()
{
	if (isProgramRunning) {
		ExitVM();
		isProgramRunning = false;
	}
	runState = RunState::STOP;
}

static void Step()
{
	if (!isProgramRunning) {
		InitVM(puzzle, program);
		isProgramRunning = true;
	}

	runState = RunState::STEP;
	TickVM();
}

static void Run()
{
	if (!isProgramRunning) {
		InitVM(puzzle, program);
		isProgramRunning = true;
	}
	runState = RunState::RUN;
}

static void Fast()
{
	if (!isProgramRunning) {
		InitVM(puzzle, program);
		isProgramRunning = true;
	}
	runState = RunState::FAST;
}

void SceneInGameUpdate()
{
	if (runState == RunState::RUN) {
		TickVM();
	}
	else if (runState == RunState::FAST) {
		for (int iter = 0; iter < 100; iter++)
			TickVM();
	}
}

void SceneInGameRender()
{
	for (int i = 0; i < NODE_NUM; i++)
	{
		DrawNode(nodes_box[i]);
	}
	DrawSidebar();
}

void SceneInGameExit()
{
}

void SceneInGameKeyEventProc(KEY_EVENT_RECORD ker)
{
	if (isProgramRunning) return;

	if (ker.bKeyDown == FALSE) {
		if (ker.wVirtualKeyCode == VK_ESCAPE) {
			ChangeScene(SceneName::Main);
		}
	}
	else if (ker.bKeyDown == TRUE) {
		if (ker.wVirtualKeyCode == VK_RETURN) {
			NewLineNode(nodes_box[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_UP) {
			MoveCusorUpNode(nodes_box[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_DOWN) {
			MoveCursorDownNode(nodes_box[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_LEFT) {
			MoveCursorLeftNode(nodes_box[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_RIGHT) {
			MoveCursorRightNode(nodes_box[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_BACK) {
			BackspaceNode(nodes_box[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_DELETE) {
			DeleteNode(nodes_box[selectedNode]);
		}
		else if (isprint(ker.uChar.AsciiChar)) {
			WriteCharToNode(nodes_box[selectedNode], ker.uChar.AsciiChar);
		}

		AdaptNodeLines();
		SaveProgram(programPath, program);
		program = LoadProgram(programPath);
	}
}

void SceneInGameMouseEventProc(MOUSE_EVENT_RECORD mer)
{
	if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && mer.dwEventFlags == 0)
	{
		for (int i = 0; i < NODE_NUM; i++)
		{
			if (nodes_box[i].type == NodeTileType::TILE_COMPUTE && InRect(mer.dwMousePosition, nodes_box[i].textRect))
			{
				nodes_box[selectedNode].is_selected = false;

				selectedNode = i;
				nodes_box[selectedNode].is_selected = true;

				int x = mer.dwMousePosition.X;
				int y = mer.dwMousePosition.Y;

				x -= nodes_box[i].rect.Left + 1;
				y -= nodes_box[i].rect.Top + 1;

				if (y < nodes_box[i].lines.size()) {
					nodes_box[i].cursor.X = min(x, nodes_box[i].lines[y].length());
					nodes_box[i].cursor.Y = y;
				}
				else {
					nodes_box[i].cursor.Y = nodes_box[i].lines.size() - 1;
					nodes_box[i].cursor.X = nodes_box[i].lines[nodes_box[i].cursor.Y].length();
				}
			}
		}

		if (InRect(mer.dwMousePosition, stopBtnRect)) {
			Stop();
		}
		else if (InRect(mer.dwMousePosition, stepBtnRect)) {
			Step();
		}
		else if (InRect(mer.dwMousePosition, runBtnRect)) {
			Run();
		}
		else if (InRect(mer.dwMousePosition, fastBtnRect)) {
			Fast();
		}
	}
}