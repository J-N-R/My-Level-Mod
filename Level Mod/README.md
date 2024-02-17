# Important files
**MyLevelMod.cpp** - Entry point for My Level Mod, where users can add their own code.

**LevelImporter.cpp** - A library that automates importing levels into Sonic Adventure 2.
* Main functions:
  * init(): Sets up the LevelImporter, fixes file structure problems, as well as checks for any updates.
  * importLevel(landTableName, levelFileName, texturePakName): Imports a level into Sonic Adventure 2.
    * The parameters are optional, see the inlined documentation to learn more.

**IniReader.cpp** - A library that automates reading from level_options.ini, and can read Spline files.
* Main functions:
  * readLevelOptions(): Reads and stores options from level_options.ini, which is then used by LevelImporter.
  * readSplines(): Reads all Splines in the mod folder, and loads it into your level.
  * readSpline(filePath): Reads a given Spline file and returns Spline information. Useful for manually reading splines.
