#include "pch.h"

std::string removeFileExtension(std::string fileName) {
	std::string fileNameCopy = fileName; // C++ 11 forces deep copy.
	size_t lastDot = fileNameCopy.find_last_of(".");
	if (lastDot != std::string::npos) {
		fileNameCopy = fileNameCopy.substr(0, lastDot);
	}
	return fileNameCopy;
}

void printDebug(std::string message) {
	PrintDebug(("[My Level Mod] " + message).c_str());
}

void showWarning(std::string message) {
	printDebug(message);
	MessageBoxA(
		NULL,                     // Owner window
		message.c_str(),          // The text to display
		"[My Level Mod] warning", // The title of the window
		MB_OK | MB_ICONWARNING    // Buttons + Warning Icon
	);
}