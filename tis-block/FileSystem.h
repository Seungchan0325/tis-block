#pragma once

#include <vector>
#include <string>
#include <map>


#define NODE_NUM 12
#define LINE_NUM 10
#define LINE_WIDTH 16

enum class NodeTileType {
	TILE_COMPUTE,
	TILE_MEMORY,
	TILE_DAMAGED,
};

enum class StreamType {
	STREAM_IN,
	STREAM_OUT,
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

struct SaveData {
	std::map<std::string, bool> isSolved;
};

Puzzle LoadPuzzle(std::string path);

Program LoadProgram(std::string path);

void SaveProgram(std::string path, Program program);

SaveData LoadSaveData(std::string path);

void SaveSaveData(std::string path, SaveData saveData);