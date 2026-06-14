#pragma once
#include "ChatTaskMgr.h"
#include "LlmChat.h"
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <unordered_map>


struct SymbolLinkItem
{
	std::wstring messageId;
	std::wstring symbol;
};

// 符号解析结果
struct SymbolResolveResult
{
	int fileIndex; // 文件索引，通过 CChatTask_ResolveSymbolLinks::GetFilePath() 获取路径
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

	// 文件索引表（全局唯一，无需考虑多线程）
	static int GetFileIndex(const std::string& filePath);
	static const std::string& GetFilePath(int fileIndex);
	// 获取文件的基名（去后缀的全路径，用于同名文件判断，如 src\Foo.h 与 src\Foo.cpp 基名相同）
	static const std::string& GetFileBaseName(int fileIndex);
	static void ClearFileTable();

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

	// 文件索引表
	static std::vector<std::string> s_fileTable;
	static std::unordered_map<std::string, int> s_fileToIndex;
	// 文件基名表（去后缀全路径，与 s_fileTable 同序），用于同名文件判断
	static std::vector<std::string> s_fileBaseNames;
};