#include "VM.h"

#include <algorithm>
#include <sstream>

#include "common.h"
#include "FileSystem.h"

std::vector<Node> nodes;

static bool ParseLocation(std::string token, Location& location, LocationType& locationType)
{
	if (token == "UP") {
		locationType = LocationType::ADDRESS;
		location.address = AddressType::UP;
	}
	else if (token == "RIGHT") {
		locationType = LocationType::ADDRESS;
		location.address = AddressType::RIGHT;
	}
	else if (token == "DOWN") {
		locationType = LocationType::ADDRESS;
		location.address = AddressType::DOWN;
	}
	else if (token == "LEFT") {
		locationType = LocationType::ADDRESS;
		location.address = AddressType::LEFT;
	}
	else if (token == "NIL") {
		locationType = LocationType::ADDRESS;
		location.address = AddressType::NIL;
	}
	else if (token == "ACC") {
		locationType = LocationType::ADDRESS;
		location.address = AddressType::ACC;
	}
	else if (token == "ANY") {
		locationType = LocationType::ADDRESS;
		location.address = AddressType::ANY;
	}
	else if (token == "LAST") {
		locationType = LocationType::ADDRESS;
		location.address = AddressType::LAST;
	}
	else {
		if (all_of(token.begin(), token.end(), ::isdigit)) {
			locationType = LocationType::NUMBER;
			location.number = std::clamp(std::stoi(token), MIN_ACC, MAX_ACC);
		}
		else {
			return false;
		}
	}

	return true;
}

static bool IsLabelExist(std::string name, std::vector<Label>& labels, Label& out_label)
{
	for (Label& label : labels) {
		if (label.name == name) {
			out_label = label;
			return true;
		}
	}
	return false;
}

static bool CreateInstruction(std::vector<std::string> tokens, std::vector<Label> labels, Instruction& instruction)
{
	// we already parsed the label
	if (tokens[0].back() == ':') tokens.erase(tokens.begin());

	if (tokens[0] == "NOP") {
		if (tokens.size() != 1)
			return false;
		instruction.operation = Operation::NOP;
	}
	else if (tokens[0] == "MOV") {
		if (tokens.size() != 3)
			return false;
		instruction.operation = Operation::MOV;
		if (!ParseLocation(tokens[1], instruction.src, instruction.srcType))
			return false;
		if (!ParseLocation(tokens[2], instruction.dst, instruction.dstType) || instruction.dstType != LocationType::ADDRESS)
			return false;
	}
	else if (tokens[0] == "SWP") {
		if (tokens.size() != 1)
			return false;
		instruction.operation = Operation::SWP;
	}
	else if (tokens[0] == "SAV") {
		if (tokens.size() != 1)
			return false;
		instruction.operation = Operation::SAV;
	}
	else if (tokens[0] == "ADD") {
		if (tokens.size() != 2)
			return false;
		instruction.operation = Operation::ADD;
		if (!ParseLocation(tokens[1], instruction.src, instruction.srcType) || instruction.srcType != LocationType::NUMBER)
			return false;
	}
	else if (tokens[0] == "SUB") {
		if (tokens.size() != 2)
			return false;
		instruction.operation = Operation::SUB;
		if (!ParseLocation(tokens[1], instruction.src, instruction.srcType) || instruction.srcType != LocationType::NUMBER)
			return false;
	}
	else if (tokens[0] == "NEG") {
		if (tokens.size() != 1)
			return false;
		instruction.operation = Operation::NEG;
	}
	else if (tokens[0] == "JMP") {
		if (tokens.size() != 2)
			return false;
		instruction.operation = Operation::JMP;
		Label label;
		if (!IsLabelExist(tokens[1], labels, label))
			return false;
		instruction.label = label;
	}
	else if (tokens[0] == "JEZ") {
		if (tokens.size() != 2)
			return false;
		instruction.operation = Operation::JEZ;
		Label label;
		if (!IsLabelExist(tokens[1], labels, label))
			return false;
		instruction.label = label;
	}
	else if (tokens[0] == "JNZ") {
		if (tokens.size() != 2)
			return false;
		instruction.operation = Operation::JNZ;
		Label label;
		if (!IsLabelExist(tokens[1], labels, label))
			return false;
		instruction.label = label;
	}
	else if (tokens[0] == "JGZ") {
		if (tokens.size() != 2)
			return false;
		instruction.operation = Operation::JGZ;
		Label label;
		if (!IsLabelExist(tokens[1], labels, label))
			return false;
		instruction.label = label;
	}
	else if (tokens[0] == "JLZ") {
		if (tokens.size() != 2)
			return false;
		instruction.operation = Operation::JLZ;
		Label label;
		if (!IsLabelExist(tokens[1], labels, label))
			return false;
		instruction.label = label;
	}
	else if (tokens[0] == "JRO") {
		if (tokens.size() != 2)
			return false;
		instruction.operation = Operation::JRO;
		if (!ParseLocation(tokens[1], instruction.src, instruction.srcType) || instruction.srcType != LocationType::NUMBER)
			return false;
	}
	else {
		return false;
	}

	return true;
}

static std::vector<int> ParseInstruction(Node& node)
{
	std::vector<int> wrong_lines;
	for (int i = 0; i < node.compute.lines.size(); i++) {
		std::string line = node.compute.lines[i];
		line = line.substr(0, line.find('#'));
		std::stringstream ss(line);
		std::vector<std::string> tokens;

		std::string token;
		while (ss >> token) {
			tokens.push_back(token);
		}

		if (tokens.empty()) continue;

		Instruction instruction;
		if (!CreateInstruction(tokens, node.compute.labels, instruction)) {
			wrong_lines.push_back(i);
			continue;
		}

		node.compute.instructions.push_back(instruction);
	}
	return wrong_lines;
}

static void ParseLabels(Node& node)
{
	int addr = 0;
	for (int i = 0; i < node.compute.lines.size(); i++) {
		std::string line = node.compute.lines[i];
		line = line.substr(0, line.find('#'));
		std::stringstream ss(line);
		std::vector<std::string> tokens;

		std::string token;
		while (ss >> token) {
			tokens.push_back(token);
		}

		if (token.empty()) continue;

		if (tokens[0].back() == ':') {
			std::string name = tokens[0].substr(0, tokens[0].size() - 1);
			Label label;
			label.line = i;
			label.addr = addr;
			label.name = name;
			node.compute.labels.push_back(label);

			if (tokens.size() > 1) {
				node.compute.PC2Line.push_back(i);
				addr++;
			}
		}
		else {
			if (!tokens.empty()) {
				node.compute.PC2Line.push_back(i);
				addr++;
			}
		}
	}
}

static void NodeInit(Node& node)
{
	node.writing = false;
	node.reading = false;
	node.anyPort = false;
	node.rwPort = -1;
	node.outValue = 0;
	node.LAST = -1;
	node.UP = -1;
	node.DOWN = -1;
	node.LEFT = -1;
	node.RIGHT = -1;

	if (node.type == NodeType::ComputeNode) {
		node.compute.mode = ComputeNodeMode::IDLE;

		node.compute.ACC = 0;
		node.compute.BAK = 0;
		node.compute.PC = 0;

		node.compute.lines.clear();
		node.compute.instructions.clear();
		node.compute.labels.clear();
	}
	else if (node.type == NodeType::MemoryNode) {
		node.memory.stack.clear();
	}
	else if (node.type == NodeType::DamagedNode) {
		// nothing to do
	}
}

static int NodeGetInputPort(Node& node, AddressType direction)
{
	if (direction == AddressType::ANY) {
		for (int nodeIdx : {node.LEFT, node.RIGHT, node.UP, node.DOWN}) {
			if (nodeIdx == -1) continue;
			Node& neighbor = nodes[nodeIdx];
			if (neighbor.writing && (neighbor.rwPort == node.id || neighbor.anyPort)) {
				return nodeIdx;
			}
		}
	}
	else if (direction == AddressType::LAST) {
		return node.LAST;
	}
	else {
		int nodeIdx = -1;
		switch (direction)
		{
		case AddressType::UP:
			nodeIdx = node.UP;
			break;
		case AddressType::RIGHT:
			nodeIdx = node.RIGHT;
			break;
		case AddressType::DOWN:
			nodeIdx = node.DOWN;
			break;
		case AddressType::LEFT:
			nodeIdx = node.LEFT;
			break;
		}
		if (nodeIdx != -1) {
			Node& neighbor = nodes[nodeIdx];
			if (neighbor.writing && (neighbor.rwPort == node.id || neighbor.anyPort)) {
				return nodeIdx;
			}
		}
	}
	return -1;
}

static bool NodeRead(Node& node, LocationType type, Location location, int& value)
{
	if (type == LocationType::NUMBER) {
		value = location.number;
		return true;
	}
	else {
		switch (location.address)
		{
		case AddressType::NIL:
			value = 0;
			return true;
		case AddressType::ACC:
			value = node.compute.ACC;
			return true;
		case AddressType::UP:
		case AddressType::RIGHT:
		case AddressType::DOWN:
		case AddressType::LEFT:
		case AddressType::ANY:
		case AddressType::LAST:
			node.reading = true;
			int port = NodeGetInputPort(node, location.address);
			if (port != -1) {
				node.reading = false;
				node.compute.mode = ComputeNodeMode::RUN;
				value = nodes[port].outValue;
				nodes[port].writing = false;
				return true;
			}
			node.compute.mode = ComputeNodeMode::READ;
			return false;
		}
	}
	return false;
}

static bool NodeWrite(Node& node, AddressType direction, int value)
{
	if (node.compute.mode == ComputeNodeMode::WRITE)
	{
		if (node.writing == false) {
			node.rwPort = -1;
			node.anyPort = false;
			node.outValue = false;
			node.compute.mode = ComputeNodeMode::RUN;
			return true;
		}
		else {
			return false;
		}
	}

	if (direction == AddressType::ACC) {
		node.compute.ACC = value;
		return true;
	}
	else if (direction == AddressType::NIL) {
		// do nothing
		return true;
	}
	else if (direction == AddressType::ANY) {
		node.writing = true;
		node.anyPort = true;
		node.outValue = value;
		node.compute.mode = ComputeNodeMode::WRITE;
		return false;
	}
	else {
		int nodeIdx = -1;
		switch (direction)
		{
		case AddressType::UP:
			nodeIdx = node.UP;
			break;
		case AddressType::RIGHT:
			nodeIdx = node.RIGHT;
			break;
		case AddressType::DOWN:
			nodeIdx = node.DOWN;
			break;
		case AddressType::LEFT:
			nodeIdx = node.LEFT;
			break;
		}
		if (nodeIdx != -1) {
			node.writing = true;
			node.anyPort = false;
			node.outValue = value;
			node.rwPort = nodeIdx;
			node.compute.mode = ComputeNodeMode::WRITE;
			return false;
		}
		else {
			// do nothing
			return true;
		}
	}
	return false;
}

static void increasePC(Node& node)
{
	node.compute.PC++;
	if (node.compute.PC >= node.compute.instructions.size())
		node.compute.PC = 0;
}

static void NodeTick(Node& node)
{
	if (node.type == NodeType::ComputeNode) {
		if (node.compute.instructions.empty())
			return;
		Instruction instruction = node.compute.instructions[node.compute.PC];
		switch (instruction.operation)
		{
		case Operation::NOP:
			increasePC(node);
			return;
		case Operation::MOV: {
			int value = 0;
			if (!NodeRead(node, instruction.srcType, instruction.src, value))
				return;
			if (!NodeWrite(node, instruction.dst.address, value))
				return;
			increasePC(node);
			return;
		}
		case Operation::SWP:
			std::swap(node.compute.ACC, node.compute.BAK);
			increasePC(node);
			return;
		case Operation::SAV:
			node.compute.BAK = node.compute.ACC;
			increasePC(node);
			return;
		case Operation::ADD: {
			int value = 0;
			if (!NodeRead(node, instruction.srcType, instruction.src, value))
				return;
			node.compute.ACC += value;
			node.compute.ACC = std::clamp(node.compute.ACC, MIN_ACC, MAX_ACC);
			increasePC(node);
			return;
		}
		case Operation::SUB: {
			int value = 0;
			if (!NodeRead(node, instruction.srcType, instruction.src, value))
				return;
			node.compute.ACC -= value;
			node.compute.ACC = std::clamp(node.compute.ACC, MIN_ACC, MAX_ACC);
			increasePC(node);
			return;
		}
		case Operation::NEG:
			node.compute.ACC = -node.compute.ACC;
			increasePC(node);
			return;
		case Operation::JMP:
			node.compute.PC = instruction.label.addr;
			return;
		case Operation::JEZ:
			if (node.compute.ACC == 0)
				node.compute.PC = instruction.label.addr;
			else
				increasePC(node);
			return;
		case Operation::JNZ:
			if (node.compute.ACC != 0)
				node.compute.PC = instruction.label.addr;
			else
				increasePC(node);
			return;
		case Operation::JGZ:
			if (node.compute.ACC > 0)
				node.compute.PC = instruction.label.addr;
			else
				increasePC(node);
			return;
		case Operation::JLZ:
			if (node.compute.ACC < 0)
				node.compute.PC = instruction.label.addr;
			else
				increasePC(node);
			return;
		case Operation::JRO: {
			int value = 0;
			if (!NodeRead(node, instruction.srcType, instruction.src, value))
				return;
			node.compute.PC += value;
			node.compute.PC = std::clamp(node.compute.PC, 0, (int)node.compute.instructions.size() - 1);
			return;
		}
		default:
			assert("except switch");
			return;
		}
	}
}

void InitVM(Puzzle puzzle, Program program)
{
	nodes.clear();
	nodes.resize(PROGRAM_NODES);
	for (int i = 0; i < PROGRAM_NODES; i++) {
		if (puzzle.layout[i] == NodeTileType::TILE_COMPUTE) {
			nodes[i].type = NodeType::ComputeNode;
			NodeInit(nodes[i]);
			nodes[i].compute.lines = program.nodes[i].lines;
			ParseLabels(nodes[i]);
			ParseInstruction(nodes[i]);
			if(!nodes[i].compute.instructions.empty())
				nodes[i].compute.mode = ComputeNodeMode::RUN;
		}
		else if (puzzle.layout[i] == NodeTileType::TILE_MEMORY) {
			nodes[i].type = NodeType::MemoryNode;
			NodeInit(nodes[i]);
		}
		else if (puzzle.layout[i] == NodeTileType::TILE_DAMAGED) {
			nodes[i].type = NodeType::DamagedNode;
			NodeInit(nodes[i]);
		}
		nodes[i].id = i;
	}
}

void TickVM()
{
	for (int i = 0; i < PROGRAM_NODES; i++) {
		NodeTick(nodes[i]);
	}
}

void ExitVM()
{
	nodes.clear();
}