#pragma once
#include "pch.h"
#include <string>
#include <vector>

class IniReader {
	public:
		int levelID;
		bool hasSimpleDeathPlane;
		float simpleDeathPlane;
		std::vector<std::vector<std::string>> importLevelQueries;
		IniReader(const char* path, const HelperFunctions& helperFunctions);
		void readLevelOptions();
		LoopHead** readSplines();
		LoopHead* readSpline(std::string filePath);

	private:
		const char* optionsPath;
		const char* gdPCPath;
		const HelperFunctions& helperFunctions;
		void printDebug(std::string message);
};
