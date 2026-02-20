#include "pch.h"
#include <vector>
#define DISABLED_PLANE -1.666f

/* Optional features for levels. */
struct LevelOptions {
	NJS_VECTOR startPosition = { 0, 0, 0 };
	NJS_VECTOR endPosition = { 0, 0, 0 };
	// My Level Mod feature that kills Sonic below a given y coordinate.
	// Defaults to a disabled number, denoted by DISABLED_PLANE.
	float simpleDeathPlane = DISABLED_PLANE;
	// The names of the ini files to load splines from.
	std::vector<std::string> splineFileNames;
};

/*
  The data necessary to import a level. For My Level Mod, this data is stored
  in level_options.ini. All resources are dynamically loaded on level init.
*/
struct ImportRequest {
	// Must have either levelID or landTableName defined. Defaults to -1.
	LevelIDs levelID = LevelIDs_Invalid;
	// Must have either levelID or landTableName defined. Defaults to empty
	// string.
	std::string landTableName = std::string();
	// The name of the sa2blvl file to use. If not present, automatically
	// detect one in your mod's gd_PC folder.
	std::string levelFileName = std::string();
	// The name of the sa2blvl file to use. If not present, automatically
	// detect one in your mod's gd_PC folder.
	std::string pakFileName = std::string();
	// The My Level Mod features to enable for this level. Optional.
	LevelOptions levelOptions;
};

/*** Shared Functions ***/

/* Returns a copy of the given string without a file extension. */
std::string removeFileExtension(std::string fileName);

/*
  Saves debug information in your mod loader's debug file. Enable "FILE" in the
  mod loader's debug menu to see messages.
*/
void printDebug(std::string message);

/*
  Displays a warning using a windows dialog, with required confirmation from the
  user.
*/
void showWarning(std::string message);
