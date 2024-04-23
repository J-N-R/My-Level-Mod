#pragma once

#include "pch.h"
#include "IniReader.h"
#include "LevelImporter.h"

/*
  Checks the internet for an update to My Level Mod and saves a 
  notification file in the mod folder if an update is detected.
*/
void checkForUpdate(const char* modFolderPath);

/*
  Checks for and fixes incorrect file placements in the mod folder. If any
  fixes occur, a restart will be required.
*/
void fixFileStructure(const char* modFolderPath, LevelIDs levelID);

/*
  Sets up My Level Mod by reading from level_options.ini and setting up level
  imports.
*/
void myLevelModInit(const char* modFolderPath, LevelImporter* levelImporter);

/** Helper function for checkForUpdate(). */
int writer(char* data, size_t size, size_t nmemb, std::string* buffer);
