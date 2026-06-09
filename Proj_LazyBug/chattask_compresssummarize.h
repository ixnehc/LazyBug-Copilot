#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>

class CChatTask_CompressSummarize : public CChatTask
{
public:
	// workingOpIndex: CChatOpsCompress::_workingOps 的索引
	// summarizeApiName: 用于 summarize 的 API 名称
	// isSessionMode: 是否为 session 模式（压缩整个 session）
	CChatTask_CompressSummarize(int workingOpIndex, const std::string& summarizeApiName, bool isSessionMode = false);

	const char* GetType() override { return "CompressSummarize"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	bool NeedLlmSession() override { return true; }

private:
	void _Fail(const std::string& reason = "");
	void _Succeed(const std::string& result);

	// 收集 session 中需要压缩的内容
	std::string _CollectSessionContent();

	// 生成压缩结果 log 字符串
	std::string _MakeCompressLogString(const std::string& originalContent, const std::string& compressedContent, int originalTokens, int compressedTokens);

	// 生成简短结果信息字符串
	std::string _MakeShortResultString(bool success, const std::string& reason, int originalTokens = 0, int compressedTokens = 0);

	std::string _originalContent;
	int _originalTokenCount;
	std::string _resultMessage;  // 用于 UI 显示的简短结果信息

	bool _hasStartedRequest;
	bool _requestInterrupt;
	int _workingOpIndex;
	std::string _summarizeApiName;
	bool _isSessionMode;  // 是否为 session 模式
};