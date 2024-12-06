#pragma once

#include <vector>
#include <string>

#include "FileSystem.h"

#define PROGRAM_NODES 12
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
	ComputeNode,
	MemoryNode,
	DamagedNode,
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

struct Node {
	int id;
	NodeType type;

	// Use only one of these
	ComputeNode compute;
	MemoryNode memory;
	DamagedNode damaged;

	bool writing;
	bool reading;
	bool anyPort;
	int rwPort;
	int outValue;

	int LAST;
	int UP, DOWN, LEFT, RIGHT;
};

extern std::vector<Node> nodes;

void InitVM(Puzzle puzzle, Program program);
void TickVM();
void ExitVM();