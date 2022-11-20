# My Level Mod
A tool that makes importing custom levels into Sonic Adventure 2 easy for anyone. The goal
of this project is to do away with all code requirements, and let modders focus on creativity
and building their own level.

With this tool, modders can simply drag and drop their level files, and play! An exposed
'level_options.ini' file allows modders to quickly add features into their level without
code, and more features are constantly being added. This tool also acts as a mod template,
so once you've added your level, feel free to republish as your own mod to share with friends.
Happy Modding!

## Start by checking the [wiki](https://github.com/J-N-R/My-Level-Mod/wiki).

## Notes
This tool was built and used in the [How to Create a Custom Level for Sonic Adventure 2](https://github.com/X-Hax/SA2BModdingGuide/wiki/Basic-Level-Modding) tutorial, on the official Sonic Adventure 2 modding wiki.

Special thanks to the [X-Hax community](https://github.com/X-Hax/) for provided libraries as well as coding help!

Feel free to contribute or message me for any information! J-N-R

[Download Latest Version](https://github.com/J-N-R/My-Level-Mod/releases)

## Features
* Simple drag-and-drop setup to begin playing your level.
* Requires zero coding knowledge to use.
* Auto update detection and notifications.
* Adds optional features through an 'options' file, with support for:
   * Multiple level imports

   * Spawn location
   
   * Victory pose location
   
   * Death plane (have Sonic die under a certain y-level)
  
   * Specific level ID you would like to replace
   
* Comes with 2 public c++ libraries for those who do know how to code:
   * LevelImporter.cpp - A tool that lets you easily import and modify levels. (Requires IniReader)
   
   * IniReader.cpp - A tool that can read options from an ini file, and can load spline/rail data from files.
   
* Completely open source, and very easy to add your own custom code (See MyLevelMod.cpp).
   
## Future goals
- Add SET file functionality for easy goal ring placement.
- ~~Add Spline functionality for loops / rails.~~
- ~~Convert existing code to library.~~
- ~~Add Chao garden support.~~
- ~~Add support for multiple levels.~~
- Add support for SA1

## NOTE
For developers who would like to contribute, do NOT update "VERSION.txt" unless you have updated My Level Mod. It will prompt a manual update to My Level Mod users.

If you would like to advance the project, all you need is visual studio code and the Sonic Adventure 2 Mod Loader libmodutils files. Contact the x-hax team at discord for information about necessary libraries.
