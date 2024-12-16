#include "SceneInGame.h"

#include <Windows.h>
#include <stdio.h>
#include <sstream>
#include <string>
#include <algorithm>

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
	L"║                                 ║"	\
	L"║                                 ║"	\
	L"║                                 ║"	\
	L"╚═════════════════════════════════╝"	\

#define DESCRIPTION_WIDTH 35
#define DESCRIPTION_HEIGHT 9

#define IN_STREAM_BOX \
	L"╔═══╗" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"║   ║" \
	L"╚═══╝" \

#define IN_STREAM_BOX_WIDTH 5
#define IN_STREAM_BOX_HEIGHT 26

#define OUT_STREAM_BOX \
	L"╔═══╦═══╗" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"║   ║   ║" \
	L"╚═══╩═══╝" \

#define OUT_STREAM_BOX_WIDTH 9
#define OUT_STREAM_BOX_HEIGHT 26

#define COMPUTE_NODE_ \
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
	L"║  STOP ║"	\
	L"║       ║"	\
	L"║ <ESC> ║"	\
	L"╚═══════╝"	\


#define STEP_BTN \
	L"╔═══════╗"	\
	L"║  STEP ║"	\
	L"║       ║"	\
	L"║  <F9> ║"	\
	L"╚═══════╝"	\


#define RUN_BTN \
	L"╔═══════╗"	\
	L"║  RUN  ║"	\
	L"║       ║"	\
	L"║  <F5> ║"	\
	L"╚═══════╝"	\


#define FAST_BTN \
	L"╔═══════╗"	\
	L"║       ║"	\
	L"║  FAST ║"	\
	L"║       ║"	\
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

struct StreamBox {
	int id;
	StreamType type;
	SMALL_RECT rect;
};

std::string puzzlePath;
std::string programPath;

static long long enterTick;
static CHAR_INFO descriptionBuffer[DESCRIPTION_WIDTH * DESCRIPTION_HEIGHT];
static CHAR_INFO inStreamBuffer[IN_STREAM_BOX_WIDTH * IN_STREAM_BOX_HEIGHT];
static CHAR_INFO outStreamBuffer[OUT_STREAM_BOX_WIDTH * OUT_STREAM_BOX_HEIGHT];
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
static bool isValidProgram = false;
static RunState runState = RunState::STOP;
static Puzzle puzzle;
static Program program;
static NodeBox nodes_box[NODE_NUM];
static std::vector<StreamBox> streamBoxes;
static int selectedNode = 0;
static std::vector<std::vector<int>> wrong_lines;

static void DrawStream(StreamBox stream)
{
	std::string name = puzzle.streams[stream.id].name;
	WriteText(name, stream.rect, Align::CenterTop);
	SMALL_RECT rect = stream.rect;
	rect.Top += 1;
	if (stream.type == StreamType::STREAM_IN) {
		WriteConsoleOutput(hOut, inStreamBuffer, { IN_STREAM_BOX_WIDTH, IN_STREAM_BOX_HEIGHT }, { 0, 0 }, &rect);

		if (isProgramRunning) {
			const Node& node = nodes[stream.id + PROGRAM_NODES];
			int s = max(0, node.streamIn.pointer - (IN_STREAM_BOX_HEIGHT - 2) / 2); // -2 for border
			for (int i = 0; i + s < node.streamIn.data.size() && i < IN_STREAM_BOX_HEIGHT - 2; i++) {
				std::string value = std::to_string(node.streamIn.data[i + s]);
				SMALL_RECT valueRect = rect;
				valueRect.Top += i + 1;
				valueRect.Bottom = valueRect.Top;
				valueRect.Left++;
				valueRect.Right--;
				WriteText(value, valueRect, Align::RightTop);

				if (node.streamIn.pointer < node.streamIn.data.size()) {
					if (i + s == node.streamIn.pointer) {
						DWORD written;
						COORD leftTop;
						leftTop.X = (SHORT)rect.Left + 1;
						leftTop.Y = (SHORT)valueRect.Top;
						FillConsoleOutputAttribute(hOut, BACKGROUND_INTENSITY, IN_STREAM_BOX_WIDTH - 2, leftTop, &written);
					}
				}
			}
		}
		else {
			Stream rStream = puzzle.streams[stream.id];
			for (int i = 0; i < rStream.values.size() && i < IN_STREAM_BOX_HEIGHT - 2; i++) {
				std::string value = std::to_string(rStream.values[i]);
				SMALL_RECT valueRect = rect;
				valueRect.Top += i + 1;
				valueRect.Bottom = valueRect.Top;
				valueRect.Left++;
				valueRect.Right--;
				WriteText(value, valueRect, Align::RightTop);
			}
		}
	}
	else {
		WriteConsoleOutput(hOut, outStreamBuffer, { OUT_STREAM_BOX_WIDTH, OUT_STREAM_BOX_HEIGHT }, { 0, 0 }, &rect);

		if (isProgramRunning) {
			const Node& node = nodes[stream.id + PROGRAM_NODES];
			int pointer = (int)nodes[stream.id + PROGRAM_NODES].streamOut.data.size() - 1;
			int s = max(0, pointer - (OUT_STREAM_BOX_HEIGHT - 2) / 2); // -2 for border
			for (int i = 0; i + s < node.streamOut.answer.size() && i < OUT_STREAM_BOX_HEIGHT - 2; i++) {
				std::string value = std::to_string(node.streamOut.answer[i + s]);
				SMALL_RECT valueRect = rect;
				valueRect.Top += i + 1;
				valueRect.Bottom = valueRect.Top;
				valueRect.Left++;
				valueRect.Right = valueRect.Left + 2;
				WriteText(value, valueRect, Align::RightTop);
				if (pointer < node.streamOut.answer.size()) {
					if (i + s == pointer) {
						DWORD written;
						COORD leftTop;
						leftTop.X = (SHORT)rect.Left + 1;
						leftTop.Y = (SHORT)valueRect.Top;
						FillConsoleOutputAttribute(hOut, BACKGROUND_INTENSITY, 3, leftTop, &written);
					}
				}
			}

			for (int i = 0; i + s < node.streamOut.data.size() && i < OUT_STREAM_BOX_HEIGHT - 2; i++) {
				std::string value = std::to_string(node.streamOut.data[i + s]);
				SMALL_RECT valueRect = rect;
				valueRect.Top += i + 1;
				valueRect.Bottom = valueRect.Top;
				valueRect.Left += 5;
				valueRect.Right = valueRect.Left + 2;
				WriteText(value, valueRect, Align::RightTop);
				if (pointer < node.streamOut.data.size()) {
					if (i + s == pointer) {
						DWORD written;
						COORD leftTop;
						leftTop.X = (SHORT)rect.Left + 1;
						leftTop.Y = (SHORT)valueRect.Top;
						FillConsoleOutputAttribute(hOut, BACKGROUND_INTENSITY, 3, leftTop, &written);
					}

					if (node.streamOut.data[i + s] != node.streamOut.answer[i + s]) {
						DWORD written;
						COORD leftTop;
						leftTop.X = (SHORT)rect.Left + 5;
						leftTop.Y = (SHORT)valueRect.Top;
						FillConsoleOutputAttribute(hOut, BACKGROUND_RED, 3, leftTop, &written);
					}
				}
			}
		}
		else {
			Stream rStream = puzzle.streams[stream.id];
			for (int i = 0; i < rStream.values.size() && i < OUT_STREAM_BOX_HEIGHT - 2; i++) {
				std::string value = std::to_string(rStream.values[i]);
				SMALL_RECT valueRect = rect;
				valueRect.Top += i + 1;
				valueRect.Bottom = valueRect.Top;
				valueRect.Left++;
				valueRect.Right = valueRect.Left + 2;
				WriteText(value, valueRect, Align::RightTop);
			}
		}
	}
}

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
				if ((int)cursor.X + s.size() + 1 >= DESCRIPTION_WIDTH - 2) { // 1 for blank, -2 for padding
					cursor.X = 0;
					cursor.Y++;
				}
				else {
					s = " " + s;
				}

				COORD absCurosr = { rect.Left + 1 + cursor.X, rect.Top + 1 + cursor.Y };
				SetConsoleCursorPosition(hOut, absCurosr);
				WriteConsoleA(hOut, s.c_str(), (DWORD)s.size(), NULL, NULL);

				cursor.X += (SHORT)s.size();
			}

			cursor.X = 0;
			cursor.Y++;
		}
	}

	// Draw Streams
	{
		for (StreamBox stream : streamBoxes) {
			DrawStream(stream);
		}
	}

	// Draw Buttons
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
			WriteConsoleA(hOut, node.lines[i].c_str(), (DWORD)node.lines[i].size(), NULL, NULL);
			if (std::find(wrong_lines[node.id].begin(), wrong_lines[node.id].end(), i) != wrong_lines[node.id].end()) {
				DWORD written;
				FillConsoleOutputAttribute(hOut, BACKGROUND_RED, LINE_WIDTH, pos, &written);
			}
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
				COORD absCusor = { (SHORT)node.rect.Left + 1, (SHORT)node.rect.Top + 1 + (SHORT)line };
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

			// ACC
			WriteText(std::to_string(nodes[node.id].compute.ACC), infoRect, Align::CenterTop);

			// BAK
			infoRect.Top += 2;
			infoRect.Bottom += 2;
			WriteText(std::to_string(nodes[node.id].compute.BAK), infoRect, Align::CenterTop);

			// LAST
			infoRect.Top += 2;
			infoRect.Bottom += 2;
			if (nodes[node.id].LAST == -1) {
				WriteText("N/A", infoRect, Align::CenterTop);
			}
			else {
				if (nodes[node.id].LAST == nodes[node.id].LEFT) {
					WriteText("LEFT", infoRect, Align::CenterTop);
				}
				else if (nodes[node.id].LAST == nodes[node.id].RIGHT) {
					WriteText("RIGHT", infoRect, Align::CenterTop);
				}
				else if (nodes[node.id].LAST == nodes[node.id].UP) {
					WriteText("UP", infoRect, Align::CenterTop);
				}
				else if (nodes[node.id].LAST == nodes[node.id].DOWN) {
					WriteText("DOWN", infoRect, Align::CenterTop);
				}
				else {
					assert(0);
				}
			}

			// MODE
			infoRect.Top += 2;
			infoRect.Bottom += 2;
			std::string mode;
			switch (nodes[node.id].compute.mode) {
			case ComputeNodeMode::IDLE:
				mode = "IDLE";
				break;
			case ComputeNodeMode::RUN:
				mode = "RUN";
				break;
			case ComputeNodeMode::WRITE:
				mode = "WRITE";
				break;
			case ComputeNodeMode::READ:
				mode = "READ";
				break;
			default:
				assert(0);
			}
			WriteText(mode, infoRect, Align::CenterTop);
		}
	}
	else if (node.type == NodeTileType::TILE_DAMAGED) {
		WriteConsoleOutput(hOut, errorNodeBuffer, { NODE_WIDTH, NODE_HEIGHT }, { 0, 0 }, &node.rect);
	}
	else if (node.type == NodeTileType::TILE_MEMORY) {
		WriteConsoleOutput(hOut, memoryNodeBuffer, { NODE_WIDTH, NODE_HEIGHT }, { 0, 0 }, &node.rect);
	}
}

static void DrawNodes()
{
	for (NodeBox node : nodes_box) {
		DrawNode(node);
	}

	int id = 0;
	for (Stream stream : puzzle.streams) {
		if (stream.type == StreamType::STREAM_IN) {
			SMALL_RECT rect = nodes_box[stream.position].rect;
			rect.Top -= 1;
			if (isProgramRunning) {
				const Node& node = nodes[id + PROGRAM_NODES];
				if (node.writing) {
					int digit = (int)std::to_string(node.wValue).size();
					std::string port = std::string(digit, ' ') + stream.name + " ▼ " + std::to_string(node.wValue);
					WriteText(port, rect, Align::CenterTop);
				}
				else {
					std::string port = stream.name + " ▽ ";
					WriteText(port, rect, Align::CenterTop);
				}
			}
			else {
				std::string port = stream.name + " ▽ ";
				WriteText(port, rect, Align::CenterTop);
			}
		}
		else if (stream.type == StreamType::STREAM_OUT) {
			SMALL_RECT rect = nodes_box[stream.position + PROGRAM_WIDTH * (PROGRAM_HEIGHT - 1)].rect;
			rect.Bottom += 1;
			if (isProgramRunning) {
				const Node& node = nodes[stream.position + PROGRAM_WIDTH * (PROGRAM_HEIGHT - 1)];
				if (node.writing) {
					int digit = (int)std::to_string(node.wValue).size();
					std::string port = std::string(digit, ' ') + stream.name + " ▼ " + std::to_string(node.wValue);
					WriteText(port, rect, Align::CenterBottom);
				}
				else {
					std::string port = stream.name + " ▽ ";
					WriteText(port, rect, Align::CenterBottom);
				}
			}
			else {
				std::string port = stream.name + " ▽ ";
				WriteText(port, rect, Align::CenterBottom);
			}
		}
		else {
			assert(0);
		}
		id++;
	}

	for (int i = 0; i < PROGRAM_NODES; i++) {
		bool UP = false;
		bool DOWN = false;
		bool LEFT = false;
		bool RIGHT = false;
		if (i >= PROGRAM_WIDTH) UP = true;
		if (i < PROGRAM_WIDTH * (PROGRAM_HEIGHT - 1)) DOWN = true;
		if (i % PROGRAM_WIDTH != 0) LEFT = true;
		if (i % PROGRAM_WIDTH != PROGRAM_WIDTH - 1) RIGHT = true;
		
		SMALL_RECT rect = nodes_box[i].rect;
		if (nodes_box[i].type == NodeTileType::TILE_COMPUTE) {
			if (isProgramRunning) {
				if (UP) {
					std::string port;
					if (nodes[i].writing && (nodes[i].wPort == nodes[i].UP || nodes[i].wAnyPort)) {
						port = std::to_string(nodes[i].wValue) + " ▲ ";
					}
					else if (nodes[i - PROGRAM_WIDTH].reading && (nodes[i - PROGRAM_WIDTH].rPort == i || nodes[i - PROGRAM_WIDTH].rAnyPort)) {
						port = "? ▲ ";
					}
					else {
						port = "";
					}
					rect.Left -= 2;
					rect.Right -= 2;
					rect.Top -= 1;
					WriteText(port, rect, Align::CenterTop);
					rect.Left += 2;
					rect.Right += 2;
					rect.Top += 1;
				}

				if (DOWN) {
					std::string port;
					if (nodes[i].writing && (nodes[i].wPort == nodes[i].DOWN || nodes[i].wAnyPort)) {
						port = " ▼ " + std::to_string(nodes[i].wValue);
					}
					else if (nodes[i + PROGRAM_WIDTH].reading && (nodes[i + PROGRAM_WIDTH].rPort == i || nodes[i + PROGRAM_WIDTH].rAnyPort)) {
						port = " ▼ ?";
					}
					else {
						port = "";
					}
					rect.Left += 2;
					rect.Right += 2;
					rect.Bottom += 1;
					WriteText(port, rect, Align::CenterBottom);
					rect.Bottom -= 1;
					rect.Left -= 2;
					rect.Right -= 2;
				}


				if (LEFT) {
					std::string port;
					std::string value;
					if (nodes[i].writing && (nodes[i].wPort == nodes[i].LEFT || nodes[i].wAnyPort)) {
						value = std::to_string(nodes[i].wValue);
						port = "◀";
					}
					else if (nodes[i - 1].reading && (nodes[i - 1].rPort == i || nodes[i - 1].rAnyPort)) {
						value = "?";
						port = "◀";
					}
					else {
						port = "";
					}
					rect.Left -= 3;
					rect.Top += 1;
					rect.Bottom += 1;
					WriteText(port, rect, Align::LeftCenter);
					rect.Left += 3;
					rect.Top -= 1;
					rect.Bottom -= 1;

					rect.Left -= 3;
					rect.Top += 2;
					rect.Bottom += 2;
					WriteText(value, rect, Align::LeftCenter);
					rect.Left += 3;
					rect.Top -= 2;
					rect.Bottom -= 2;
				}

				if (RIGHT) {
					std::string port;
					std::string value;
					if (nodes[i].writing && (nodes[i].wPort == nodes[i].RIGHT || nodes[i].wAnyPort)) {
						value = std::to_string(nodes[i].wValue);
						port = "▶";
					}
					else if (nodes[i + 1].reading && (nodes[i + 1].rPort == i || nodes[i + 1].rAnyPort)) {
						value = "?";
						port = "▶";
					}
					else {
						port = "";
					}
					rect.Right += 3;
					rect.Top -= 1;
					rect.Bottom -= 1;
					WriteText(port, rect, Align::RightCenter);
					rect.Right -= 3;
					rect.Top += 1;
					rect.Bottom += 1;

					rect.Right += 3;
					rect.Top -= 2;
					rect.Bottom -= 2;
					WriteText(value, rect, Align::RightCenter);
					rect.Right -= 3;
					rect.Top += 2;
					rect.Bottom += 2;
				}
			}
			else {
				std::string port = "△";
				rect.Left -= 2;
				rect.Right -= 2;
				rect.Top -= 1;
				if (UP) WriteText(port, rect, Align::CenterTop);
				rect.Left += 2;
				rect.Right += 2;
				rect.Top += 1;

				port = "▽";
				rect.Left += 2;
				rect.Right += 2;
				rect.Bottom += 1;
				if (DOWN) WriteText(port, rect, Align::CenterBottom);
				rect.Bottom -= 1;
				rect.Left -= 2;
				rect.Right -= 2;


				port = "◁";
				rect.Left -= 3;
				rect.Top += 1;
				rect.Bottom += 1;
				if (LEFT) WriteText(port, rect, Align::LeftCenter);
				rect.Left += 3;
				rect.Top -= 1;
				rect.Bottom -= 1;

				port = "▷";
				rect.Right += 3;
				rect.Top -= 1;
				rect.Bottom -= 1;
				if (RIGHT) WriteText(port, rect, Align::RightCenter);
				rect.Right -= 3;
				rect.Top += 1;
				rect.Bottom += 1;
			}
		}
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
			node.cursor.X = (SHORT)node.lines[node.cursor.Y].size();
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
			x = (SHORT)node.lines[y].size();
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

	for (int i = 0; i < IN_STREAM_BOX_WIDTH * IN_STREAM_BOX_HEIGHT; i++)
	{
		inStreamBuffer[i].Char.UnicodeChar = IN_STREAM_BOX[i];
		inStreamBuffer[i].Attributes = FOREGROUND_WHITE;
	}

	for (int i = 0; i < OUT_STREAM_BOX_WIDTH * OUT_STREAM_BOX_HEIGHT; i++)
	{
		outStreamBuffer[i].Char.UnicodeChar = OUT_STREAM_BOX[i];
		outStreamBuffer[i].Attributes = FOREGROUND_WHITE;
	}

	for (int i = 0; i < NODE_WIDTH * NODE_HEIGHT; i++)
	{
		nodeBuffer[i].Char.UnicodeChar = COMPUTE_NODE_[i];
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

	wrong_lines = IsValidProgram(puzzle, program);
	isValidProgram = true;
	for (auto& lines : wrong_lines)
	{
		if (!lines.empty())
		{
			isValidProgram = false;
			break;
		}
	}

	COORD streamLeftTop = { 3, 12 };
	for (int i = 0; i < puzzle.streams.size(); i++) {
		StreamBox stream;
		stream.id = i;
		stream.rect;
		if (puzzle.streams[i].type == StreamType::STREAM_IN) {
			stream.type = StreamType::STREAM_IN;
			stream.rect.Left = streamLeftTop.X;
			stream.rect.Right = streamLeftTop.X + IN_STREAM_BOX_WIDTH - 1;
			stream.rect.Top = streamLeftTop.Y;
			stream.rect.Bottom = streamLeftTop.Y + IN_STREAM_BOX_HEIGHT - 1 + 1; // +1 for padding
			streamLeftTop.X = stream.rect.Right + 1;
		}
		else if (puzzle.streams[i].type == StreamType::STREAM_OUT) {
			stream.type = StreamType::STREAM_OUT;
			stream.rect.Left = streamLeftTop.X;
			stream.rect.Right = streamLeftTop.X + OUT_STREAM_BOX_WIDTH - 1;
			stream.rect.Top = streamLeftTop.Y;
			stream.rect.Bottom = streamLeftTop.Y + OUT_STREAM_BOX_HEIGHT - 1 + 1; // +1 for padding
			streamLeftTop.X = stream.rect.Right + 1;
		}
		streamBoxes.push_back(stream);
	}

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
	if (isValidProgram) {
		if (!isProgramRunning) {
			InitVM(puzzle, program);
			isProgramRunning = true;
		}
		else {
			TickVM();
		}
		runState = RunState::STEP;
	}
}

static void Run()
{
	if (isValidProgram) {
		if (!isProgramRunning) {
			InitVM(puzzle, program);
			isProgramRunning = true;
		}
		runState = RunState::RUN;
	}
}

static void Fast()
{
	if (isValidProgram) {
		if (!isProgramRunning) {
			InitVM(puzzle, program);
			isProgramRunning = true;
		}
		runState = RunState::FAST;
	}
}

void SceneInGameUpdate()
{
	if (runState == RunState::RUN) {
		TickVM();
	}
	else if (runState == RunState::FAST) {
		for (int iter = 0; iter < 91; iter++)
			TickVM();
	}

	if (isProgramRunning) {
		bool isDone = true;
		for (int i = 0; i < nodes.size(); i++) {
			if (nodes[i].type == NodeType::STREAM_OUT_NODE) {
				if (nodes[i].streamOut.data.size() < nodes[i].streamOut.answer.size()) {
					isDone = false;
				}
			}
		}

		bool solved = true;
		if (isDone) {
			for (int i = 0; i < nodes.size(); i++) {
				if (nodes[i].type == NodeType::STREAM_OUT_NODE) {
					if (nodes[i].streamOut.data != nodes[i].streamOut.answer) {
						solved = false;
						break;
					}
				}
			}
			runState = RunState::STOP;
			ExitVM();
			isProgramRunning = false;

			SaveData saveData = LoadSaveData("./save.txt");
			saveData.isSolved[puzzlePath] |= solved;
			SaveSaveData("./save.txt", saveData);
		}
	}
}

void SceneInGameRender()
{
	DrawNodes();
	DrawSidebar();
}

void SceneInGameExit()
{
	streamBoxes.clear();
}

void SceneInGameKeyEventProc(KEY_EVENT_RECORD ker)
{
	if (isProgramRunning) {
		if (ker.bKeyDown == FALSE) {
			if (ker.wVirtualKeyCode == VK_ESCAPE) {
				Stop();
			}
			else if (ker.wVirtualKeyCode == VK_F5) {
				Run();
			}
		}
		else {
			if (ker.wVirtualKeyCode == VK_F9) {
				Step();
			}
		}
	}
	else {
		if (ker.bKeyDown == FALSE) {
			if (ker.wVirtualKeyCode == VK_ESCAPE) {
				ChangeScene(SceneName::Main);
			}
			else if (ker.wVirtualKeyCode == VK_F9) {
				Step();
			}
			else if (ker.wVirtualKeyCode == VK_F5) {
				Run();
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
			wrong_lines = IsValidProgram(puzzle, program);
			isValidProgram = true;
			for (auto& lines : wrong_lines)
			{
				if (!lines.empty())
				{
					isValidProgram = false;
					break;
				}
			}
		}
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
					nodes_box[i].cursor.X = (SHORT)min(x, nodes_box[i].lines[y].length());
					nodes_box[i].cursor.Y = (SHORT)y;
				}
				else {
					nodes_box[i].cursor.Y = (SHORT)nodes_box[i].lines.size() - 1;
					nodes_box[i].cursor.X = (SHORT)nodes_box[i].lines[nodes_box[i].cursor.Y].length();
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