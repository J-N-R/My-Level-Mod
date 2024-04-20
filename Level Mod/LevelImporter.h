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
		 * @return Pointer to the level's land table.
		 */
		LandTableInfo* importLevel(std::string landTableName);

		/**
		 * Imports a custom level over an existing level. Automatically detects
		 * a pak and sa2blvl file to use in your mod's folder. Recommended for
		 * mods that only import one level.
		 *
		 * @param [landTableName] - The name of the land table to replace.
		 * @param [levelOptions] - The features enabled for this level.
		 * @return Pointer to the level's land table.
		 */
		LandTableInfo* importLevel(
			std::string landTableName,
			LevelOptions* levelOptions
		);

		/**
		 * Imports a custom level over an existing level.
		 * 
		 * @param [landTableName] - The name of the land table to replace.
		 * @param [levelFileName] - The name of the sa2blvl file to use.
		 * @param [pakFileName] - The name of the pak file to use.
		 * @return Pointer to the level's land table.
		 */
		LandTableInfo* importLevel(
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
		 * @return Pointer to the level's land table.
		 */
		LandTableInfo* importLevel(
			std::string landTableName,
			std::string levelFileName,
			std::string pakFileName,
			LevelOptions* levelOptions
		);

		/**
		 * Imports a custom level over an existing level. Automatically detects
		 * a pak and sa2blvl file to use in your mod's folder. Recommended for
		 * mods that only import one level.
		 *
		 * @param [levelID] - The ID of the level to replace.
		 * @return Pointer to the level's land table.
		 */
		LandTableInfo* importLevel(LevelIDs levelID);

		/**
		 * Imports a custom level over an existing level. Automatically detects
		 * a pak and sa2blvl file to use in your mod's folder. Recommended for
		 * mods that only import one level.
		 *
		 * @param [levelID] - The ID of the level to replace.
		 * @param [levelOptions] - The features enabled for this level.
		 * @return Pointer to the level's land table.
		 */
		LandTableInfo* importLevel(
			LevelIDs levelID,
			LevelOptions* levelOptions
		);

		/**
		 * Imports a custom level over an existing level.
		 *
		 * @param [levelID] - The ID of the level to replace.
		 * @param [levelFileName] - The name of the sa2blvl file to use.
		 * @param [pakFileName] - The name of the pak file to use.
		 * @return Pointer to the level's land table.
		 */
		LandTableInfo* importLevel(
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
		 * @return Pointer to the level's land table.
		 */
		LandTableInfo* importLevel(
			LevelIDs levelID,
			std::string levelFileName,
			std::string pakFileName,
			LevelOptions* levelOptions
		);

		/* Runs on every frame, used to enable My Level Mod features. */
		void onFrame();
		/*
		  Enables features that require running when a level loads. Currently
		  used to enable loading splines for levels imported by level id.
		*/
		void onLevelHook();
		/* Frees the memory allocated by LevelImporter. */
		void free();
		Level* activeLevel;
		std::vector<Level*> levels;
		LevelIDs getLevelID(std::string landTableName);
		std::string getLandTableName(LevelIDs levelID);
		/*
		  Forces a refresh on active level. Only use this if you need to access
		  the active level before it has been initialized (Such as when writing
		  custom level hook logic).
		*/
		void resetActiveLevel();

	private:
		std::string modFolderPath;
		std::string gdPCPath;
		std::string PRSPath;
		const HelperFunctions& helperFunctions;
		void registerPosition(NJS_VECTOR* position, LevelIDs levelID, bool isStart);
		/* Start positions must be registered before a level is loaded. */
		void registerStartPositions(LevelOptions* levelOptions, std::string landTableName);
		static std::string detectFile(std::string path, std::string fileExtension);
};
