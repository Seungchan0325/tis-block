#include "SceneInGame.h"

#include <Windows.h>
#include <stdio.h>
#include <sstream>
#include <string>

#include "common.h"
#include "Screen.h"
#include "Scenes.h"
#include "Filesystem.h"

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
	L"╔═════════╗"	\
	L"║  █████  ║"	\
	L"║  █████  ║"	\
	L"║   STOP  ║"	\
	L"╚═════════╝"	\


#define STEP_BTN \
	L"╔═════════╗"	\
	L"║         ║"	\
	L"║         ║"	\
	L"║   STEP  ║"	\
	L"╚═════════╝"	\


#define RUN_BTN \
	L"╔═════════╗"	\
	L"║         ║"	\
	L"║         ║"	\
	L"║   RUN   ║"	\
	L"╚═════════╝"	\


#define FAST_BTN \
	L"╔═════════╗"	\
	L"║         ║"	\
	L"║         ║"	\
	L"║   FAST  ║"	\
	L"╚═════════╝"	\

#define BTN_WIDTH 11
#define BTN_HEIGHT 5

struct NodeBox {
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

static Puzzle puzzle;
static Program program;
static NodeBox nodes[NODE_NUM];
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

		if (node.is_selected) {
			COORD absCusor = { node.rect.Left + 1 + node.cursor.X, node.rect.Top + 1 + node.cursor.Y };
			DWORD written;
			FillConsoleOutputAttribute(hOut, BACKGROUND_INTENSITY, 1, absCusor, &written);
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

	puzzle = LoadPuzzle(puzzlePath);
	program = LoadProgram(programPath);

	for (int i = 0; i < NODE_NUM; i++) {
		nodes[i].type = puzzle.layout[i];
		nodes[i].lines = program.nodes[i].lines;
		nodes[i].cursor = { 0, 0 };
		nodes[i].is_selected = false;

		if (nodes[i].type == NodeTileType::TILE_COMPUTE && nodes[i].lines.empty())
			nodes[i].lines.push_back("");

		int x = i % 4;
		int y = i / 4;
		int left = 41 + x * (NODE_WIDTH + 4);
		int top = 4 + y * (NODE_HEIGHT + 1);
		nodes[i].rect.Top = top;
		nodes[i].rect.Bottom = top + NODE_HEIGHT - 1;
		nodes[i].rect.Left = left;
		nodes[i].rect.Right = left + NODE_WIDTH - 1;

		nodes[i].textRect.Top = top + 1;
		nodes[i].textRect.Bottom = nodes[i].textRect.Top + LINE_NUM - 1;
		nodes[i].textRect.Left = left + 1;
		nodes[i].textRect.Right = nodes[i].textRect.Left + LINE_WIDTH;
	}

	selectedNode = 0;
	nodes[selectedNode].is_selected = true;
}

static void AdaptNodeLines()
{
	for (int i = 0; i < NODE_NUM; i++) {
		program.nodes[i].lines = nodes[i].lines;
	}
}

void SceneInGameUpdate()
{

}

void SceneInGameRender()
{
	for (int i = 0; i < NODE_NUM; i++)
	{
		DrawNode(nodes[i]);
	}
	DrawSidebar();
}

void SceneInGameExit()
{
}

void SceneInGameKeyEventProc(KEY_EVENT_RECORD ker)
{
	if (ker.bKeyDown == FALSE) {
		if (ker.wVirtualKeyCode == VK_ESCAPE) {
			ChangeScene(SceneName::Main);
		}
	}
	else if (ker.bKeyDown == TRUE) {
		if (ker.wVirtualKeyCode == VK_RETURN) {
			NewLineNode(nodes[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_UP) {
			MoveCusorUpNode(nodes[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_DOWN) {
			MoveCursorDownNode(nodes[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_LEFT) {
			MoveCursorLeftNode(nodes[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_RIGHT) {
			MoveCursorRightNode(nodes[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_BACK) {
			BackspaceNode(nodes[selectedNode]);
		}
		else if (ker.wVirtualKeyCode == VK_DELETE) {
			DeleteNode(nodes[selectedNode]);
		}
		else if (isprint(ker.uChar.AsciiChar)) {
			WriteCharToNode(nodes[selectedNode], ker.uChar.AsciiChar);
		}

		AdaptNodeLines();
		SaveProgram(programPath, program);
	}
}

void SceneInGameMouseEventProc(MOUSE_EVENT_RECORD mer)
{
	if (mer.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED && mer.dwEventFlags == 0)
	{
		for (int i = 0; i < NODE_NUM; i++)
		{
			if (nodes[i].type == NodeTileType::TILE_COMPUTE && InRect(mer.dwMousePosition, nodes[i].textRect))
			{
				nodes[selectedNode].is_selected = false;

				selectedNode = i;
				nodes[selectedNode].is_selected = true;

				int x = mer.dwMousePosition.X;
				int y = mer.dwMousePosition.Y;

				x -= nodes[i].rect.Left + 1;
				y -= nodes[i].rect.Top + 1;

				if (y < nodes[i].lines.size()) {
					nodes[i].cursor.X = min(x, nodes[i].lines[y].length());
					nodes[i].cursor.Y = y;
				}
				else {
					nodes[i].cursor.Y = nodes[i].lines.size() - 1;
					nodes[i].cursor.X = nodes[i].lines[nodes[i].cursor.Y].length();
				}
			}
		}
	}
}