#pragma once
#include <unordered_map>
#include <string>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

extern HMODULE dll_handle;

inline std::string GetDLLPath(const std::string& path)
{
	char dllPath[MAX_PATH] = { 0 };
	GetModuleFileNameA(dll_handle, dllPath, MAX_PATH);

	std::string fullPath(dllPath);

	size_t lastSlash = fullPath.find_last_of("\\/");
	std::string baseDirectory = (lastSlash != std::string::npos) ? fullPath.substr(0, lastSlash + 1) : "";

	std::ostringstream ss;
	ss << baseDirectory << path;

	return ss.str();
}

struct SkillPair
{
	uint16_t skillOriginal;
	uint16_t skillOverride;
};

inline SkillPair GetPairFromLine(const std::string& line, int lineNum)
{
	size_t equalPos = line.find('=');

	if (equalPos == std::string::npos)
		return { 0,0 };

	std::string first = line.substr(0, equalPos - 1);
	std::string second = line.substr(equalPos);

	std::erase_if(first, [](unsigned char c) { return !std::isdigit(c); });

	auto index = std::find_if(first.begin(), first.end(), [](unsigned char c) { return std::isdigit(c); });

	if (index == first.end())
	{
		std::cout << "Line: " << lineNum << " contains no digits!\n";
		return { 0,0 };
	}

	uint16_t original = std::stoi(first);

	std::erase_if(second, [](unsigned char c) { return !std::isdigit(c); });

	index = std::find_if(second.begin(), second.end(), [](unsigned char c) { return std::isdigit(c); });

	if (index == second.end())
	{
		std::cout << "Line: " << lineNum << " contains no digits!\n";
		return { 0,0 };
	}

	uint16_t replacement = std::stoi(second);

	return { original, replacement };
}

inline void ParseFileText(std::ifstream& file, std::unordered_map<uint16_t, uint16_t>& map)
{
	if (!file.is_open())
	{
		std::cout << "[Skill Effect Override] Config failed to open!\n";
		return;
	}

	std::string line;
	int lineNum = 0;
	while (std::getline(file, line))
	{
		lineNum++;
		if (line.empty())
			continue;

		if (line[0] == '#')
			continue;

		auto pair = GetPairFromLine(line, lineNum);
		map.emplace(pair.skillOriginal, pair.skillOverride);
	}
}