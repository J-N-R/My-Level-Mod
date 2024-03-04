#pragma once
#include "pch.h"
#include "IniReader.h"
#include <string>
#include <vector>
#include <curl/curl.h>

class LevelImporter {
	public:
		LevelImporter(const char* path, const HelperFunctions& helperFunctions);
		void importLevel(int levelID);
		void importLevel(std::string landTableName);
		void importLevel(std::string landTableName, std::string levelFileName);
		void importLevel(std::string landTableName, std::string levelFileName, std::string texturePakName);
		void init();
		void onFrame();
		void onLevelHook();
		std::string getLandTableName(int levelID);

	private:
		float version;
		std::vector<std::vector<NJS_TEXNAME>> customTexnames;
		std::vector<NJS_TEXLIST> customTexlists;
		std::string modFolderPath;
		std::string gdPCPath;
		std::string PRSPath;
		IniReader* iniReader;
		const HelperFunctions& helperFunctions;
		void checkForUpdate();
		void fixFileStructure();
		static void printDebug(std::string message);
		static int writer(char* data, size_t size, size_t nmemb, std::string* buffer);
};
