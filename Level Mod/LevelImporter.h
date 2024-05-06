#pragma once
#include "pch.h"
#include "IniReader.h"
#include <string>
#include <vector>
#include <curl/curl.h>

/** Utility to import levels into Sonic Adventure 2. */
class LevelImporter {
	public:
		LevelImporter(const char* path, const HelperFunctions& helperFunctions);

		/** 
		 * Imports a custom level over an existing level. Automatically detects
		 * a pak and sa2blvl file to use in your mod's folder. Recommended for
		 * mods that only import one level.
		 * 
		 * @param [landTableName] - The name of the land table to replace.
		 */
		void importLevel(std::string landTableName);

		/**
		 * Imports a custom level over an existing level. Automatically detects
		 * a pak and sa2blvl file to use in your mod's folder. Recommended for
		 * mods that only import one level.
		 *
		 * @param [landTableName] - The name of the land table to replace.
		 * @param [levelOptions] - The features enabled for this level.
		 */
		void importLevel(
			std::string landTableName,
			LevelOptions levelOptions
		);

		/**
		 * Imports a custom level over an existing level.
		 * 
		 * @param [landTableName] - The name of the land table to replace.
		 * @param [levelFileName] - The name of the sa2blvl file to use.
		 * @param [pakFileName] - The name of the pak file to use.
		 */
		void importLevel(
			std::string landTableName,
			std::string levelFileName,
			std::string pakFileName
		);

		/**
		 * Imports a custom level over an existing level.
		 *
		 * @param [landTableName] - The name of the land table to replace.
		 * @param [levelFileName] - The name of the sa2blvl file to use.
		 * @param [pakFileName] - The name of the pak file to use.
		 * @param [levelOptions] - The features enabled for this level.
		 */
		void importLevel(
			std::string landTableName,
			std::string levelFileName,
			std::string pakFileName,
			LevelOptions levelOptions
		);

		/**
		 * Imports a custom level over an existing level. Automatically detects
		 * a pak and sa2blvl file to use in your mod's folder. Recommended for
		 * mods that only import one level.
		 *
		 * @param [levelID] - The ID of the level to replace.
		 */
		void importLevel(LevelIDs levelID);

		/**
		 * Imports a custom level over an existing level. Automatically detects
		 * a pak and sa2blvl file to use in your mod's folder. Recommended for
		 * mods that only import one level.
		 *
		 * @param [levelID] - The ID of the level to replace.
		 * @param [levelOptions] - The features enabled for this level.
		 */
		void importLevel(
			LevelIDs levelID,
			LevelOptions levelOptions
		);

		/**
		 * Imports a custom level over an existing level.
		 *
		 * @param [levelID] - The ID of the level to replace.
		 * @param [levelFileName] - The name of the sa2blvl file to use.
		 * @param [pakFileName] - The name of the pak file to use.
		 */
		void importLevel(
			LevelIDs levelID,
			std::string levelFileName,
			std::string pakFileName
		);

		/**
		 * Imports a custom level over an existing level.
		 *
		 * @param [levelID] - The ID of the level to replace.
		 * @param [levelFileName] - The name of the sa2blvl file to use.
		 * @param [pakFileName] - The name of the pak file to use.
		 */
		void importLevel(
			LevelIDs levelID,
			std::string levelFileName,
			std::string pakFileName,
			LevelOptions levelOptions
		);

		/**
		 * Imports multiple custom levels into the game. Convenience method for
		 * My Level Mod, which knows all level imports at startup.
		 */
		void importLevels(std::vector<ImportRequest> requests);

		/* Runs on every frame, used to enable My Level Mod features. */
		void onFrame();

		/*
		  Loads a custom level and its resources if the game attempts to load a
		  level designated for replacement.
		*/
		void onLevelLoad();

		/*
		  Frees the memory allocated by loading the previous custom level. A
		  no-op if the last level was not a custom level.
		*/
		void freeLevelResources();

		/* Frees the memory allocated by LevelImporter. */
		void free();

		/*
		  A list of pointers to the currently loaded custom land tables. 
		  Typically containing one element, the only scenario this list has
		  multiple elements is if multiple chao gardens are being replaced.
		*/
		std::vector<LandTableInfo*> activeLandTables;

		std::vector<ImportRequest> importRequests;
		LevelIDs getLevelID(std::string landTableName);
		std::string getLandTableName(LevelIDs levelID);

    // TODO: Implement replaceLevelInit as an alternative import method.
	private:
		std::string modFolderPath;
		std::string gdPCPath;
		std::string PRSPath;
		IniReader* iniReader;
		std::vector<LoopHead**> activeSplines;
		LevelOptions activeOptions;
		const HelperFunctions& helperFunctions;
		LandTable* generateLandTable(std::string levelFileName,
			std::string pakFileName,
			std::string landTableName
		);
		void setLevelOptions(LevelOptions options);
		/*
		  Imports a level into Sonic Adventure 2 by replacing an existing
		  level's land table. Warning: This method keeps the LevelHeader.Init
		  method in-tact, causing original level assets to load. This may cause
		  missing texture crashes depending on your level.
		*/
		void replaceLandTable(LandTable* landTableInfo, std::string landTableName);
		void registerPosition(NJS_VECTOR position, LevelIDs levelID, bool isStart);
		static std::string detectFile(std::string path, std::string fileExtension);
};
