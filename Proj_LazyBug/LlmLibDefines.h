#pragma once


enum class LlmApiPurpose
{
	None,
	MajorChat,
	MinorChat,//简单对话
	FastApply_Dedicated,//第一优先
	FastApply_Adaptive,//第二优先
	Embedding,
	Complete,
	Max,
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
};

enum class LlmThinkingMode
{
	Auto,
	Enable,
	Disable,
};