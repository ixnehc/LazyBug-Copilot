#pragma once

class CCheckpoints;

#define DEFAULT_CHAT_TITLE L"[ Untitled Chat ]"

// CLI 显示状态枚举（用于 AddCliDisplay 参数）
enum class CliDisplayStatus
{
	None,     // 不在白名单，需要用户确认
	Pending,  // 在白名单，自动执行但显示停止按钮
	Accepted  // 已被用户接受或白名单命令，自动执行
};

// CLI 运行时状态枚举
enum class CliStatus
{
	None,    // 非pending状态
	Pending, // 等待用户操作
	Reject,  // 用户点击禁止
	Accept,  // 用户点击播放
	Stop     // 用户点击停止（终止运行中的CLI）
};

// MCP 运行时状态枚举
enum class McpStatus
{
	None,    // 正常状态
	Stop     // 用户点击停止
};

class IChatUi
{
public:

	virtual void PostJsonMessage(const std::wstring& jsonMessage)	{	}

	// 应用 Symbol 链接样式
	// symbolsWithResults: vector<pair<symbol, resultsJson>>
	// resultsJson 格式: [{"filePath":"xxx","lineNumber":123},...]
	virtual void ApplySymbolLinks(const std::wstring& messageId, const std::vector<std::pair<std::wstring, std::wstring>>& symbolsWithResults)	{	}

	virtual void ActivateCheckpointFileChange(const std::wstring& fileEditId)	{	}

	virtual void ShowPause(bool show, bool flow = true)	{	}
	virtual void StopPauseFlow(bool stop)	{	}

	// Question/Answer 相关方法
	virtual __int64 AddQuestion(const std::wstring& messageId, const std::wstring& question, const std::vector<std::wstring>& options, bool multiSelect = false) { return 0; }
	virtual bool GetQuestionAnswer(__int64 questionId, std::wstring& answer) { return false; }
	virtual bool HasQuestionAnswer(__int64 questionId) { return false; }
	virtual void ClearQuestion() { }
	
	// Question Display 方法 - 显示问题和答案
	virtual void AddQuestionDisplay(const std::wstring& messageId, const std::wstring& question, const std::wstring& answer) { }

	// CLI 输入相关方法
	virtual void SendCliInput(const std::wstring& cliId, const std::wstring& input) { }
	virtual bool GetCliInput(const std::wstring& cliId, std::wstring& input) { return false; }
	virtual bool HasCliInput(const std::wstring& cliId) { return false; }
	virtual void ClearCliInput() { }

	// CLI Display 相关方法
	virtual void AddCliDisplay(const std::wstring& messageId, const std::wstring& cliId, const std::wstring& command, const std::wstring& desc = L"", CliDisplayStatus displayStatus = CliDisplayStatus::None, const std::wstring& shellType = L"") { }
	virtual bool IsCliPending(const std::wstring& cliId) { return false; }
	virtual void RemovePendingCli(const std::wstring& cliId) { }
	virtual CliStatus GetCliStatus(const std::wstring& cliId) { return CliStatus::None; }
	virtual void SetCliStatus(const std::wstring& cliId, CliStatus status) { }

	// MCP Display 相关方法
	virtual void AddMcpDisplay(const std::wstring& messageId, const std::wstring& mcpId, const std::wstring& mcpName, const std::wstring& toolName, const std::wstring& arguments, const std::wstring& argsSummary) { }
	virtual void SetMcpResult(const std::wstring& mcpId, const std::wstring& result) { }
	virtual McpStatus GetMcpStatus(const std::wstring& mcpId) { return McpStatus::None; }
	virtual void SetMcpStatus(const std::wstring& mcpId, McpStatus status) { }

};

class IChatNotify
{
public:
	// 发送到 LLM 前通知
	// isUserMessage: true = 用户发送消息, false = 发送 ToolCall 结果
	// 返回 false 可取消请求
	virtual bool OnBeforeSendToLlm(bool isUserMessage) { return true; }

	// 从 LLM 接收完成后通知
	virtual void OnAfterReceiveFromLlm() {}

	//返回要压缩的token数,返回0表示不需要压缩
	virtual int OnCheckCompress()	{		return 0;	}
};

struct ChatAgentContext
{
	ChatAgentContext()
	{
		checkpoints = nullptr;
	}
	std::string dbFolderPath;
	CCheckpoints* checkpoints;
	std::vector<std::string> rulesFiles;
};

