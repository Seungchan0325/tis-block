#include "FileSystem.h"

#include <algorithm>
#include <fstream>
#include <string>
#include <filesystem>

#include "common.h"

namespace fs = std::filesystem;

static std::string PuzzleParseName(std::ifstream& file)
{
	std::string name;
	std::getline(file, name);
	return name;
}

static std::vector<std::string> PuzzleParseDescription(std::ifstream& file)
{
	std::vector<std::string> description;
	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty()) break;
		std::transform(line.begin(), line.end(), line.begin(), ::toupper);
		description.push_back(line);
	}
	return description;

}

static Stream PuzzleParseStream(std::ifstream& file)
{
	Stream stream;
	std::string line;
	std::getline(file, line);
	if (line == "STREAM_INPUT") stream.type = StreamType::STREAM_IN;
	else if (line == "STREAM_OUTPUT") stream.type = StreamType::STREAM_OUT;
	else ErrorExit("unknown stream type");

	std::getline(file, line);
	stream.name = line;

	std::getline(file, line);
	stream.position = std::stoi(line);

	while (std::getline(file, line))
	{
		if (line.empty()) break;
		stream.values.push_back(std::stoi(line));
	}

	return stream;
}

static std::vector<NodeTileType> PuzzleParseLayout(std::ifstream& file)
{
	std::vector<NodeTileType> layout;
	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty()) break;
		if (line == "TILE_COMPUTE") layout.push_back(NodeTileType::TILE_COMPUTE);
		else if (line == "TILE_MEMORY") layout.push_back(NodeTileType::TILE_MEMORY);
		else if (line == "TILE_DAMAGED") layout.push_back(NodeTileType::TILE_DAMAGED);
		else ErrorExit("unknown tile type");
	}

	if (layout.size() != NODE_NUM) ErrorExit("layout must have 12 tiles");

	return layout;
}

Puzzle LoadPuzzle(std::string path)
{
	std::ifstream file;
	file.open(path);
	if (file.is_open() == false) ErrorExit("file not found");

	Puzzle puzzle;

	std::string line;
	while (std::getline(file, line))
	{
		if (line.empty()) continue;

		if (line == "@name") puzzle.name = PuzzleParseName(file);
		else if (line == "@description") puzzle.description = PuzzleParseDescription(file);
		else if (line == "@stream") puzzle.streams.push_back(PuzzleParseStream(file));
		else if (line == "@layout") puzzle.layout = PuzzleParseLayout(file);
	}

	file.close();

	return puzzle;
}

Program LoadProgram(std::string path)
{
	std::ifstream file;
	file.open(path);
	if (file.is_open() == false) ErrorExit("file not found");

	Program program;
	program.nodes.resize(NODE_NUM);

	int index = -1;
	std::string line;
	while (std::getline(file, line))
	{
		//if (line.empty()) continue;

		if (line[0] == '@') {
			index = std::stoi(line.substr(1));
		}
		else {
			if (int name_start = (int)line.find("##"); program.name.empty() && name_start != std::string::npos) {
				name_start += 2; // skip "##"
				std::string name = line.substr(name_start);
				trim(name);
				program.name = name;
			}
			program.nodes[index].lines.push_back(line);
		}
	}

	if (program.name.empty()) program.name = "UNTITLED PROGRAM";

	file.close();

	return program;
}

void SaveProgram(std::string path, Program program)
{
	fs::path dir = fs::path(path).parent_path();
	if (fs::exists(dir) == false) fs::create_directories(dir);
	std::ofstream file;
	file.open(path);
	if (file.is_open() == false) ErrorExit("failed to save");

	for (int i = 0; i < NODE_NUM; i++)
	{
		file << "@" << i << std::endl;
		for (std::string line : program.nodes[i].lines)
		{
			file << line << std::endl;
		}
	}

	file.close();
}