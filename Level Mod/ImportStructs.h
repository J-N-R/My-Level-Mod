#include "pch.h"
#define DISABLED_PLANE -1.666f

/* Optional features for levels. */
struct LevelOptions {
	NJS_VECTOR* startPosition = nullptr;
	NJS_VECTOR* endPosition = nullptr;
	LoopHead** splines = nullptr;
	// My Level Mod feature that kills Sonic below a given y coordinate.
	// Defaults to a disabled number, denoted by DISABLED_PLANE.
	float simpleDeathPlane = DISABLED_PLANE;
};

/* Important data used to import levels. */
struct Level {
	std::string landTableName;
	LandTableInfo* landTableInfo;
	NJS_TEXNAME* textureNames;
	NJS_TEXLIST textureList;
	LevelOptions* levelOptions;
};

/*
  The data necessary to import a level. For My Level Mod, this data is stored
  in level_options.ini.
*/
struct ImportRequest {
	// Must have either levelID or landTableName defined. Defaults to -1.
	LevelIDs levelID = (LevelIDs)-1;
	// Must have either levelID or landTableName defined. Defaults to empty
	// string.
	std::string landTableName = std::string();
	// The name of the sa2blvl file to use. If not present, automatically
	// detect one in your mod's gd_PC folder.
	std::string levelFileName = std::string();
	// The name of the sa2blvl file to use. If not present, automatically
	// detect one in your mod's gd_PC folder.
	std::string pakFileName = std::string();
	// The names of the ini files to load splines from. Optional.
	std::vector<std::string> splineFileNames;
	// The My Level Mod features to enable for this level.
	LevelOptions* levelOptions = nullptr;
};