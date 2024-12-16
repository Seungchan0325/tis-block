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
		if (all_of(token.begin(), token.end(), ::isdigit) || (token[0] == '-' && all_of(token.begin()+1, token.end(), ::isdigit))) {
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
	if (tokens.empty()) assert(0);

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
		if (!ParseLocation(tokens[1], instruction.src, instruction.srcType))
			return false;
	}
	else if (tokens[0] == "SUB") {
		if (tokens.size() != 2)
			return false;
		instruction.operation = Operation::SUB;
		if (!ParseLocation(tokens[1], instruction.src, instruction.srcType))
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
		if (!ParseLocation(tokens[1], instruction.src, instruction.srcType))
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
		line.erase(std::remove(line.begin(), line.end(), ','), line.end());
		line = line.substr(0, line.find('#'));
		std::stringstream ss(line);
		std::vector<std::string> tokens;

		std::string token;
		while (ss >> token) {
			tokens.push_back(token);
		}

		if (tokens.empty()) continue;

		if (tokens[0].back() == ':') tokens.erase(tokens.begin());

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
		line.erase(std::remove(line.begin(), line.end(), ','), line.end());
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
	node.writed = false;
	node.wAnyPort = false;
	node.wPort = -1;
	node.wValue = 0;

	node.reading = false;
	node.rAnyPort = false;
	node.rPort = -1;

	node.LAST = node.UP = node.DOWN = node.LEFT = node.RIGHT = -1;

	if (node.type == NodeType::COMPUTE_NODE) {
		node.compute.mode = ComputeNodeMode::IDLE;

		node.compute.ACC = 0;
		node.compute.BAK = 0;
		node.compute.PC = 0;

		node.compute.isRead = false;
		node.compute.readValue = 0;

		node.compute.lines.clear();
		node.compute.instructions.clear();
		node.compute.labels.clear();
	}
	else if (node.type == NodeType::MEMORY_NODE) {
		node.memory.stack.clear();
	}
	else if (node.type == NodeType::DAMAGED_NODE) {
		// nothing to do
	}
	else if (node.type == NodeType::STREAM_IN_NODE) {
		node.streamIn.pointer = 0;
		node.streamIn.position = 0;
		node.streamIn.name = "";
		node.streamIn.data.clear();
	}
	else if (node.type == NodeType::STREAM_OUT_NODE) {
		node.streamOut.position = 0;
		node.streamOut.name = "";
		node.streamOut.data.clear();
		node.streamOut.answer.clear();
	}
}

static int NodeGetInputPort(Node& node, AddressType direction)
{
	if (direction == AddressType::ANY) {
		for (int nodeIdx : {node.LEFT, node.RIGHT, node.UP, node.DOWN}) {
			if (nodeIdx == -1) continue;
			Node& neighbor = nodes[nodeIdx];
			if ((neighbor.writing && !neighbor.writed) && (neighbor.wPort == node.id || neighbor.wAnyPort)) {
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
			if ((neighbor.writing && !neighbor.writed) && (neighbor.wPort == node.id || neighbor.wAnyPort)) {
				return nodeIdx;
			}
		}
	}
	return -1;
}

static bool NodeRead(Node& node, AddressType addr, int& value)
{
	if (node.reading) {
		if (node.read) {
			node.reading = false;
			node.read = false;
			node.rAnyPort = false;
			node.rPort = -1;

			value = node.rValue;
			return true;
		}
		else {
			return false;
		}
	}

	if (addr == AddressType::NIL) {
		value = 0;
		return true;
	}

	int port = NodeGetInputPort(node, addr);
	if (port != -1) {
		if (addr == AddressType::ANY)
			node.LAST = port;
		if (nodes[port].wAnyPort)
			nodes[port].LAST = node.id;

		value = nodes[port].wValue;
		nodes[port].writed = true;

		node.reading = false;
		node.read = false;
		node.rAnyPort = false;
		node.rPort = -1;
		return true;
	}

	node.reading = true;
	node.read = false;
	if (addr == AddressType::ANY)
		node.rAnyPort = true;
	else {
		node.rAnyPort = false;
		switch (addr) {
		case AddressType::UP:
			node.rPort = node.UP;
			break;
		case AddressType::RIGHT:
			node.rPort = node.RIGHT;
			break;
		case AddressType::DOWN:
			node.rPort = node.DOWN;
			break;
		case AddressType::LEFT:
			node.rPort = node.LEFT;
			break;
		case AddressType::LAST:
			node.rPort = node.LAST;
			break;
		default:
			assert(0);
		}
	}

	return false;
}

static bool NodeWrite(Node& node, AddressType addr, int value)
{
	if (node.writing) {
		if (node.writed) {
			node.writing = false;
			node.writed = false;
			node.wAnyPort = false;
			node.wPort = -1;
			return true;
		}
		else {
			return false;
		}
	}

	if (addr == AddressType::NIL) {
		return true;
	}
	else if (addr == AddressType::ANY) {
		node.willWrite = true;
		node.wAnyPort = true;
		node.wValue = value;
		node.wPort = -1;
		return false;
	}
	else {
		int nodeIdx = -1;
		switch (addr)
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
		case AddressType::LAST:
			nodeIdx = node.LAST;
			break;
		default:
			assert(0);
		}
		if (nodeIdx != -1) {
			node.willWrite = true;
			node.wAnyPort = false;
			node.wValue = value;
			node.wPort = nodeIdx;
			return false;
		}
		else {
			// do nothing
			return true;
		}
	}
}

static void NodeReadUpdate(Node& node)
{
	if (node.reading) {
		int port = node.rPort;
		if (node.rAnyPort) port = NodeGetInputPort(node, AddressType::ANY);
		if (port != -1 && nodes[port].writing && !nodes[port].writed && (nodes[port].wAnyPort || nodes[port].wPort == node.id)) {
			nodes[port].writed = true;
			if (node.rAnyPort)
				node.LAST = port;
			if (nodes[port].wAnyPort)
				nodes[port].LAST = node.id;

			node.rValue = nodes[port].wValue;

			node.read = true;
		}
	}
}

static void NodeWriteUpdate(Node& node)
{
	if (node.willWrite) {
		node.writing = true;
		node.writed = false;
		node.willWrite = false;
	}
}

static bool ComputeNodeRead(Node& node, LocationType type, Location location, int& value)
{
	if (type == LocationType::NUMBER) {
		value = location.number;
		return true;
	}
	else if (location.address == AddressType::ACC) {
		value = node.compute.ACC;
		return true;
	}
	else {
		AddressType addr = location.address;
		bool ret = NodeRead(node, addr, value);
		if (ret)
			node.compute.mode = ComputeNodeMode::RUN;
		else
			node.compute.mode = ComputeNodeMode::READ;
		return ret;
	}
	return false;
}

static bool ComputeNodeWrite(Node& node, AddressType direction, int value)
{
	if (direction == AddressType::ACC) {
		node.compute.ACC = value;
		return true;
	}
	else {
		bool ret = NodeWrite(node, direction, value);
		if (ret)
			node.compute.mode = ComputeNodeMode::RUN;
		else
			node.compute.mode = ComputeNodeMode::WRITE;
		return ret;
	}
	return false;
}

static void ComputeNodeIncreasePC(Node& node)
{
	node.compute.PC++;
	if (node.compute.PC >= node.compute.instructions.size())
		node.compute.PC = 0;
}

static void ComputeNodeTick(Node& node)
{
	if (node.compute.instructions.empty())
		return;
	Instruction instruction = node.compute.instructions[node.compute.PC];
	bool& isRead = node.compute.isRead; // for MOV instruction
	int& readValue = node.compute.readValue; // for MOV instruction
	switch (instruction.operation)
	{
	case Operation::NOP:
		ComputeNodeIncreasePC(node);
		return;
	case Operation::MOV: {
		if (!isRead) {
			if (!ComputeNodeRead(node, instruction.srcType, instruction.src, readValue))
				return;
		}
		isRead = true;
		if (!ComputeNodeWrite(node, instruction.dst.address, readValue))
			return;
		isRead = false;
		ComputeNodeIncreasePC(node);
		return;
	}
	case Operation::SWP:
		std::swap(node.compute.ACC, node.compute.BAK);
		ComputeNodeIncreasePC(node);
		return;
	case Operation::SAV:
		node.compute.BAK = node.compute.ACC;
		ComputeNodeIncreasePC(node);
		return;
	case Operation::ADD: {
		int value = 0;
		if (!ComputeNodeRead(node, instruction.srcType, instruction.src, value))
			return;
		node.compute.ACC += value;
		node.compute.ACC = std::clamp(node.compute.ACC, MIN_ACC, MAX_ACC);
		ComputeNodeIncreasePC(node);
		return;
	}
	case Operation::SUB: {
		int value = 0;
		if (!ComputeNodeRead(node, instruction.srcType, instruction.src, value))
			return;
		node.compute.ACC -= value;
		node.compute.ACC = std::clamp(node.compute.ACC, MIN_ACC, MAX_ACC);
		ComputeNodeIncreasePC(node);
		return;
	}
	case Operation::NEG:
		node.compute.ACC = -node.compute.ACC;
		ComputeNodeIncreasePC(node);
		return;
	case Operation::JMP:
		node.compute.PC = instruction.label.addr;
		return;
	case Operation::JEZ:
		if (node.compute.ACC == 0)
			node.compute.PC = instruction.label.addr;
		else
			ComputeNodeIncreasePC(node);
		return;
	case Operation::JNZ:
		if (node.compute.ACC != 0)
			node.compute.PC = instruction.label.addr;
		else
			ComputeNodeIncreasePC(node);
		return;
	case Operation::JGZ:
		if (node.compute.ACC > 0)
			node.compute.PC = instruction.label.addr;
		else
			ComputeNodeIncreasePC(node);
		return;
	case Operation::JLZ:
		if (node.compute.ACC < 0)
			node.compute.PC = instruction.label.addr;
		else
			ComputeNodeIncreasePC(node);
		return;
	case Operation::JRO: {
		int value = 0;
		if (!ComputeNodeRead(node, instruction.srcType, instruction.src, value))
			return;
		node.compute.PC += value;
		node.compute.PC = std::clamp(node.compute.PC, 0, (int)node.compute.instructions.size() - 1);
		return;
	}
	default:
		assert(0);
		return;
	}
}

static void MemoryNodeTick(Node& node)
{
	int value = 0;
	if (NodeRead(node, AddressType::ANY, value)) {
		node.memory.stack.push_back(value);
	}

	if (node.memory.stack.size() > 0) {
		if (NodeWrite(node, AddressType::ANY, node.memory.stack.back()))
			node.memory.stack.pop_back();
	}
}

static void StreamInNodeTick(Node& node)
{
	while (node.streamIn.pointer < node.streamIn.data.size() && NodeWrite(node, AddressType::DOWN, node.streamIn.data[node.streamIn.pointer])) {
		node.streamIn.pointer++;
	}
}

static void StreamOutNodeTick(Node& node)
{
	if (node.streamOut.data.size() < node.streamOut.answer.size()) {
		int value = 0;
		if (NodeRead(node, AddressType::UP, value))
			node.streamOut.data.push_back(value);
	}
}

static void NodeTick(Node& node)
{
	if (node.type == NodeType::COMPUTE_NODE) {
		ComputeNodeTick(node);
	}
	else if (node.type == NodeType::MEMORY_NODE) {
		MemoryNodeTick(node);
	}
	else if (node.type == NodeType::DAMAGED_NODE) {
		// do nothing
	}
	else if (node.type == NodeType::STREAM_IN_NODE) {
		StreamInNodeTick(node);
	}
	else if (node.type == NodeType::STREAM_OUT_NODE) {
		StreamOutNodeTick(node);
	}
}

std::vector<std::vector<int>> IsValidProgram(Puzzle puzzle, Program program)
{
	std::vector<std::vector<int>> wrong_lines;
	for (int i = 0; i < PROGRAM_NODES; i++) {
		if (puzzle.layout[i] == NodeTileType::TILE_COMPUTE) {
			Node node;
			node.id = i;
			node.type = NodeType::COMPUTE_NODE;
			NodeInit(node);
			node.compute.lines = program.nodes[i].lines;
			ParseLabels(node);
			auto wrong_line = ParseInstruction(node);
			wrong_lines.push_back(wrong_line);
		}
		else {
			wrong_lines.push_back(std::vector<int>());
		}
	}
	return wrong_lines;
}

void InitVM(Puzzle puzzle, Program program)
{
	nodes.clear();
	nodes.resize(PROGRAM_NODES);
	for (int i = 0; i < PROGRAM_NODES; i++) {
		nodes[i].id = i;
		if (puzzle.layout[i] == NodeTileType::TILE_COMPUTE) {
			nodes[i].type = NodeType::COMPUTE_NODE;
			NodeInit(nodes[i]);

			nodes[i].compute.lines = program.nodes[i].lines;
			ParseLabels(nodes[i]);
			ParseInstruction(nodes[i]);
			if (!nodes[i].compute.instructions.empty()) {
				nodes[i].compute.mode = ComputeNodeMode::RUN;
			}
			else
				nodes[i].compute.mode = ComputeNodeMode::IDLE;
		}
		else if (puzzle.layout[i] == NodeTileType::TILE_MEMORY) {
			nodes[i].type = NodeType::MEMORY_NODE;
			NodeInit(nodes[i]);
		}
		else if (puzzle.layout[i] == NodeTileType::TILE_DAMAGED) {
			nodes[i].type = NodeType::DAMAGED_NODE;
			NodeInit(nodes[i]);
		}
	}

	for (int i = 0; i < PROGRAM_NODES; i++) {
		if (nodes[i].type != NodeType::DAMAGED_NODE) {
			int left = i - 1;
			if (left % PROGRAM_WIDTH == PROGRAM_WIDTH - 1 || left < 0 || nodes[left].type == NodeType::DAMAGED_NODE) left = -1;
			int right = i + 1;
			if (right % PROGRAM_WIDTH == 0 || right >= PROGRAM_NODES || nodes[right].type == NodeType::DAMAGED_NODE) right = -1;
			int up = i - PROGRAM_WIDTH;
			if (up < 0 || nodes[up].type == NodeType::DAMAGED_NODE) up = -1;
			int down = i + PROGRAM_WIDTH;
			if (down >= PROGRAM_NODES || nodes[down].type == NodeType::DAMAGED_NODE) down = -1;

			nodes[i].LEFT = left;
			nodes[i].RIGHT = right;
			nodes[i].UP = up;
			nodes[i].DOWN = down;
		}
	}

	for (auto stream : puzzle.streams) {
		Node node;
		node.id = (int)nodes.size();
		if (stream.type == StreamType::STREAM_IN)
			node.type = NodeType::STREAM_IN_NODE;
		else if (stream.type == StreamType::STREAM_OUT)
			node.type = NodeType::STREAM_OUT_NODE;

		NodeInit(node);

		if (stream.type == StreamType::STREAM_IN) {
			node.streamIn.name = stream.name;
			node.streamIn.position = stream.position;
			node.streamIn.data = stream.values;
			node.DOWN = stream.position;
			nodes[node.DOWN].UP = node.id;
		}
		else if (stream.type == StreamType::STREAM_OUT) {
			node.streamOut.name = stream.name;
			node.streamOut.position = stream.position;
			node.streamOut.answer = stream.values;
			node.UP = PROGRAM_WIDTH * (PROGRAM_HEIGHT - 1) + stream.position;
			nodes[node.UP].DOWN = node.id;
		}

		nodes.push_back(node);
	}
}

void TickVM()
{
	for (int i = 0; i < nodes.size(); i++) {
		NodeReadUpdate(nodes[i]);
	}
	for (int i = 0; i < nodes.size(); i++) {
		NodeTick(nodes[i]);
	}
	for (int i = 0; i < nodes.size(); i++) {
		NodeWriteUpdate(nodes[i]);
	}
}

void ExitVM()
{
	nodes.clear();
}