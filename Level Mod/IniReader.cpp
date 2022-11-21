/*
 * IniReader.cpp v4.1
 *
 * Description:
 *   Script for reading the content of ini files for My Level Mod.
 *   
 *   Ini file reader for My Level Mod and SA2 modding purposes. Can read from a
 *   'level_options.ini' file, and read from spline files and return official
 *   LoopHead (spline) objects.
 */

#include "pch.h"
#include "IniReader.h"
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <algorithm>

ObjectFunc(LoopController, 0x497B50);
ObjectFunc(RailController, 0x4980C0);

IniReader::IniReader(const char* path,
		const HelperFunctions& helperFunctions)
			: helperFunctions(helperFunctions) {
	this->optionsPath = _strdup((std::string(path) +
			"\\level_options.ini").c_str());
	this->gdPCPath = _strdup((std::string(path) + "\\gd_PC").c_str());
	this->levelID = -1;
	this->simpleDeathPlane = 0;
	this->hasSimpleDeathPlane = false;
}

// Read and configure options from 'level_options.ini.' For any devs out there,
// this is where you can add/create new options.
void IniReader::loadIniOptions() {
	std::ifstream iniFile;
	iniFile.open(optionsPath, std::ios::in);

	// Read through level_options.ini line by line, looking for an equals sign.
	// 'token' will then be the option name. Using getline again grabs the
	// value.
	if (!iniFile.is_open()) {
		printDebug("(Warning) Error reading from options file. (is it"
			"missing?)");
	}

	printDebug("Reading from level_options.");

	std::string line;
	while (line.find("Explanations:") == std::string::npos) {
		std::getline(iniFile, line);
		if (line.find('=') != std::string::npos) {
			std::string key = line.substr(0, line.find('='));
			std::string value = line.substr(line.find('=') + 1);

			// Create a simple import level query using the given level ID.
			if (key == "Level_Import_ID") {
				try {
					levelID = std::stoi(value);
					printDebug("Level ID set to " + value + ".");

					if (levelID >= 67) {
						printDebug("Chao Garden level ID detected.");
						printDebug("Will port level over " + getChaoGarden() +
							".");
					}
					else {
						printDebug("Will port level over objLandTable00" +
							value + ".");
					}
				}
				catch (const std::exception& e) {
					printDebug("(Warning) Error reading level import ID.");
					printDebug(e.what());
				}
			}

			// Spawn and Victory locations for the simple import level.
			else if (levelID != -1 && key == "Spawn_Coordinates" ||
				key == "Victory_Coordinates") {
				boolean isStart = key == "Spawn_Coordinates" ? true : false;
				const auto ignoreThis = std::remove(value.begin(), value.end(), ' ');
				std::istringstream ss(value);

				// Get x, y, and z coordinates.
				float coords[3]{};
				try {
					std::string token;
					for (int i = 0; i < 3; i++) {
						std::getline(ss, token, ',');
						coords[i] = std::stof(token);
					}
				}
				// Print debug information in case of error.
				catch (const std::exception& e) {
					printDebug("(Warning) Error reading coordinates." 
						"Defaulting to 0, 0, 0.");
					printDebug(e.what());
				}

				NJS_VECTOR coordinates{ coords[0], coords[1], coords[2] };
				StartPosition startPos = {
							(short)levelID,
							0,
							0,
							0,
							coordinates,
							coordinates,
							coordinates
				};
				if (isStart) {
					printDebug("Level Start coordinates set to: " + value);
					helperFunctions.RegisterStartPosition(Characters_Sonic,
						startPos);
				}
				else {
					printDebug("Level End coordinates set to: " + value);
					helperFunctions.RegisterEndPosition(Characters_Sonic,
						startPos);
				}
			}

			// A Simple Death Plane to kill sonic without death zones, for the
			// simple import level.
			else if (key == "Simple_Death_Plane") {
				try {
					simpleDeathPlane = std::stof(value);
					hasSimpleDeathPlane = true;
				}
				// Catch will run if user inputted "OFF" or something else.
				catch (const std::exception& e) {
					const auto ignoreThis = e;
				}
				if (hasSimpleDeathPlane) {
					printDebug("Simple Death Plane set to ON. Player will now die"
						"under y=" + std::to_string(simpleDeathPlane) + ".");
				}
				else {
					printDebug("Simple Death Plane set to OFF.");
				}
			}

			// User inputable importLevel() queries that can import a level
			// over a specific landtable, with specific files. Can be used
			// multiple times  to import multiple levels. Does not support the
			// other features, such as Spawn location or death plane.
			else if (key == "Import_Level") {
				const auto ignoreThis = std::remove(value.begin(), value.end(), ' ');
				std::istringstream ss(value);

				printDebug("Import Level query found!");
				printDebug("Building query with these parameters: " + value);

				// Build importLevel() query, store in local variable.
				std::vector<std::string> parameters;
				std::string token;
				while (std::getline(ss, token, ',')) {
					parameters.push_back(token);
				}
				if (parameters.size() != 0) {
					importLevelQueries.push_back(
						std::vector<std::string>(parameters.begin(),
							parameters.end()));
				}
			}
		}
	}
	printDebug("Done reading from level_options.");
	iniFile.close();
}

// Automatically reads and generates spline objects from spline ini files.
// Checks both gd_PC folder and paths folder for spline files.
LoopHead** IniReader::readSplines() {
	std::vector<LoopHead*> splines{};
	std::string pathToPathsFolder = std::string(gdPCPath) + "\\Paths";

	// Find ini files in the gd_PC folder, assume they are spline files.
	for (const auto& file : std::filesystem::directory_iterator(gdPCPath)) {
		std::string filePath = file.path().string();

		if (filePath.find(".ini") != std::string::npos) {
			printDebug("Spline file \"" + filePath + "\" found.");
			LoopHead* spline = readSpline(filePath);
			if (spline != nullptr) {
				splines.push_back(spline);
			}
		}
	}

	// Find ini files in the paths folder, assume they are spline files. 
	if (std::filesystem::exists(pathToPathsFolder)) {
		for (const auto& file :
			std::filesystem::directory_iterator(pathToPathsFolder)) {
			std::string filePath = file.path().string();

			if (filePath.find(".ini") != std::string::npos) {
				printDebug("Spline file \"" + filePath + "\" found.");
				LoopHead* spline = readSpline(filePath);
				if (spline != nullptr) {
					splines.push_back(spline);
				}
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

	printDebug("No splines found.");
	return nullptr;
}

/**
 * Reads a given spline ini file and returns a single spline object. Returns
 * nullptr if something goes wrong.
 *
 * @param [filePath] - The full file path to your ini file.
 */
LoopHead* IniReader::readSpline(std::string filePath) {
	std::ifstream splineFile;
	splineFile.open(filePath, std::ios::in);

	if (!splineFile.is_open()) {
		printDebug("(Warning) Error opening spline file.");
		return nullptr;
	}

	float totalDistance{};
	std::string code{};
	std::vector<LoopPoint> points{};

	try {
		// Read spline file header.
		std::string line;
		while (line.find("[") == std::string::npos) {
			std::getline(splineFile, line);
			if (line.find('=') != std::string::npos) {
				std::string key = line.substr(0, line.find('='));
				std::string value = line.substr(line.find('=') + 1);

				if (key == "TotalDistance") {
					totalDistance = std::stof(value);
				}
				else if (key == "Code") {
					code = value;
				}
			}
		}

		// Read spline data and create spline objects.
		while (line.find("[") != std::string::npos &&
			!splineFile.eof()) {
			short xRot{}, zRot{};
			float distance{};
			float coords[3]{};

			std::getline(splineFile, line);
			while (line.find("[") == std::string::npos &&
				!splineFile.eof()) {
				if (line.find('=') != std::string::npos) {
					std::string key = line.substr(0, line.find('='));
					std::string value = line.substr(line.find('=') + 1);

					if (key == "XRotation") {
						xRot = (short)std::stoi(value, 0, 16);
					}
					else if (key == "ZRotation") {
						zRot = (short)std::stoi(value, 0, 16);
					}
					else if (key == "Distance") {
						distance = std::stof(value);
					}
					else if (key == "Position") {
						// Remove spaces from line.
						std::string::iterator end_pos =
							std::remove(value.begin(), value.end(),
								' ');
						value.erase(end_pos, value.end());

						std::istringstream ss(value);
						try {
							std::string token{};
							for (int i = 0; i < 3; i++) {
								std::getline(ss, token, ',');
								coords[i] = std::stof(token);
							}
						}
						// Print debug information in case of error.
						catch (const std::exception& e) {
							printDebug("(Warning) Unexpected "
								"error/value in spline point: " +
								value + ".");
							printDebug("(Warning) Deleting spline.");
							printDebug(e.what());
							return nullptr;
						}
					}
				}
				std::getline(splineFile, line);
			}

			// Create new point from read data.
			LoopPoint point{
				xRot,
				zRot,
				distance,
				{
					coords[0],
					coords[1],
					coords[2]
				}
			};
			points.push_back(point);
		}

		LoopHead* spline = new LoopHead;
		spline->anonymous_0 = (int16_t)1;
		spline->Count = (int16_t)points.size();
		spline->TotalDistance = totalDistance;
		spline->Points = new LoopPoint[points.size()];

		// If spline is for rails, load spline as rail data.
		if (code == "4980C0") {
			spline->Object = (ObjectFuncPtr)RailController;
		}
		// If spline is for loops, load spline as loop data.
		else if (code == "497B50") {
			spline->Object = (ObjectFuncPtr)LoopController;
		}
		else {
			printDebug("(Warning) Unknown spline code detected. "
				"Deleting spline.");
			return nullptr;
		}

		std::copy(points.begin(), points.end(), spline->Points);

		splineFile.close();
		return spline;
	}
	// Print debugs if there was an error loading the spline.
	catch (const std::exception& e) {
		printDebug("(ERROR) Invalid value found while reading "
			"spline file.");
		printDebug(e.what());
	}

	splineFile.close();
	return nullptr;
}

// Convenience method used to grab level id (right now used for debug printing).
std::string IniReader::getLevelID() {
	return std::to_string(levelID);
}

std::string IniReader::getChaoGarden() {
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

// Debug message helper.
void IniReader::printDebug(std::string message) {
	PrintDebug(("[My Level Mod] " + message).c_str());
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
