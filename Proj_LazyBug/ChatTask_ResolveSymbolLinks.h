#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>


struct SymbolLinkItem
{
	std::wstring messageId;
	std::wstring symbol;
};

// 符号解析结果
struct SymbolResolveResult
{
	std::string filePath;
	int lineNumber; // -1 表示没有行号
};

class CChatTask_ResolveSymbolLinks : public CChatTask
{
public:
	CChatTask_ResolveSymbolLinks();
	CChatTask_ResolveSymbolLinks(const std::vector<SymbolLinkItem>& symbolLinks);
	~CChatTask_ResolveSymbolLinks();
	
	const char* GetType() override { return "ResolveSymbolLinks"; }
	void Start() override;
	void Update() override;
	void Interrupt() override;
	bool NeedLlmSession() override { return false; }

	bool DependsOn(CChatTask* task) override;

	void SetSymbolLinks(const std::vector<SymbolLinkItem>& symbolLinks);

	// 模糊查询符号位置
	// 返回所有匹配结果，不再限制数量
	static std::vector<SymbolResolveResult> FuzzyResolveSymbol(const std::string& dbFolderPath, const std::string& rawSymbolText);

private:
	void _Fail();
	void _Succeed();
	void _ThreadFunc();

	std::string _dbFolderPath;
	
	std::thread* _workerThread;
	std::atomic<bool> _shouldStop;
	std::atomic<bool> _threadFinished;
	std::mutex _resultMutex;

	std::string _threadResult;
	std::string _threadMessage;
	bool _threadSuccess;

	std::vector<SymbolLinkItem> _symbolLinks;
	
	// 已解析的符号队列（线程安全）
	std::mutex _resolvedQueueMutex;
	// <messageId, symbol, resultsJson>
	std::vector<std::tuple<std::wstring, std::wstring, std::wstring>> _resolvedQueue;
};