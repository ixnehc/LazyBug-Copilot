#pragma once

#include <string>
#include <vector>

#include "LlmLib.h"

class CLlmLibLoader
{
public:

	static void LoadInto(std::vector<LlmApiProvider>& providers, std::vector<LlmApi>& apis, const char* iniPath);

};

