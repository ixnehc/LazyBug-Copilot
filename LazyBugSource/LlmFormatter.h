#pragma once

#include <string>
#include <vector>
#include <deque>

#include "LlmLibDefines.h"

struct LlmApi;

class CLlmFormatter
{
public:

	static bool ConvertLlmRequestToAnthoropicFormat(json& requestJson);
	static bool ProcessLlmResponseFromAnthropicFormat(std::deque<std::string>& inputLines, std::vector<std::string>& outputLines, const LlmApi& api);

	static bool ConvertLlmRequestToGeminiFormat(json& requestJson);
	static bool ProcessLlmResponseFromGeminiFormat(std::deque<std::string>& inputLines, std::vector<std::string>& outputLines, const LlmApi& api);

	static bool ConvertLlmRequestToOpenAiCompatibleFormat(json& requestJson,LlmApiFormat fmt);
	static bool ProcessLlmResponseFromOpenAiCompatibleFormat(std::deque<std::string>& inputLines, std::vector<std::string>& outputLines, const LlmApi& api);

};

