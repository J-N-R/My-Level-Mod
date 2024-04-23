#pragma once
#include "pch.h"
#include <string>
#include <vector>

class IniReader {
	public:
		IniReader(const char* path);
		std::vector<ImportRequest> readLevelOptions();
		LoopHead** readSplines(std::vector<std::string> splineFileNames);
		LoopHead* readSpline(std::string filePath);

	private:
		const char* optionsPath;
		const char* gdPCPath;
		// Parses a comma seperated string for a position variable.
		NJS_VECTOR getPosition(std::string position);
		// Parses a comma seperated string and returns the tokens.
		std::vector<std::string> getTokens(std::string value);
};
