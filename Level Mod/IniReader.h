#pragma once
#include "SA2ModLoader.h"
#include <string>
#include <vector>

class IniReader {
	public:
		int levelID;
		bool hasSimpleDeathPlane;
		float simpleDeathPlane;
		std::vector<std::vector<std::string>> importLevelQueries;
		IniReader(const char* path, const HelperFunctions& helperFunctions);
		void loadIniOptions();
		LoopHead** loadSplines();
		std::string getChaoGarden();
		std::string getLevelID();

	private:
		const char* optionsPath;
		const char* gdPCPath;
		const HelperFunctions& helperFunctions;
		void printDebug(std::string message);
};
