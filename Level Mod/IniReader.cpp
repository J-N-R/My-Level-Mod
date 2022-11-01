/**
 * IniReader.cpp v4
 *
 * Description:
 *   Script for reading the content of ini files for My Level Mod.
 *   
 *   Ini file reader for My Level Mod and SA2 modding purposes.
 *   Can read from a 'level_options.ini' file, and read from
 *   spline files and return official LoopHead (spline) objects.
 */

#include "pch.h"
#include "IniReader.h"
#include <fstream>
#include <string>
#include <sstream>
#include <cstdio>
#include <filesystem>

ObjectFunc(LoopController, 0x497B50);
ObjectFunc(RailController, 0x4980C0);
ObjectFunc(CameraSplineController, nullptr);
ObjectFunc(AltCameraSplineController, nullptr);

IniReader::IniReader(const char* path,
		const HelperFunctions& helperFunctions)
			: helperFunctions(helperFunctions) {
	this->optionsPath = _strdup((std::string(path) +
			"\\level_options.ini").c_str());
	this->gdPCPath = _strdup((std::string(path) + "\\gd_PC").c_str());
	this->levelID = -1;
	this->simpleDeathPlane = 0;
	this->hasSimpleDeathPlane = false;
	this->replaceChaoGarden = -1;
}

// Read and configure options from 'level_options.ini'.
void IniReader::loadIniOptions() {
	std::ifstream iniFile;
	iniFile.open(optionsPath, std::ios::in);

	if (iniFile.is_open()) {
		printDebug("Reading from level_options.");

		std::string token;
		while (std::getline(iniFile, token, '=')) {
			// Simple import ID that will automatically build a query.
			if (token == "Level_Import_ID") {
				std::getline(iniFile, token);

				try {
					levelID = std::stoi(token);
					printDebug("Level ID set to " + token + ".");
					printDebug("Will port level over objLandTable00" +
						token + ".");
				}
				catch (const std::exception& e) {
					levelID = 13;
					printDebug("(Warning) Error reading level ID. Setting to "
						"City Escape.");
					printDebug(e.what());
				}
			}

			// Spawn and Victory locations for the simple import level.
			else if (levelID != -1 && token == "Spawn_Coordinates" ||
					token == "Victory_Coordinates") {
				boolean isStart = token == "Spawn_Coordinates" ? true : false;

				std::getline(iniFile, token);
				std::istringstream ss(token);

				// Get x, y, and z coordinates.
				float coords[3]{};
				try {
					std::string temp;
					for (int i = 0; i < 3; i++) {
						std::getline(ss, temp, ',');
						coords[i] = std::stof(temp);
					}
				}
				// Print debug information in case of error.
				catch (const std::exception& e) {
					printDebug("(Warning) Error reading coordinates. "
						"Defaulting to 0, 0, 0.");
					printDebug(e.what());
				}

				NJS_VECTOR coordinates{coords[0], coords[1], coords[2]};
				StartPosition startPos = {
							levelID,
							0,
							0,
							0,
							coordinates,
							coordinates,
							coordinates
				};
				if (isStart) {
					printDebug("Level Start coordinates set to: " + token);
					helperFunctions.RegisterStartPosition(Characters_Sonic,
						startPos);
				}
				else {
					printDebug("Level End coordinates set to: " + token);
					helperFunctions.RegisterEndPosition(Characters_Sonic,
						startPos);
				}
			}

			// A Simple Death Plane to kill sonic without death zones, for the
			// simple import level.
			else if (token == "Simple_Death_Plane") {
				std::getline(iniFile, token);

				try {
					simpleDeathPlane = std::stof(token);
					hasSimpleDeathPlane = true;
				}
				catch (const std::exception& e) {}

				if (hasSimpleDeathPlane) {
					printDebug("Simple Death Plane set to ON. Player will now "
						"die under y=" + std::to_string(simpleDeathPlane) + ".");
				}
				else {
					printDebug("Simple Death Plane set to OFF.");
				}
			}

			else if (token == "Replace_Chao_Garden_ID") {
				std::getline(iniFile, token);

				try {
					replaceChaoGarden = std::stoi(token);
					printDebug("Chao Garden ID detected. Will port level over" +
						getChaoGarden() + ".");
				}
				catch (const std::exception& e) {
					replaceChaoGarden = -1;
					printDebug("(Warning) Unknown Chao Garden ID detected."
						"Chao Garden will not be replaced.");
				}
			}

			else if (token == "Import_Level") {
				std::getline(iniFile, token);
				std::istringstream ss(token);

				// Build importLevel() query, store in local variable.
				std::vector<std::string> parameters;
				std::string temp;
				while (std::getline(ss, temp, ',')) {
					parameters.push_back(temp);
				}
				if (parameters.size() != 0) {
					importLevelQueries.push_back(
						std::vector<std::string>(parameters.begin(),
							parameters.end()));
				}
			}
		}
		printDebug("Done reading from level_options.");
		iniFile.close();
	}
	else {
		printDebug("(Warning) Error reading from level options. (is it "
			"missing?)");
	}
}

// Automatically detect and read from spline ini files.
LoopHead** IniReader::loadSplines() {
	std::vector<LoopHead*> splines{};

	// Find ini files in the gd_PC folder, assume they are spline files.
	for (const auto& file : std::filesystem::directory_iterator(gdPCPath)) {
		std::string filePath = file.path().string();

		if (filePath.find(".ini") != std::string::npos) {
			std::ifstream splineFile;
			splineFile.open(filePath, std::ios::in);

			if (splineFile.is_open()) {
				printDebug("Spline file \"" + filePath + "\" found.");

				float totalDistance{};
				std::string code{};
				std::vector<LoopPoint> points{};

				try {
					// Read spline file header (currently only 3 lines).
					std::string token;
					for (int i = 0; i < 2; i++) {
						std::getline(splineFile, token, '=');

						if (token == "TotalDistance") {
							std::getline(splineFile, token);
							totalDistance = std::stof(token);
						}
						else if (token == "Code") {
							std::getline(splineFile, token);
							code = token;
						}
						else {
							printDebug("(Warning) Unexpected error/value in "
								"header: " + token + ".");
							printDebug("(Warning) Deleting spline.");
							return nullptr;
						}
					}

					// Skip rest of empty lines in header.
					while (token.find("[") == std::string::npos) {
						std::getline(splineFile, token);
					}
	
					// Read spline data and create spline objects.
					while (token.find("[") != std::string::npos && !splineFile.eof()) {
						int xRot{}, zRot{};
						float distance{};
						float coords[3]{};

						for (int i = 0; i < 4; i++) {
							std::getline(splineFile, token, '=');

							if (token == "XRotation") {
								std::getline(splineFile, token);
								xRot = std::stoi(token, 0, 16);
							}
							else if (token == "ZRotation") {
								std::getline(splineFile, token);
								zRot = std::stoi(token, 0, 16);
							}
							else if (token == "Distance") {
								std::getline(splineFile, token);
								distance = std::stof(token);
							}
							else if (token == "Position") {
								std::getline(splineFile, token);

								// Remove spaces from line.
								std::string::iterator end_pos =
									std::remove(token.begin(), token.end(), ' ');
								token.erase(end_pos, token.end());

								std::istringstream ss(token);
								try {
									std::string temp{};
									for (int i = 0; i < 3; i++) {
										std::getline(ss, temp, ',');
										coords[i] = std::stof(temp);
									}
								}
								// Print debug information in case of error.
								catch (const std::exception& e) {
									printDebug("(Warning) Unexpected "
										"error/value in spline point: " +
										token + ".");
									printDebug("(Warning) Deleting spline.");
									printDebug(e.what());
									return nullptr;
								}
							}
							else if (token == "\n") {
								break;
							}
							else if (token != "\n") {
								printDebug("(Warning) Unexpected error/value "
									"in spline point: " + token + ".");
								printDebug("(Warning) Deleting spline.");
								return nullptr;
							}
						}

						// Create new point from read data.
						LoopPoint point {
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

						// Skip rest of lines to find new point.
						while (token.find("[") == std::string::npos
								&& !splineFile.eof()) {
							std::getline(splineFile, token);
						}
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
					splines.push_back(spline);
				}
				// Print debugs if there was an error loading the spline.
				catch (const std::exception& e) {
					printDebug("(ERROR) Invalid value found while reading "
						"spline file.");
					printDebug(e.what());
				}
			}
			else {
				printDebug("(Warning) Error reading from spline file: " +
					filePath + ".");
			}
			splineFile.close();
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
	else {
		printDebug("No splines found.");
	}

	return nullptr;
}

// Convenience method used to grab level id (right now used for debug printing).
std::string IniReader::getLevelID() {
	return std::to_string(levelID);
}

std::string IniReader::getChaoGarden() {
	switch (replaceChaoGarden) {
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

// Notes:
// For start position and setting rotation, 0x4000 == 90 degrees.

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
