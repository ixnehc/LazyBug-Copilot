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

	static bool ConvertLlmRequestToOpenAIResponsesFormat(json& requestJson, bool store);
	static bool ConvertLlmRequestToOpenAIResponsesFormat(json& requestJson, const std::string& previousResponseId, bool store);
	static bool ProcessLlmResponseFromOpenAIResponsesFormat(std::deque<std::string>& inputLines, std::vector<std::string>& outputLines, const LlmApi& api);

	// 清理请求 JSON 中的临时标记字段（如 _current_turn），所有格式转换完成后调用
	static void CleanupTempFields(json& requestJson);

};

