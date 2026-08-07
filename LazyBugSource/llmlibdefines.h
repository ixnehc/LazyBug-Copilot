#pragma once


enum class LlmApiRole
{
	None,
	Agent,      // 主要代理
	Auxiliary,  // 辅助
	Embedding,  // 嵌入
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
	OpenAIResponses,
};

// ReadMedia tool 仅在这些格式下可用（支持多模态图片输入）
inline bool IsReadMediaSupported(LlmApiFormat format)
{
	return format == LlmApiFormat::Anthropic_ || format == LlmApiFormat::Kimi ||
	       format == LlmApiFormat::OpenAI_ || format == LlmApiFormat::OpenAIResponses;
}

// 判断 API 格式是否支持在 tool result 消息中直接包含 media 内容（image_url block）
// 不支持的格式会将 media 提取为独立的 user 多模态消息
inline bool IsToolResultMediaSupported(LlmApiFormat format)
{
	return format == LlmApiFormat::Anthropic_ || format == LlmApiFormat::Kimi;
}

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
	ReadMedia,
	CLI_Cmd,
	CLI_Bash,
	CLI_RunScript,
	Question,
	QueryFinish,
	CreateSkill,
	Mcp,
	AddMcpServer,
	//XXXXX: more tool type  
	Max,
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

// Embedding API 特殊选项
#define EMBEDDING_API_DISABLE "<disable>"

// InputHint API 特殊选项
#define INPUTHINT_API_DISABLE "<disable>"