/*
 * LevelImporter.cpp Version 4.2
 *
 * Description:
 *    A c++ class dedicated to making importing levels into Sonic Adventure 2
 *    easier. Using assets provided by the mod folder and a level_options.ini
 *    file, this class allows users to import levels into SA2 with minimal
 *    coding knowledge.
 *
 *	  To debug issues with this code, enable SA2ModLoader's "file" debug system
 *	  and check your Sonic Adventure 2 install folder for auto-generated log
 *	  files.
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
#include <vector>
#define VERSION 4.2f
// By default, LevelImporter supports up to 1000 custom textures.
// Change the number in here if you need more.
#define NUMBER_OF_TEXTURES 1000
// Whether My Level Mod should automatically check the internet for new updates.
// Used to save an update notification to the mod folder.
#define CHECK_FOR_UPDATE true
// Whether My Level Mod should automatically attempt to fix mod file structure
// issues. If issues are fixed, My Level Mod will require a restart.
#define FIX_FILE_STRUCTURE true

LevelImporter::LevelImporter(const char* path,
		const HelperFunctions& helperFunctions)
			: helperFunctions(helperFunctions) {
	this->path = path;
	this->version = VERSION;
	this->iniReader = new IniReader(path, helperFunctions);
}

/**
 * Dynamically imports levels by parsing level_options.ini. Will automatically
 * check for updates to My Level Mod and fix potential file structure errors if
 * those features are enabled.
 */
void LevelImporter::init() {
	iniReader->readLevelOptions();
	if (CHECK_FOR_UPDATE) {
		checkForUpdate();
	}
	if (FIX_FILE_STRUCTURE) {
		fixFileStructure();
	}
	
	// If a level id was provided in level_options.ini, perform a simple import.
	if (iniReader->levelID != -1) {
		importLevel(iniReader->levelID);
	}

	// If complex import queries were provided in level_options.ini, perform
	// complex imports. Used when importing multiple levels with one mod.
	for (std::vector<std::string> parameters : iniReader->importLevelQueries) {
		switch (parameters.size()) {
			case 1:
				importLevel(parameters[0]);
				break;
			case 2:
				importLevel(parameters[0], parameters[1]);
				break;
			case 3:
				importLevel(parameters[0], parameters[1], parameters[2]);
				break;
			default:
				printDebug("(Warning) Broken Import_Level query found in"
					"level_options.ini. Disregarding.");
		}
	}
}

/**
 * Imports a custom level over an existing level through its levelID, using
 * any sa2blvl and PAK file found in the mod folder.
 */
void LevelImporter::importLevel(int levelID) {
	if (iniReader->levelID == -1) {
		printDebug("(Warning) landTableName and levelID not found. "
			"Cancelling level import.");
		return;
	}
	// Try to find pak & sa2blvl files to use in the mod folder.
	const std::string gdPCPath = std::string(path) + "\\gd_PC\\";
	const std::string PRSPath = gdPCPath + "PRS\\";
	std::string levelFileName = "", texturePakName = "";
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
	importLevel(getLandTableName(iniReader->levelID), levelFileName, texturePakName);
}

/**
 * Imports a custom level over an existing level, using the landTableName as
 * the levelFileName and texturePakName.
 */
void LevelImporter::importLevel(std::string landTableName) {
	importLevel(landTableName, landTableName, landTableName);
}

/**
 * Imports a custom level over an existing level, using the levelFileName as
 * the texturePakName.
 */
void LevelImporter::importLevel(std::string landTableName, std::string levelFileName) {
	importLevel(landTableName, levelFileName, levelFileName);
}

/** 
 * Imports a custom level over an existing level.
 * 
 * @param [landTableName] - The name of the land table you want to replace.
 * @param [levelFileName] - The name of the .sa2blvl file in your mod folder
 *    to use for the import.
 * @param [texturePakName] - The name of the .pak file in your mod folder to
 *    use for the import.
 */
void LevelImporter::importLevel(std::string landTableName,
		std::string levelFileName, std::string texturePakName) {
	// Remove file extension from levelFileName and texturePakName.
	size_t lastDot = levelFileName.find_last_of(".");
	if (lastDot != std::string::npos) {
		levelFileName = levelFileName.substr(0, lastDot);
	}
	lastDot = texturePakName.find_last_of(".");
	if (lastDot != std::string::npos) {
		texturePakName = texturePakName.substr(0, lastDot);
	}

	// Grab original land table and replace with custom land table.
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

	printDebug("Attempting to import level: " + levelFileName + " with texture "
		"pack: " + texturePakName + " over land table: " + landTableName + ".");
	
	// Set land table to use custom textures. A global vector is used so multiple
	// texture PAKs can be used and persist outside of this function.
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
	printDebug("Import successful.");
}

/**
 * Since Chao gardens are not numbered in the game, this function attempts to
 * number the Chao gardens based on the pre-existing numbering pattern of
 * levels. The final numbered level is 59 (Death Chamber 2P) and assuming the
 * boss levels were numbered, the final boss is 66 (Biolizard). Therefore, Chao
 * gardens level ids start at 67 (objLandTableLobby000).
 *
 * @returns an empty string if a land table is not found.
 */
std::string LevelImporter::getLandTableName(int levelID) {
	if (levelID < 60) {
		return "objLandTable00" + std::to_string(levelID);
	}
	switch (levelID) {
		case 67:
			return "objLandTableLobby000";
		case 68:
			return "objLandTableLobby00k";
		case 69:
			return "objLandTableLobby0dk";
		case 70:
			return "objLandTableLobbyh0k";
		case 71:
			return "objLandTableLobbyhdk";
		case 72:
			return "objLandTableLobby";
		case 73:
			return "objLandTableDark";
		case 74:
			return "objLandTableHero";
		case 75:
			return "objLandTableNeut";
		case 76:
			return "objLandTableStadium";
		case 77:
			return "objLandTableEntrance";
		case 78:
			return "objLandTableRace";
		case 79:
			return "objLandTableRaceDark";
		case 80:
			return "objLandTableRaceHero";
		case 81:
			return "objLandTableChaoKarate";
		case 82:
			return "objLandTableKinderBl";
		case 83:
			return "objLandTableKinderCl";
		case 84:
			return "objLandTableKinderCo";
		case 85:
			return "objLandTableKinderFo";
		case 86:
			return "objLandTableKinderHe";
		case 87:
			return "objLandTableKinderHo";
		case 88:
			return "objLandTableKinderPr";
		case 89:
			return "objLandTableKinderLi";
		case 90:
			return "objLandTableKinderPl";
		default:
			return "";
	}
}

/**
 * Automatically detect and attempt to fix files being in the wrong folder. If
 * any fixes occur, the game will unforunately need a restart.
 */ 
void LevelImporter::fixFileStructure() {
	const std::string gdPCPath = std::string(path) + "\\gd_PC\\";
	const std::string PRSPath = gdPCPath + "PRS\\";
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		const auto filePath = file.path();
		if (filePath.extension().string() == ".sa2blvl") {
			printDebug("(ERROR) The level file has been detected to be in the "
				"wrong folder.");
			if (std::rename(filePath.string().c_str(),
					(gdPCPath + filePath.filename().string()).c_str()) == 0) {
				printDebug("Successfully moved the level file to the folder "
					"~yourModFolder\\gd_PC\\.");
				printDebug("Expect a game crash, please run the game again.");
			}
			else {
				printDebug("(Warning) Error moving the level file to the "
					"right folder.");
				printDebug("(Warning) The level file should be saved to"
					"(~yourModFolder\\gd_PC\\(your-level).sa2blvl).");
			}
		}
		else if (filePath.extension().string() == ".pak") {
			printDebug("(ERROR) The texture pack file has been detected to be "
				"in the wrong folder.");
			if (std::rename(filePath.string().c_str(),
					(PRSPath + filePath.filename().string()).c_str()) == 0) {
				printDebug("Successfully moved the texture pack file to the "
					"folder ~yourModFolder\\gd_PC\\.");
				printDebug("Expect a game crash, please run the game again.");
			}
			else {
				printDebug("(Warning) Error moving the texture pack file to "
					"the right folder.");
				printDebug("(Warning) The texture pack file should be saved "
					"to (~yourModFolder\\gd_PC\\PRS\\(your-texture-pak).pak).");
			}
		}
	}
	if (iniReader->levelID != -1) {
		for (const auto& file : std::filesystem::directory_iterator(gdPCPath)) {
			const auto filePath = file.path();
			std::string levelIDString = std::to_string(iniReader->levelID);
			if (filePath.extension().string() == ".bin" &&
					filePath.filename().string()
						.find(levelIDString) == std::string::npos) {
				std::stringstream ss(filePath.filename().string());
				std::vector<std::string> tokens;
				std::string token;
				while (getline(ss, token, '_')) {
					tokens.push_back(token);
				}
				std::string fileName = "set00" + levelIDString;
				tokens.erase(tokens.begin());
				for (std::string _token : tokens) {
					fileName += "_" + _token;
				}
				printDebug("(ERROR) A SET file that doesn't match the level ID: "
					+ levelIDString + " was found.");
				if (std::rename(filePath.string().c_str(),
						(gdPCPath + fileName).c_str()) == 0) {
					printDebug("Successfully renamed " +
						filePath.filename().string() + " to " + fileName + ".");
					printDebug("Expect a game crash, please run game again.");
				}
				else {
					printDebug("(ERROR) Error renaming the SET file to match "
						"the level ID.");
					printDebug("(ERROR) The SET file should be named " +
						fileName + ".");
				}
			}
			else if (filePath.extension().string() == ".pak") {
				printDebug("(ERROR) The texture pack file has been detected to be "
					"in the wrong folder.");
				if (std::rename(filePath.string().c_str(),
					(PRSPath + filePath.filename().string()).c_str()) == 0) {
					printDebug("Successfully moved the texture pack file to the "
						"folder ~yourModFolder\\gd_PC\\.");
					printDebug("Expect a game crash, please run the game again.");
				}
				else {
					printDebug("(Warning) Error moving the texture pack file to "
						"the right folder.");
					printDebug("(Warning) The texture pack file should be saved "
						"to (~yourModFolder\\gd_PC\\PRS\\(your-texture-pak).pak).");
				}
			}
		}
	}
}

/**
 * Check the internet for an update to My Level Mod and save a notification
 * file in the mod folder if an update is detected.
 */
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
	// Disable cached HTML requests.
	struct curl_slist* headers = NULL;
	headers = curl_slist_append(headers, "Cache-control: no-cache");

	// Read version number from github, store result as string.
	curl_easy_setopt(curl, CURLOPT_URL,
		"https://raw.githubusercontent.com/J-N-R/"
			"My-Level-Mod/master/VERSION.txt");
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, this->writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
	CURLcode curlResult = curl_easy_perform(curl);
	if (curlResult != CURLE_OK) {
		printDebug("(Warning) Could not check for update. "
			"[Error reaching internet]");
		return;
	}

	// Save a notification file if an update is detected.
	if (VERSION < std::stof(result)) {
		printDebug("Update detected! Creating update reminder.");
		std::string updatePath = std::string(path) + "\\MANUALLY UPDATE TO "
			"VERSION " + std::to_string(std::stof(result)) + ".txt";
		if (!std::filesystem::exists(updatePath)) {
			std::ofstream updateFile;
			updateFile.open(updatePath, std::ofstream::out);
			updateFile << "An update has been detected for My Level Mod!" <<
				std::endl;
			updateFile << "Please download and update your mod manually. "
				"Download link:" << std::endl;
			updateFile << "https://github.com/J-N-R/My-Level-Mod/"
				"releases" << std::endl;
			updateFile.close();
		}
	}
	// Delete existing notification files if My Level Mod is up to date.
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

/**
 * Enables features that require constant frame checking to work. Currently
 * used to enable the simple death plane in levels imported by level id.
 */
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

/**
 * Enables features that require running when a level loads. Currently used to
 * enable loading splines for levels imported by level id.
 */
void LevelImporter::onLevelHook() {
	if (iniReader->levelID != -1 && CurrentLevel == iniReader->levelID) {
		printDebug("Level load detected. Loading splines.");

		LoopHead** splines = iniReader->readSplines();
		if (splines) {
			LoadStagePaths(splines);
		}
	}
}

/** Helper function to print debug messages. */
void LevelImporter::printDebug(std::string message) {
	PrintDebug(("[My Level Mod] " + message).c_str());
}

/** Helper function for checkForUpdate(). */
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
