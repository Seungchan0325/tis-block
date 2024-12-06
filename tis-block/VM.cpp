#include "VM.h"

#include "FileSystem.h"

namespace VM {
	static void ParseInstruction()
	{

	}

	void InitVM(Puzzle puzzle, Program program)
	{
		for (int i = 0; i < PROGRAM_NODES; i++) {
			if (puzzle.layout[i] == NodeTileType::TILE_COMPUTE) {
				nodes[i].type = NodeType::ComputeNode;
			}
			else if (puzzle.layout[i] == NodeTileType::TILE_MEMORY) {

			}
			else if (puzzle.layout[i] == NodeTileType::TILE_DAMAGED) {

			}
		}
	}

	void TickVM()
	{

	}
};