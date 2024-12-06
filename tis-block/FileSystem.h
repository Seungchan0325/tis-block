#pragma once

#include <vector>
#include <string>


#define NODE_NUM 12
#define LINE_NUM 10
#define LINE_WIDTH 16

enum class NodeTileType {
	TILE_COMPUTE,
	TILE_MEMORY,
	TILE_DAMAGED,
};

enum class StreamType {
	STREAM_INPUT,
	STREAM_OUTPUT,
};

struct Stream {
	StreamType type;
	std::string name;
	int position;
	std::vector<int> values;
};

struct Puzzle {
	std::string name;
	std::vector<std::string> description;
	std::vector<Stream> streams;
	std::vector<NodeTileType> layout;
};

struct NodeData {
	std::vector<std::string> lines;
};

struct Program {
	std::string name;
	std::vector<NodeData> nodes;
};

Puzzle LoadPuzzle(std::string path);

Program LoadProgram(std::string path);

void SaveProgram(std::string path, Program program);