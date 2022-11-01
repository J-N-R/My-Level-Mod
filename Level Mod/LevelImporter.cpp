/**
 * LevelImporter.cpp Version 4
 *
 * Description:
 *    A c++ class dedicated to making and importing levels into Sonic
 *    Adventure 2 easier. Using the simple replacement method as well
 *    as an exposed options ini file, it allows users to create level
 *    mods without writing any code.
 *
 *    All reports / error messages will be printed to SA2ModLoader's
 *    debug system. Enable 'file' in the SA2ModLoader debug menu to
 *    see My Level Mod debug messages.
 * 
 *    Currently maintained by https://github.com/J-N-R.
 *
 *    If you have any questions, feel free to ask at the x-hax discord:
 *    https://discord.gg/gqJCF47
 */

#include "pch.h"
#include "LevelImporter.h"
#include "IniReader.h"
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <filesystem>
#include <curl/curl.h>
#include <functional>
#include <vector>
#define VERSION 4.0f
// By default, LevelImporter supports up to 1000 custom textures.
// Change the number in here if you need more.
#define NUMBER_OF_TEXTURES 1000
// My Level Mod uses libcurl to connect to the internet to detect updates.
#define CHECK_FOR_UPDATE true

LevelImporter::LevelImporter(const char* path,
		const HelperFunctions& helperFunctions)
			: helperFunctions(helperFunctions) {
	this->path = path;
	this->version = VERSION;
	this->iniReader = new IniReader(path, helperFunctions);
}

void LevelImporter::init() {
	// Check for any updates.
	if (CHECK_FOR_UPDATE) {
		checkForUpdate();
	}
	// Fix any file structure problems.
	fixFileStructure();

	// Read from level_options.ini.
	iniReader->loadIniOptions();

	// Create simple importLevel() queries from read options.
	if (iniReader->levelID != -1 || !iniReader->getChaoGarden().empty()) {
		importLevel();
	}
	// Create complex importLevel() queries from read options.
	for (std::vector<std::string> parameters : iniReader->importLevelQueries) {
		switch (parameters.size()) {
			case 1:
				// importLevel(parameters[0]);
				printDebug("importLevel(" + parameters[0]);
				break;
			case 2:
				// importLevel(parameters[0], parameters[1]);
				printDebug("importLevel(" + parameters[0] + ", " + parameters[1] + ")");
				break;
			case 3:
				// importLevel(parameters[0], parameters[1], parameters[2]);
				printDebug("importLevel(" + parameters[0] + ", " + parameters[1] + ", " + parameters[2] + ")");
				break;
			default:
				printDebug("(Warning) Strange Import_Level query found in"
					"level_options.ini. Disregarding.");
		}
	}
}

/**
 * @function importLevel
 * @description Imports a custom level over an existing level. Parameters are
 * optional.
 *
 * @param [landTableName] - The original land table you want to replace. Uses
 *	  level id in level_options by default. Uses any .sa2blvl and .pak found by
 *	  default.
 * @param [levelFileName] - The .sa2blvl file you want to import. Uses
 *	  landTableName as file name by default.
 * @param [texturePakName] - Name of the .pak file you want to use. Uses
 *	  levelFileName for the pak name as default.
 */
void LevelImporter::importLevel(std::string landTableName,
		std::string levelFileName,
		std::string texturePakName) {
	// If no parameters are given, use levelID from iniReader and any .pak /
	// .sa2blvl file you can find.
	if (landTableName.empty()) {
		if (!iniReader->getChaoGarden().empty()) {
			landTableName = iniReader->getChaoGarden();
		}
		else if (iniReader->levelID == -1) {
			printDebug("(Warning) landTableName and levelID not found. "
				"Cancelling level import.");
			return;
		}
		else {
			landTableName = "objLandTable00" + iniReader->getLevelID();
		}
		levelFileName = landTableName;
		texturePakName = landTableName;

		// Try to find pak & sa2blvl files to use.
		const std::string gdPCPath = std::string(path) + "\\gd_PC\\";
		const std::string PRSPath = std::string(path) + "\\gd_PC\\PRS\\";
		for (const auto& file : std::filesystem::directory_iterator(gdPCPath)) {
			const auto filePath = file.path();
			if (filePath.extension().string() == ".sa2blvl") {
				levelFileName = filePath.stem().string();
			}
		}
		for (const auto& file : std::filesystem::directory_iterator(PRSPath)) {
			const auto filePath = file.path();
			if (filePath.extension().string() == ".pak") {
				texturePakName = filePath.stem().string();
			}
		}
	}
	// If only a landtable is given, assume .sa2blvl and .pak file are named
	// after the landtable. (e.g. objLandTable0013.sa2blvl)
	else if (levelFileName.empty()) {
		levelFileName = landTableName;
		texturePakName = landTableName;
	}
	// If a landtable and an .sa2blvl file is given, assume the texture pak
	// is named after the .sa2blvl file.
	else if (texturePakName.empty()) {
		// Remove file extension from levelFileName and texturePakName.
		size_t lastDot = levelFileName.find_last_of(".");
		if (lastDot != std::string::npos) {
			levelFileName = levelFileName.substr(0, lastDot);
		}
		texturePakName = levelFileName;
	}
	// If everything is given, use given file names.
	else {
		// Remove file extension from levelFileName and texturePakName.
		size_t lastDot = levelFileName.find_last_of(".");
		if (lastDot != std::string::npos) {
			levelFileName = levelFileName.substr(0, lastDot);
		}
		lastDot = levelFileName.find_last_of(".");
		if (lastDot != std::string::npos) {
			levelFileName = levelFileName.substr(0, lastDot);
		}
	}
	
	// Grab original landtable and replace with ours.
	HMODULE v0 = **datadllhandle;
	LandTable* Land = (LandTable*)GetProcAddress(v0, landTableName.c_str());
	try {
		*Land = *(new LandTableInfo(std::string(path) + "\\gd_PC\\" +
			levelFileName + ".sa2blvl"))->getlandtable();
	}
	catch (const std::exception& e) {
		printDebug("(ERROR) Land Table not found.");
		printDebug("(ERROR) Please make sure you're creating the lvl file "
			"properly.");
		printDebug(e.what());
	}
	
	// Set landtable to use our custom textures. Here, I use a vector stored
	// in the class so that the variables and references stay persistent
	// after they leave scope. This allows me to programatically import
	// multiple levels.
	NJS_TEXNAME customTextureNames[NUMBER_OF_TEXTURES]{};
	customTexnames.push_back(
		std::vector<NJS_TEXNAME>(std::begin(customTextureNames),
			std::end(customTextureNames)));
	customTexlists.push_back({customTexnames.back().data(), NUMBER_OF_TEXTURES});
	Land->TextureList = &customTexlists.back();
	Land->TextureName = _strdup(texturePakName.c_str());

	// (Safety feature) disable blockbit system, entire level will be loaded
	// at once.
	WriteData<5>((void*)0x5DCE2D, 0x90);
}

// Automatically detect and attempt to fix files being in the wrong folder.
// Game will unforunately need a restart.
void LevelImporter::fixFileStructure() {
	const std::string gdPCPath = std::string(path) + "\\gd_PC\\";
	const std::string PRSPath = std::string(path) + "\\gd_PC\\PRS\\";

	for (const auto& file : std::filesystem::directory_iterator(path)) {
		const auto filePath = file.path();

		if (filePath.extension().string() == ".sa2blvl") {
			printDebug("(ERROR) Incorrect level file position detected.");

			if (std::rename(filePath.string().c_str(),
					(gdPCPath + filePath.filename().string()).c_str()) == 0) {
				printDebug("Successfully moved level file.");
				printDebug("Expect a game crash, please run game again.");
			}
			else {
				printDebug("(Warning) Error moving level file to the right "
					"position.");
				printDebug("(Warning) Level file should be at"
					"(\\gd_PC\\(your-level).sa2blvl).");
			}
		}
		else if (filePath.extension().string() == ".pak") {
			printDebug("(ERROR) Incorrect texture pack position detected.");

			if (std::rename(filePath.string().c_str(),
					(PRSPath + filePath.filename().string()).c_str()) == 0) {
				printDebug("Successfully moved texture pack file.");
				printDebug("Expect a game crash, please run game again.");
			}
			else {
				printDebug("(Warning) Error moving texture pack file to the "
					"right position.");
				printDebug("(Warning) Texture pack file should be at "
					"(\\gd_PC\\PRS\\(your-texture-pak).pak).");
			}
		}
	}

	if (iniReader->levelID != -1) {
		for (const auto& file : std::filesystem::directory_iterator(gdPCPath)) {
			const auto filePath = file.path();

			if (filePath.extension().string() == ".bin" &&
					filePath.filename().string()
						.find(iniReader->getLevelID()) == std::string::npos) {
				std::stringstream ss(filePath.filename().string());
				std::vector<std::string> tokens;
				std::string token;
				while (getline(ss, token, '_')) {
					tokens.push_back(token);
				}

				std::string fileName = "set00" + iniReader->getLevelID();
				tokens.erase(tokens.begin());
				for (std::string _token : tokens) {
					fileName += "_" + _token;
				}

				printDebug("(ERROR) SET file found not matching level ID: "
					+ iniReader->getLevelID() + ".");

				if (std::rename(filePath.string().c_str(),
						(gdPCPath + fileName).c_str()) == 0) {
					printDebug("Successfully renamed " +
						filePath.filename().string() + " to " + fileName + ".");
					printDebug("Expect a game crash, please run game again.");
				}
				else {
					printDebug("(ERROR) Error renaming SET file.");
					printDebug("(ERROR) SET file should be named "
						+ fileName + ".");
				}
			}
			else if (filePath.extension().string() == ".pak") {
				printDebug("(ERROR) Incorrect texture pack position detected.");

				if (std::rename(filePath.string().c_str(),
						(PRSPath + filePath.filename().string()).c_str()) == 0) {
					printDebug("Successfully moved texture pack file.");
					printDebug("Expect a game crash, please run game again.");
				}
				else {
					printDebug("(Warning) Error moving texture pack file to the "
						"right position.");
					printDebug("(Warning) Texture pack file should be at "
						"(\\gd_PC\\PRS\\(your-texture-pak).pak).");
				}
			}
		}
	}
}

// Detect if an update is available for this mod and notify if any.
void LevelImporter::checkForUpdate() {
	printDebug("Checking for updates...");

	curl_global_init(CURL_GLOBAL_ALL);

	std::string result{ };
	CURL* curl = curl_easy_init();

	if (!curl) {
		printDebug("(Warning) Could not check for update. "
			"[Curl will not instantiate]");
		return;
	}

	// No cache.
	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Cache-control: no-cache");

	// Read version number from github, store result as string.
	curl_easy_setopt(curl, CURLOPT_URL,
		"https://raw.githubusercontent.com/X-Hax/SA2BModdingGuide"
			"/master/Level%20Editing/My%20Level%20Mod/VERSION.txt");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, this->writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
	CURLcode curlResult = curl_easy_perform(curl);

	if (curlResult != CURLE_OK) {
		printDebug("(Warning) Could not check for update. "
			"[Error reaching internet]");
		return;
	}

	std::string updatePath = std::string(path) + "\\MANUALLY UPDATE TO VERSION "
		+ std::to_string(std::stof(result)) + ".txt";

	// If they need to update, add update file.
	if (VERSION < std::stof(result)) {
		printDebug("Update detected! Creating update reminder.");

		if (!std::filesystem::exists(updatePath)) {
			std::ofstream updateFile;
			updateFile.open(updatePath, std::ofstream::out);
			updateFile << "An update has been detected for My Level Mod!" <<
				std::endl;
			updateFile << "Please download and update your mod manually. "
				"Download link:" << std::endl;
			updateFile << "https://github.com/X-Hax/SA2BModdingGuide/"
				"releases" << std::endl;
			updateFile.close();
		}
	}
	// If they don't need to update, but the update file is there, get rid of it.
	else {
		printDebug("Mod up to date.");
		bool cleaned = false;

		for (const auto& file : std::filesystem::directory_iterator(path)) {
			std::string filePath = file.path().string();

			if (filePath.find("UPDATE") != std::string::npos) {
				std::filesystem::remove(filePath);
				cleaned = true;
			}
		}

		if (cleaned) {
			printDebug("Cleaned up update reminders.");
		}
	}

	curl_easy_cleanup(curl);
	curl_global_cleanup();
}

// Activate Simple Death Plane if enabled.
void LevelImporter::onFrame() {
	if (iniReader->levelID != -1 && GameState == GameStates_Ingame &&
			CurrentLevel == iniReader->levelID && 
				iniReader->hasSimpleDeathPlane) {
		if (MainCharObj1 != nullptr && MainCharObj1[0] != nullptr
				&& MainCharObj1[0]->Position.y <= iniReader->simpleDeathPlane) {
			GameState = GameStates_NormalRestart;
		}
	}
}

// Load splines into the level when the internal "LoadLevel" function gets called.
void LevelImporter::onLevelHook() {
	if (iniReader->levelID != -1 && CurrentLevel == iniReader->levelID) {
		printDebug("Level load detected. Loading splines.");

		LoopHead** splines = iniReader->loadSplines();
		if (splines) {
			LoadStagePaths(splines);
		}
	}
}

// Helper function to print debug messages.
void LevelImporter::printDebug(std::string message) {
	PrintDebug(("[My Level Mod] " + message).c_str());
}

// Helper function for checkForUpdate().
int LevelImporter::writer(char* data, size_t size,
		size_t nmemb, std::string* buffer) {
	int result = 0;
	if (buffer != NULL) {
		buffer->append(data, size * nmemb);
		result = size * nmemb;
	}
	return result;
}

/*************************************************************************
 * Copyright 2022 Google LLC
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *	https://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *************************************************************************/
