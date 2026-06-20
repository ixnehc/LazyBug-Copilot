#pragma once


enum class LlmApiRole
{
	None,
	Agent,      // 主要代理
	Auxiliary,  // 辅助
};

enum class LlmApiFormat
{
	Unknown,
	OpenAI_,
	Anthropic_,
	Gemini_,
	OpenRouter,
	Kimi,
	GLM,
	Minimax,
	DeepSeek,
};

using LlmApiProviderTypeName = std::string;

enum LlmApiCacheControlType
{
	None_,
	Auto,
	Anthropic_,
};

enum class LlmToolType
{
	None,
	ReplaceInFile,
	FindSymbolDefine,
	FindInFiles,
	SearchFile,
	ReadFile,
	CLI_Cmd,
	CLI_Bash,
	CLI_RunScript,
	Question,
	QueryFinish,
	CreateSkill,
	Mcp,
};

enum class LlmThinkingMode
{
	Auto,
	Enable,
	Disable,
};

// Summarize API 特殊选项
#define SUMMARIZE_API_DISABLE "<disable>"
#define SUMMARIZE_API_AUTO "<auto>"