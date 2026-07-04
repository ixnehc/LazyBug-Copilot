#pragma once

#include <string>
#include <vector>

#include "LlmLib.h"

class CLlmLibLoader
{
public:

	static void LoadInto(std::vector<LlmApiProvider>& providers, std::vector<LlmApi>& apis, const char* iniPath);
	static void LoadJsonFile(CLlmLib &lib, const char *jsonFilePath);
	static void SaveJsonFile(CLlmLib &lib, const char *jsonFilePath);

};

