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
	this->modFolderPath = std::string(modFolderPath);
	this->gdPCPath = std::string(modFolderPath).append("\\gd_PC\\");
	this->PRSPath = std::string(gdPCPath).append("PRS\\");
}

LandTableInfo* LevelImporter::importLevel(std::string landTableName) {
	return importLevel(landTableName, nullptr);
}

LandTableInfo* LevelImporter::importLevel(std::string landTableName, LevelOptions* levelOptions) {
	return importLevel(
		landTableName,
		detectFile(gdPCPath, "sa2blvl"),
		detectFile(PRSPath, "pak"),
		levelOptions
	);
}

LandTableInfo* LevelImporter::importLevel(std::string landTableName, std::string levelFileName, std::string pakFileName) {
	return importLevel(landTableName, levelFileName, pakFileName, nullptr);
}

LandTableInfo* LevelImporter::importLevel(std::string landTableName, std::string levelFileName, std::string pakFileName, LevelOptions* levelOptions) {
	if (levelFileName.empty() || pakFileName.empty()) {
		printDebug("Invalid level or pak file name. Skipping import.");
		return nullptr;
	}
	printDebug("Attempting to import \"" + levelFileName + ".sa2blvl\" "
		"with texture pack \"" + pakFileName + ".pak\" over land table \"" +
		landTableName + ".\"");
	LandTableInfo* landTableInfo = new LandTableInfo(
		gdPCPath + removeFileExtension(levelFileName).append(".sa2blvl")
	);
	NJS_TEXNAME* customTextureNames = new NJS_TEXNAME[NUMBER_OF_TEXTURES]{};
	levels.push_back(new Level{
		landTableName,
		landTableInfo,
		customTextureNames,
		{ customTextureNames, NUMBER_OF_TEXTURES },
		levelOptions
	});
	LandTable* newLandTable = landTableInfo->getlandtable();
	if (newLandTable == nullptr) {
		printDebug("Error generating land table from the given files. "
			"Skipping import.");
		return nullptr;
	}
	LandTable* oldLandTable = (LandTable*)GetProcAddress(**datadllhandle, landTableName.c_str());
	*oldLandTable = *newLandTable;
	oldLandTable->TextureList = &levels.back()->textureList;
	oldLandTable->TextureName = _strdup(removeFileExtension(pakFileName).c_str());
	// Start positions must be registered before the level is loaded.
	if (levelOptions != nullptr) {
		registerStartPositions(levelOptions, landTableName);
	}
	printDebug("Level import was successful.");
	return landTableInfo;
}

LandTableInfo* LevelImporter::importLevel(LevelIDs levelID) {
	return importLevel(levelID, nullptr);
}

LandTableInfo* LevelImporter::importLevel(LevelIDs levelID, LevelOptions* levelOptions) {
	return importLevel(getLandTableName(levelID), levelOptions);
}

LandTableInfo* LevelImporter::importLevel(LevelIDs levelID, std::string levelFileName, std::string pakFileName) {
	return importLevel(levelID, levelFileName, pakFileName, nullptr);
}

LandTableInfo* LevelImporter::importLevel(LevelIDs levelID, std::string levelFileName, std::string pakFileName, LevelOptions* levelOptions) {
	return importLevel(getLandTableName(levelID), levelFileName, pakFileName, levelOptions);
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
	if (activeLevel != nullptr && activeLevel->levelOptions != nullptr) {
		float simpleDeathPlane = activeLevel->levelOptions->simpleDeathPlane;
		// Enable simple death plane.
		if (simpleDeathPlane == DISABLED_PLANE) {
			return;
		}
		if (MainCharObj1 != nullptr && MainCharObj1[0] != nullptr
			&& MainCharObj1[0]->Position.y <= simpleDeathPlane) {
			GameState = GameStates_NormalRestart;
		}
	}
}

void LevelImporter::onLevelLoad() {
	resetActiveLevel();
	if (activeLevel == nullptr) {
		return;
	}
	printDebug("Custom level load detected.");
	LevelOptions* levelOptions = activeLevel->levelOptions;
	if (levelOptions == nullptr) {
		printDebug("No My Level Mod features enabled, skipping load logic.");
		return;
	}
	LoopHead** splines = levelOptions->splines;
	if (splines != nullptr) {
		printDebug("Splines detected! Loading splines for level.");
		LoadStagePaths(splines);
		printDebug("Successfully loaded spline data.");
	}
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

void LevelImporter::registerStartPositions(LevelOptions* levelOptions, std::string landTableName) {
	LevelIDs levelID = getLevelID(landTableName);
	if (levelOptions->startPosition == nullptr) {
		levelOptions->startPosition = new NJS_VECTOR{ 0, 0, 0 };
	}
	if (levelOptions->endPosition == nullptr) {
		levelOptions->endPosition = new NJS_VECTOR{ 0, 0, 0 };
	}
	registerPosition(levelOptions->startPosition, levelID, true);
	registerPosition(levelOptions->endPosition, levelID, false);
}

void LevelImporter::registerPosition(
		NJS_VECTOR* position,
		LevelIDs levelID,
		bool isStart) {
	StartPosition startPosition = {
		(short)levelID,
		0, // Single player rotation
		0, // Multiplayer, P1 rotation
		0, // Multiplayer, P2 rotation
		*position, // Single player
		*position, // Multiplayer, P1
		*position  // Multiplayer, P2
	};
	if (isStart) {
		helperFunctions.RegisterStartPosition(CurrentCharacter, startPosition);
	}
	else {
		helperFunctions.RegisterEndPosition(CurrentCharacter, startPosition);
	}
}

void LevelImporter::resetActiveLevel() {
	activeLevel = nullptr;
	for (Level* level : levels) {
		if (CurrentLevel == getLevelID(level->landTableName)) {
			activeLevel = level;
		}
	}
}

void LevelImporter::free() {
	for (Level* level : levels) {
		LandTableInfo* landTableInfo = level->landTableInfo;
		if (landTableInfo != nullptr &&
				landTableInfo->getlandtable() != nullptr) {
			delete landTableInfo->getlandtable()->TextureList;
			delete landTableInfo->getlandtable();
			delete landTableInfo;
		}
		delete level->textureNames;
		delete level;
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
