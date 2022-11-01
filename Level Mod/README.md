# Source Code
A majority of the code in this folder are libraries provided by X-Hax. The My-Level-Mod code lies in these few files:

**MyLevelMod.cpp** - Entry point for My Level Mod, where users can add their own mod code.

**LevelImporter.cpp** - A library class built to make importing levels into Sonic Adventure 2 easier.
* init(): Sets up the LevelImporter, fixes file structure problems, as well as checks for any updates.
* importLevel(landTableName, levelFileName, texturePakName): Imports a level into Sonic Adventure 2.
* * The parameters are optional, and the function will attempt to fill in the blanks with the given information.
* * importLevel() will port over the landtable specified in level_options.ini, and will grab and .sa2blvl and .pak file it can find.
* * importLevel(landTableName) will port over a given landTable, and will attempt to grab a .sa2blvl and .pak file named after the landTable.
* * importLevel(landTableName, levelFileName) will port over a given landTable, and will attempt to grab the given .sa2blvl and a .pak named after the .sa2blvl file.
* * importLevel(landTableName, levelFileName, texturePakName) will port over the given landtable, and use the given .sa2blvl file and .pak file.

**IniReader.cpp** - A helper library class used by LevelImporter.cpp to read an options file.

## Note
The library files are from the [SA2ModLoader](https://github.com/X-Hax/sa2-mod-loader) and will not update automatically. Check for updates periodically for fixes and
new features!
