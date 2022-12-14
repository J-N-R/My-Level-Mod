#pragma once
#include "SA2ModLoader.h"
#include "IniReader.h"
#include "FunctionHook.h"
#include <string>
#include <vector>

class LevelImporter {
	public:
		LevelImporter(const char* path, const HelperFunctions& helperFunctions);
		void importLevel(std::string landTableName = "", std::string levelFileName = "", std::string texturePakName = "");
		void init();
		void onFrame();
		void onLevelHook();

	private:
		float version;
		std::vector<std::vector<NJS_TEXNAME>> customTexnames;
		std::vector<NJS_TEXLIST> customTexlists;
		const char* path;
		IniReader* iniReader;
		const HelperFunctions& helperFunctions;
		void checkForUpdate();
		void fixFileStructure();
		static void printDebug(std::string message);
		static int writer(char* data, size_t size, size_t nmemb, std::string* buffer);
};
