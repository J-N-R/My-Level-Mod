/**
 * LevelImporter.cpp
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
#include "SetupHelpers.h"
#include "IniReader.h"
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <filesystem>
#include <vector>
// By default, LevelImporter supports up to 256 custom textures.
// Change the number in here if you need more, the game has a max of 500.
#define NUMBER_OF_TEXTURES 256

LevelImporter::LevelImporter(
		const char* modFolderPath,
		const HelperFunctions& helperFunctions)
			: helperFunctions(helperFunctions) {
	this->iniReader = new IniReader(modFolderPath);
	this->modFolderPath = std::string(modFolderPath);
	this->gdPCPath = std::string(modFolderPath).append("\\gd_PC\\");
	this->PRSPath = std::string(gdPCPath).append("PRS\\");
}

void LevelImporter::importLevel(std::string landTableName) {
	importLevel(landTableName, {});
}

void LevelImporter::importLevel(std::string landTableName, LevelOptions levelOptions) {
	importLevel(
		landTableName,
		detectFile(gdPCPath, "sa2blvl"),
		detectFile(PRSPath, "pak"),
		levelOptions
	);
}

void LevelImporter::importLevel(std::string landTableName, std::string levelFileName, std::string pakFileName) {
	return importLevel(landTableName, levelFileName, pakFileName, {});
}

void LevelImporter::importLevel(std::string landTableName, std::string levelFileName, std::string pakFileName, LevelOptions levelOptions) {
	if (landTableName.empty()) {
		printDebug("Invalid land table name or level ID. Skipping import");
		return;
	}
	if (levelFileName.empty() || pakFileName.empty()) {
		printDebug("Invalid level or pak file name. Skipping import.");
		return;
	}
	ImportRequest request;
	request.levelID = getLevelID(landTableName);
	request.landTableName = landTableName;
	request.levelFileName = levelFileName;
	request.pakFileName = pakFileName,
	request.levelOptions = levelOptions;
	importRequests.push_back(request);
	// Positions must be registered before the level is loaded.
	registerPosition(levelOptions.startPosition, request.levelID, true);
	registerPosition(levelOptions.endPosition, request.levelID, false);
}

void LevelImporter::importLevel(LevelIDs levelID) {
	importLevel(levelID, {});
}

void LevelImporter::importLevel(LevelIDs levelID, LevelOptions levelOptions) {
	importLevel(getLandTableName(levelID), levelOptions);
}

void LevelImporter::importLevel(LevelIDs levelID, std::string levelFileName, std::string pakFileName) {
	importLevel(levelID, levelFileName, pakFileName, {});
}

void LevelImporter::importLevel(LevelIDs levelID, std::string levelFileName, std::string pakFileName, LevelOptions levelOptions) {
	importLevel(getLandTableName(levelID), levelFileName, pakFileName, levelOptions);
}

void LevelImporter::importLevels(std::vector<ImportRequest> requests) {
	for (ImportRequest request : requests) {
		if (request.levelFileName.empty() || request.pakFileName.empty()) {
			if (request.levelID != -1) {
				importLevel(request.levelID, request.levelOptions);
			} else {
				importLevel(request.landTableName, request.levelOptions);
			}
			continue;
		}
		importLevel(
			request.landTableName,
			request.levelFileName,
			request.pakFileName,
			request.levelOptions
		);
	}
}

std::string LevelImporter::getLandTableName(LevelIDs levelID) {
	return "objLandTable00" + std::to_string(levelID);
}

LevelIDs LevelImporter::getLevelID(std::string landTableName) {
	size_t lastDot = landTableName.find_last_of("00");
	if (lastDot != std::string::npos) {
		return (LevelIDs)std::stoi(landTableName.substr(lastDot));
	}
	return  (LevelIDs)-1;
}

void LevelImporter::onFrame() {
	if (activeLandTable != nullptr) {
		float simpleDeathPlane = activeOptions.simpleDeathPlane;
		// Enable simple death plane.
		if (simpleDeathPlane == DISABLED_PLANE) {
			return;
		}
		if (MainCharObj1 != nullptr
				&& MainCharObj1[0] != nullptr
				&& MainCharObj1[0]->Position.y <= simpleDeathPlane) {
			GameState = GameStates_NormalRestart;
		}
	}
}

void LevelImporter::onLevelLoad() {
	onLevelExit();
	bool hasActiveRequest = false;
	ImportRequest activeRequest;
	for (ImportRequest request : importRequests) {
		LevelIDs levelID = request.levelID;
		if (CurrentLevel == levelID) {
			hasActiveRequest = true;
			activeRequest = request;
			break;
		}
	}
	if (!hasActiveRequest) {
		return;
	}
	printDebug("Custom level load detected.");
	printDebug("Attempting to import \"" + activeRequest.levelFileName +
		".sa2blvl\" with texture pack \"" + activeRequest.pakFileName +
		".pak\" over land table \"" + activeRequest.landTableName + ".\"");
	// Import level.
	activeLandTable = new LandTableInfo(
		gdPCPath + removeFileExtension(activeRequest.levelFileName).append(".sa2blvl")
	);
	NJS_TEXNAME* customTextureNames = new NJS_TEXNAME[NUMBER_OF_TEXTURES]{};
	NJS_TEXLIST* texList = new NJS_TEXLIST{ customTextureNames, NUMBER_OF_TEXTURES };
	LandTable* newLandTable = activeLandTable->getlandtable();
	if (newLandTable == nullptr) {
		printDebug("Error generating land table from the given files. "
			"Skipping import.");
		return;
	}
	newLandTable->TextureList = texList;
	newLandTable->TextureName = _strdup(removeFileExtension(activeRequest.pakFileName).c_str());
	replaceLandTable(newLandTable, activeRequest.landTableName);

	// Setup level features.
	LevelOptions options = activeRequest.levelOptions;
	// Positions are pre-registered in the importLevel function.
	auto positionToString = [](NJS_VECTOR v) {
		return
			std::to_string(v.x) + ", " +
			std::to_string(v.y) + ", " +
			std::to_string(v.z) + ".";
	};
	printDebug("Setting spawn position to: " +
		positionToString(options.startPosition));
	printDebug("Setting victory position to: " +
		positionToString(options.endPosition));
	if (options.simpleDeathPlane != DISABLED_PLANE) {
		printDebug("Setting simple death plane to: " +
			std::to_string(options.simpleDeathPlane));
	}
	if (!options.splineFileNames.empty()) {
		printDebug("Spline files detected.");
		activeSplines = iniReader->readSplines(options.splineFileNames);
		if (activeSplines != nullptr) {
			LoadStagePaths(activeSplines);
		}
	}
	else if (importRequests.size() == 1) {
		printDebug("Attempting to look for splines.");
		activeSplines = iniReader->readSplines(options.splineFileNames);
		if (activeSplines != nullptr) {
			LoadStagePaths(activeSplines);
		}
	}
	printDebug("Level import was successful.");
}

void LevelImporter::replaceLandTable(LandTable* newLandTable, std::string landTableName) {
	LandTable* oldLandTable = (LandTable*)GetProcAddress(
		**datadllhandle,
		landTableName.c_str()
	);
	*oldLandTable = *newLandTable;
}

std::string LevelImporter::detectFile(std::string path, std::string fileExtension) {
	for (const auto& file : std::filesystem::directory_iterator(path)) {
		const auto fileName = file.path().filename();
		if (fileName.extension().string() == "." + fileExtension) {
			printDebug("Detected level file, using \"" + fileName.string() +
				"\" for import.");
			return fileName.stem().string();
		}
	}
	return std::string();
}

void LevelImporter::registerPosition(
		NJS_VECTOR position,
		LevelIDs levelID,
		bool isStart) {
	StartPosition startPosition = {
		(short)levelID,
		0, // Single player rotation
		0, // Multiplayer, P1 rotation
		0, // Multiplayer, P2 rotation
		position, // Single player
		position, // Multiplayer, P1
		position  // Multiplayer, P2
	};
	if (isStart) {
		helperFunctions.RegisterStartPosition(CurrentCharacter, startPosition);
	}
	else {
		helperFunctions.RegisterEndPosition(CurrentCharacter, startPosition);
	}
}

void LevelImporter::onLevelExit() {
	activeOptions = {};
	if (activeSplines != nullptr) {
		delete[] activeSplines;
	}
	if (activeLandTable != nullptr &&
		activeLandTable->getlandtable() != nullptr) {
		delete[] activeLandTable->getlandtable()->TextureList->textures;
		delete activeLandTable->getlandtable()->TextureList;
		delete activeLandTable->getlandtable();
		delete activeLandTable;
	}
}

void LevelImporter::free() {
	if (activeLandTable != nullptr &&
			activeLandTable->getlandtable() != nullptr) {
		delete[] activeLandTable->getlandtable()->TextureList->textures;
		delete activeLandTable->getlandtable()->TextureList;
		delete activeLandTable->getlandtable();
		delete activeLandTable;
	}
	if (activeSplines != nullptr) {
		delete[] activeSplines;
	}
	if (iniReader != nullptr) {
		delete iniReader;
	}
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
