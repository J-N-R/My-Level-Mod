/**
 * MyLevelMod.cpp
 * 
 * Description:
 *    The main execution script for My Level Mod, a simple way of importing
 *    levels into Sonic Adventure 2 without having to write code. But for you
 *    creative few, this is where you can add in your own code! Go crazy! Just
 *    try not to break anything :)
 * 
 *    X-Hax discord for code questions: https://discord.gg/gqJCF47
 */

#include "pch.h"
#include "IniReader.h"
#include "LevelImporter.h"
#include "SetupHelpers.h"

LevelImporter* myLevelMod;

extern "C" {
	// Runs a single time once the game starts up. Required for My Level Mod.
	__declspec(dllexport) void Init(
			const char* modFolderPath,
			const HelperFunctions& helperFunctions) {
		myLevelMod = new LevelImporter(modFolderPath, helperFunctions);
		myLevelModInit(modFolderPath, myLevelMod);
	}
	
	// Runs for every frame while the game is on. Required for My Level Mod.
    __declspec(dllexport) void __cdecl OnFrame() {
		myLevelMod->onFrame();
	}

	// Runs when the game closes. Required for My Level Mod.
	__declspec(dllexport) void __cdecl OnExit() {
		myLevelMod->free();
	}

	__declspec(dllexport) ModInfo SA2ModInfo = { ModLoaderVer };
}

// Required.
// A Function 'Hook,' that automatically runs code whenever a the game has
// loaded a level.
void onLevelLoad();
FunctionHook<void> loadLevelHook(InitCurrentLevelAndScreenCount, onLevelLoad);
void onLevelLoad() {
	myLevelMod->onLevelLoad();
	loadLevelHook.Original();
}



// Have to put this for my job.
// Don't worry! You're free to edit and do whatever you like :)
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
