#pragma once

#include <vector>
#include <string>

#include "FileSystem.h"

namespace VM
{
#define PROGRAM_NODES 12
#define MAX_ACC 999
#define MIN_ACC -999
#define MAX_INST 15
#define MAX_STACK 16

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

	struct Instruction {
		Operation operation;
		LocationType srcType;
		Location src;

		LocationType dstType;
		Location dst;
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

		int ANY;
		int LAST;
		int UP, DOWN, LEFT, RIGHT;
		int PC;

		ComputeNodeMode mode;

		
	};

	struct MemoryNode {
		int stack[MAX_STACK];
	};

	struct DamagedNode {
	};

	struct Node {
		NodeType type;

		// Use only one of these
		ComputeNode compute;
		MemoryNode memory;
		DamagedNode damaged;
	};

	Node nodes[PROGRAM_NODES];

	void InitVM(Puzzle puzzle, Program program);
	void TickVM();
}