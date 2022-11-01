/**
 * MyLevelMod.cpp
 * 
 * Description:
 *    Main execution script for My Level Mod, a simple way of importing
 *    levels into Sonic Adventure 2 without having to write code. But for you
 *    creative few, this is where you can add in your own code! Go crazy! Just
 *    try not to break anything :)
 * 
 *    X-Hax discord for code questions: https://discord.gg/gqJCF47
 */

#include "pch.h"
#include "LevelImporter.h"

LevelImporter* myLevelMod;

extern "C" {
	// Required.
	__declspec(dllexport) void Init(const char* path, const HelperFunctions& helperFunctions) {
		// Loads and Imports level. Do not remove.
		myLevelMod = new LevelImporter(path, helperFunctions);
		myLevelMod->init();
	}
	
	// Required.
    __declspec(dllexport) void __cdecl OnFrame() {
		// My Level Mod OnFrame function. Do not remove.
		if (myLevelMod != nullptr) {
			myLevelMod->onFrame();
		}
	}

	__declspec(dllexport) ModInfo SA2ModInfo = { ModLoaderVer };
}

// Required.
// A Function 'Hook,' that automatically runs onLevelLoad() whenever the
// LoadLevel internal function runs. Feel free to put logic here!
void onLevelLoad();
FunctionHook<void> InitCurrentLevelAndScreenCount_h(InitCurrentLevelAndScreenCount, onLevelLoad);
void onLevelLoad() {
	InitCurrentLevelAndScreenCount_h.Original();
	if (myLevelMod != nullptr) {
		myLevelMod->onLevelHook();
	}
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
