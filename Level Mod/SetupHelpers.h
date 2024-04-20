#pragma once

#include "pch.h"
#include "IniReader.h"
#include "LevelImporter.h"

class SetupHelpers {
public:
	SetupHelpers(const char* path, const HelperFunctions& helperFunctions);

	/*
	  Checks the internet for an update to My Level Mod and saves a 
      notification file in the mod folder if an update is detected.
    */
	void checkForUpdate();

	/*
	  Checks for and fixes incorrect file placements in the mod folder. If any
	  fixes occur, a restart will be required.
	*/
	void fixFileStructure(LevelIDs levelID);

	/*
      Sets up My Level Mod by reading from level_options.ini and setting up level
	  imports.
	*/
	void init();

	/* Frees up memory used by My Level Mod. */
	void free();

	/* Runs LevelImporter's onFrame function. */
	void onFrame();

	/* Runs every time a level is loaded. Used to load splines. */
	void onLevelLoad();

private:
	std::vector<ImportRequest*> requests;
	IniReader* iniReader;
	LevelImporter* levelImporter;
	const char* modFolderPath;
};

/** Helper function for checkForUpdate(). */
int writer(char* data, size_t size, size_t nmemb, std::string* buffer);
