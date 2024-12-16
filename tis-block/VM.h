#pragma once

#include <vector>
#include <string>

#include "FileSystem.h"

#define PROGRAM_NODES 12
#define PROGRAM_WIDTH 4
#define PROGRAM_HEIGHT 3
#define MAX_ACC 999
#define MIN_ACC -999

enum class Operation {
	NOP,
	MOV,
	SWP,
	SAV,
	ADD,
	SUB,
	NEG,
	JMP,
	JEZ,
	JNZ,
	JGZ,
	JLZ,
	JRO,
};

enum class LocationType {
	NUMBER,
	ADDRESS,
};
	
enum class AddressType {
	UP,
	RIGHT,
	DOWN,
	LEFT,
	NIL,
	ACC,
	ANY,
	LAST
};

union Location {
	short number;
	AddressType address;
};

struct Label {
	int line;
	int addr;
	std::string name;
};

struct Instruction {
	Operation operation;

	LocationType srcType;
	Location src;

	LocationType dstType;
	Location dst;

	Label label;
};

enum class NodeType {
	COMPUTE_NODE,
	MEMORY_NODE,
	DAMAGED_NODE,
	STREAM_IN_NODE,
	STREAM_OUT_NODE,
};

enum class ComputeNodeMode {
	IDLE,
	RUN,
	WRITE,
	READ
};

struct ComputeNode {
	int ACC;
	int BAK;

	int PC;

	bool isRead = false; // for MOV instruction
	int readValue = 0; // for MOV instruction

	ComputeNodeMode mode;

	std::vector<int> PC2Line;
	std::vector<std::string> lines;
	std::vector<Label> labels;
	std::vector<Instruction> instructions;
};

struct MemoryNode {
	std::vector<int> stack;
};

struct DamagedNode {
};

struct StreamInNode {
	std::string name;
	int pointer;
	std::vector<int> data;
	int position;
};

struct StreamOutNode {
	std::string name;
	std::vector<int> data;
	std::vector<int> answer;
	int position;
};

struct Node {
	int id;
	NodeType type;

	// Use only one of these
	ComputeNode compute;
	MemoryNode memory;
	DamagedNode damaged;
	StreamInNode streamIn;
	StreamOutNode streamOut;

	bool willWrite;
	bool writing;
	bool writed;
	bool wAnyPort;
	int wPort;
	int wValue;

	bool reading;
	bool read;
	bool rAnyPort;
	int rPort;
	int rValue;

	int LAST, UP, DOWN, LEFT, RIGHT;
};

extern std::vector<Node> nodes;

std::vector<std::vector<int>> IsValidProgram(Puzzle puzzle, Program program);
void InitVM(Puzzle puzzle, Program program);
void TickVM();
void ExitVM();