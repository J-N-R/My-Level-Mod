/**
 * SetupHelpers.cpp
 *
 * Description:
 *    A file dedicated to setting up My Level Mod.
 *
 *    X-Hax discord for code questions: https://discord.gg/gqJCF47
 */

#include "pch.h"
#include "IniReader.h"
#include "LevelImporter.h"
#include "SetupHelpers.h"
#include <curl/curl.h>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>
// Current version of My Level Mod.
#define VERSION 4.5f
#define UPDATE_URL "https://raw.githubusercontent.com/J-N-R/My-Level-Mod/master/VERSION.txt"
 // Whether My Level Mod should check the internet for updates to My Level Mod.
#define CHECK_FOR_UPDATE true
// Whether My Level Mod should attempt to detect and fix file structure issues.
#define FIX_FILE_STRUCTURE true
#define DEFAULT_SET_FILE "default_set_file.bin"

void myLevelModInit(const char* modFolderPath, LevelImporter* levelImporter) {
	IniReader* iniReader = new IniReader(modFolderPath);
	if (CHECK_FOR_UPDATE) {
		checkForUpdate(modFolderPath);
	}
	std::vector<ImportRequest> requests = iniReader->readLevelOptions();
	levelImporter->importLevels(requests);
	if (FIX_FILE_STRUCTURE) {
		for (ImportRequest request : levelImporter->importRequests) {
			LevelIDs levelID = request.levelID;
			if (!request.landTableName.empty()) {
				levelID = levelImporter->getLevelID(request.landTableName);
			}
			fixFileStructure(modFolderPath, levelID);
		}
	}
	delete iniReader;
}

void checkForUpdate(const char* modFolderPath) {
	printDebug("Checking for updates...");
	curl_global_init(CURL_GLOBAL_ALL);
	std::string result;
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
	curl_easy_setopt(curl, CURLOPT_URL, UPDATE_URL);
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
	CURLcode curlResult = curl_easy_perform(curl);
	if (curlResult != CURLE_OK) {
		printDebug("(Warning) Could not check for update. [" +
			std::string(curl_easy_strerror(curlResult)) + "]");
	}

	// Save a notification file if an update is detected.
	else if (VERSION < std::stof(result)) {
		printDebug("Update detected! Creating update reminder.");
		std::string updatePath = std::string(modFolderPath) + "\\Mod "
			"developers: manually UPDATE to VERSION " + 
			std::to_string(std::stof(result)) + ".txt";
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
		for (const auto& file : std::filesystem::directory_iterator(modFolderPath)) {
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

void fixFileStructure(const char* modFolderPath, LevelIDs levelID) {
	std::string gdPCPath = std::string(modFolderPath).append("\\gd_PC\\");
	std::string PRSPath = std::string(gdPCPath).append("PRS\\");
	for (const auto& file : std::filesystem::directory_iterator(modFolderPath)) {
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
	if (levelID != LevelIDs_Invalid) {
		auto isSetFile = [](auto filePath, std::string levelIDString, char type) -> bool {
			// C++ 11 forces deep copy.
			std::string fileName = filePath.filename().string();
			std::transform(
				fileName.begin(),
				fileName.end(),
				fileName.begin(),
				::tolower
			);
			std::string typeSuffix;
			typeSuffix.push_back('_');
			typeSuffix.push_back(type);
			return
				filePath.extension().string() == ".bin" &&
				fileName.find(levelIDString) != std::string::npos &&
				fileName.find(typeSuffix) != std::string::npos;
		};

		auto createSetFile = [](std::string path, std::string levelIDString, char type) {
			std::string warningMessage("(Warning) \"");
			warningMessage += type;
			printDebug(warningMessage + "\" type SET file is missing for "
				"level_id=" + levelIDString + ".");
			printDebug("(Warning) Creating missing SET file. This may cause a "
				"game crash.");
			std::string targetFileName = 
				"set00" + levelIDString + '_' + type + ".bin";
			std::filesystem::copy_file(
				path + DEFAULT_SET_FILE,
				path + targetFileName
			);
		};

		bool sSetFileExists = false, uSetFileExists = false;
		std::string levelIDString = std::to_string(levelID);
		for (const auto& file : std::filesystem::directory_iterator(gdPCPath)) {
			const auto filePath = file.path();
			if (isSetFile(filePath, levelIDString, 's')) {
				sSetFileExists = true;
			}
			if (isSetFile(filePath, levelIDString, 'u')) {
				uSetFileExists = true;
			}
			if (filePath.extension().string() == ".pak") {
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
		if (!sSetFileExists) {
			createSetFile(gdPCPath, levelIDString, 's');
		}
		if (!uSetFileExists) {
			createSetFile(gdPCPath, levelIDString, 'u');
		}
	}
}

int writer(char* data, size_t size,
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