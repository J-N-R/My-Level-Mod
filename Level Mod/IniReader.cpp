/*
 * IniReader.cpp
 *
 * Description:
 *   A class dedicated to parsing ini files for My Level Mod. This class
 *   parses over two types of ini files:
 * 
 *   The level_options.ini file:
 *      This file determines which levels to import and what level features
 *      to enable.
 *
 *	 Spline files:
 *		Files that define paths for loops, rails, and enemy paths.
 */

#include "pch.h"
#include "IniFile.hpp"
#include "IniReader.h"
#include "SetupHelpers.h"
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <algorithm>

IniReader::IniReader(const char* modFolderPath) {
	this->optionsPath = _strdup((std::string(modFolderPath) +
			"\\level_options.ini").c_str());
	this->gdPCPath = _strdup((std::string(modFolderPath) + "\\gd_PC").c_str());
}

/**
 * Parses through the level_options.ini file to find out which levels to import
 * and which level features should be enabled. It is important to note that
 * level features only work for levels imported by level id.
 */
std::vector<ImportRequest> IniReader::readLevelOptions() {
	printDebug("");
	printDebug("Reading options from \"level_options.ini.\"");
	IniFile* iniFile = new IniFile(optionsPath);

	auto printTabbed = [](std::string message) {
		printDebug("  " + message);
	};

	auto printWarning = [](std::string message) {
		showWarning("Warning: " + message);
	};

	bool hadFailedRequest = false;
	std::vector<ImportRequest> requests;
	for (auto it = iniFile->begin(); it != iniFile->end(); it++) {
		if (it->first.empty()) {
			continue;
		}
		printDebug("");
		printDebug("Custom level [" + it->first + "] found:");
		IniGroup* iniGroup = it->second;
		ImportRequest request;
		LevelOptions levelOptions;
		if (iniGroup->hasKey("level_id")) {
			try {
				printTabbed("level_id=" + iniGroup->getString("level_id"));
				request.levelID = 
					(LevelIDs)iniGroup->getInt("level_id", LevelIDs_Invalid);
			} catch (...) {
				printWarning("Invalid level_id given: \"" +
					iniGroup->getString("level_id") + "\"");
			}
		}
		if (iniGroup->hasKey("land_table_name")) {
			request.landTableName = iniGroup->getString("land_table_name");
			printTabbed("land_table_name=" + request.landTableName);
		}
		if (iniGroup->hasKey("level_file_name")) {
			request.levelFileName = iniGroup->getString("level_file_name");
			printTabbed("level_file_name=" + request.levelFileName);
		}
		if (iniGroup->hasKey("pak_file_name")) {
			request.pakFileName = iniGroup->getString("pak_file_name");
			printTabbed("pak_file_name=" + request.pakFileName);
		}
		if (iniGroup->hasKey("spline_file_names")) {
			levelOptions.splineFileNames = getTokens(
				iniGroup->getString("spline_file_names")
			);
			printTabbed("spline_file_names=" +
				iniGroup->getString("spline_file_names"));
		}
		if (iniGroup->hasKey("simple_death_plane")) {
			try {
				levelOptions.simpleDeathPlane = 
					iniGroup->getFloat("simple_death_plane", DISABLED_PLANE);
				printTabbed("simple_death_plane=" +
					std::to_string(levelOptions.simpleDeathPlane));
			} catch (...) {
				std::string simpleDeathPlaneStr = iniGroup->getString("simple_death_plane") ;
				std::transform(simpleDeathPlaneStr.begin(), simpleDeathPlaneStr.end(), simpleDeathPlaneStr.begin(), ::toupper);
				if (simpleDeathPlaneStr != "OFF" && simpleDeathPlaneStr != "FALSE") {
					printWarning("Invalid simple_death_plane given: " +
						iniGroup->getString("simple_death_plane"));
				}
			}
		}
		std::string coordinates;
		try {
			coordinates = iniGroup->getString("spawn_coordinates", "0,0,0");
			printTabbed("spawn_coordinates=" + coordinates);
			levelOptions.startPosition = getPosition(coordinates);
		}
		catch (...) {
			printWarning("Invalid spawn coordinates given: \"" + coordinates +
				".\" Using 0, 0, 0 as default.");
		}
		try {
			coordinates = iniGroup->getString("victory_coordinates", "0,0,0");
			printTabbed("victory_coordinates=" + coordinates);
			levelOptions.endPosition = getPosition(coordinates);
		}
		catch (...) {
			printWarning("Invalid victory coordinates given: \"" +
				coordinates + ".\" Using 0, 0, 0 as default.");
		}
		if (request.levelID == LevelIDs_Invalid && request.landTableName.empty()) {
			printDebug("");
			printWarning("This level import does not have a level_id "
				"or land_table_name set. Discarding import, please check your "
				"level_options.ini file if this is a mistake.");
			hadFailedRequest = true;
			continue;
		}
		request.levelOptions = levelOptions;
		requests.push_back(request);
	}
	if (requests.size() == 0 && !hadFailedRequest) {
		printWarning("Could not find options file. Please redownload My Level Mod.");
	}
	printDebug("");
	printDebug("Done reading options.");
	delete iniFile;
	return requests;
}

/**
 * Automatically detect and attempt to read all Spline files. This function
 * checks both the gd_PC folder and a "paths" folder for Spline files.
 */
LoopHead** IniReader::readSplines(std::vector<std::string> splineFileNames) {
	std::vector<std::string> fileNamesCopy;
	copy(
		splineFileNames.begin(),
		splineFileNames.end(),
		std::back_inserter(fileNamesCopy)
	);
	boolean readAllFiles = splineFileNames.empty();
	for (unsigned int i = 0; i < fileNamesCopy.size(); i++) {
		fileNamesCopy[i] = removeFileExtension(fileNamesCopy[i]).append(".ini");
	}
	std::vector<LoopHead*> splines;
	std::string pathToPathsFolder = std::string(gdPCPath) + "\\Paths";

	/* 
	  Reads a spline from a given file if it is present in a given set of file
	  names.
	*/
	auto readSplineFile = [&splines](std::string filePath, std::vector<std::string> fileNames) mutable {
		for (std::string splineFileName : fileNames) {
			if (filePath.find(splineFileName) != std::string::npos) {
				printDebug("Spline file \"" + filePath + "\" found.");
				LoopHead* spline = readSpline(filePath);
				if (spline != nullptr) {
					splines.push_back(spline);
				}
			}
		}
	};

	/* Reads a spline from a given file. Only use if there is one level. */
	auto readAllSplineFiles = [&splines](std::string filePath) mutable {
		if (filePath.find(".ini") != std::string::npos) {
			printDebug("Spline file \"" + filePath + "\" found.");
			LoopHead* spline = readSpline(filePath);
			if (spline != nullptr) {
				splines.push_back(spline);
			}
		}
	};

	// Attempt to find the given spline file names in the mod's gdPC folder.
	for (const auto& file : std::filesystem::directory_iterator(gdPCPath)) {
		if (readAllFiles) {
			readAllSplineFiles(file.path().string());
		} else {
			readSplineFile(file.path().string(), fileNamesCopy);
		}
	}

	// Attempt to find the given spline file names in the mod's Paths folder.
	if (std::filesystem::exists(pathToPathsFolder)) {
		for (const auto& file :
			std::filesystem::directory_iterator(pathToPathsFolder)) {
			if (readAllFiles) {
				readAllSplineFiles(file.path().string());
			} else {
				readSplineFile(file.path().string(), fileNamesCopy);
			}
		}
	}
	if (splines.size() != 0) {
		printDebug(std::to_string(splines.size()) + " rail spline(s) "
			"successfully added.");
		const int size = splines.size() + 1;
		LoopHead** splinesArray = new LoopHead*[size];
		std::copy(splines.begin(), splines.end(), splinesArray);
		splinesArray[size - 1] = nullptr;
		return splinesArray;
	}
	showWarning("Warning: Spline loading was called, but no splines were "
		"successfully added. Double check the file names, skipping spline "
		"read.");
	return nullptr;
}

/**
 * Attempts to read and generate a LoopHead object from a given Spline file.
 * Returns nullptr if something goes wrong.
 *
 * @param [filePath] - The full file path to your ini file.
 * 
 * Based on MainMemory's ProcessPathList function at
 * https://github.com/X-Hax/sa2-mod-loader/blob/master/SA2ModLoader/EXEData.cpp
 */
LoopHead* IniReader::readSpline(std::string filePath) {
	IniFile* splineFile = new IniFile(filePath);
	IniGroup* iniGroup;
	std::vector<LoopPoint> points;
	for (unsigned int i = 0; i < 9999; i++) {
		std::string index = std::to_string(i);
		if (!splineFile->hasGroup(index)) {
			break;
		}
		iniGroup = splineFile->getGroup(index);
		points.push_back({
			(int16_t)iniGroup->getIntRadix("XRotation", 16),
			(int16_t)iniGroup->getIntRadix("ZRotation", 16),
			iniGroup->getFloat("Distance"),
			getPosition(iniGroup->getString("Position", "0,0,0")),
		});
	}
	iniGroup = splineFile->getGroup("");
	LoopHead* spline = new LoopHead;
	// anonymous_0 must default to 1 in SA2 to work.
	spline->anonymous_0 = (int16_t)iniGroup->getInt("Unknown", 1);
	spline->Count = (int16_t)points.size();
	spline->TotalDistance = iniGroup->getFloat("TotalDistance");
	spline->Points = new LoopPoint[spline->Count];
	spline->Object = (ObjectFuncPtr)iniGroup->getIntRadix("Code", 16);
	if (!iniGroup->hasKey("Code")) {
		showWarning("Warning: The spline found at " + filePath + " is missing "
			"the \"Code\" field. Did you forget to add it? Throwing away "
			"spline.");
		delete spline;
		return nullptr;
	}
	std::copy(points.begin(), points.end(), spline->Points);
	delete splineFile;
	return spline;
}

NJS_VECTOR IniReader::getPosition(std::string position) {
	std::vector<std::string> tokens = getTokens(position);
	float coords[3]{};
	for (int i = 0; i < 3; i++) {
		coords[i] = std::stof(tokens[i]);
	}
	return NJS_VECTOR{ coords[0], coords[1], coords[2] };
}

std::vector<std::string> IniReader::getTokens(std::string value) {
	std::string valueCopy = value; // C++ 11 forces deep copy.
	std::string::iterator end_pos = std::remove(
		valueCopy.begin(),
		valueCopy.end(),
		' ');
	valueCopy.erase(end_pos, valueCopy.end());
	std::vector<std::string> tokens;
	if (valueCopy.empty()) {
		return tokens;
	}
	std::stringstream ss(valueCopy);
	std::string token;
	while (!ss.eof()) {
		std::getline(ss, token, ',');
		tokens.push_back(token);
	}
	return tokens;
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
