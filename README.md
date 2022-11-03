# My Level Mod
A custom level importer for Sonic Adventure 2! The goal of this project is to allow anyone
to make a level for Sonic Adventure 2 without having to write a single line of code. 
Installed as a regular mod, this tool will give you access to level configuration tools
and is built as a template so that anyone making their own custom level can simply
republish the mod to share with others.

Used in the SA2 Modding Tutorial [How to Create a Custom Level for Sonic Adventure 2](https://github.com/X-Hax/SA2BModdingGuide/wiki/Basic-Level-Modding).

Special thanks to the [X-Hax community](https://github.com/X-Hax/) for the provided libraries as well as coding help!

Feel free to contribute or message me for any information! J-N-R

[Download Latest Version](https://github.com/J-N-R/My-Level-Mod/releases)

## Features
- Support for importing custom levels and texture packs into Sonic Adventure 2.
- Support for importing custom splines into your level.
- Moves traditionally code-bound options to an easy to use ini file.
- Automatically moves files into the right place for ease of use.
- Auto update DETECTION
- Public libraries 'LevelImporter' and 'iniReader' so that you can include this code in your own mods.
- Simple dllmain.cpp so that anyone can make easy adjustments / additions for their own mods.
- Level options currently support (all of these are optional)
- * Character spawn position
- * Character victory position
- * Import Level ID (to choose which level you would like to port over)
- * Chao Garden import ID (to choose which chao garden you would like to port over)
- * Import Level (A more complex option to import levels, allowing users to import multiple levels)
- * Simple Death Plane (Have sonic die under a certain y-level)

## Future goals
- Add SET file functionality for easy goal ring placement.
- ~~Add Spline functionality for loops / rails.~~
- ~~Convert existing code to library.~~
- ~~Add Chao garden support.~~
- ~~Add support for multiple levels.~~
- Add support for SA1

## NOTE
Do NOT update "VERSION.txt" unless you have updated My Level Mod. It will prompt a manual update to My Level Mod users.

If you would like to advance the project, all you need is visual studio code and the Sonic Adventure 2 Mod Loader libmodutils files. Contact the x-hax team at discord for information about necessary libraries.
