#pragma once

#include <string>
#include <thread>
#include <vector>
#include <mutex>

#include "SolutionDBMsgs.h"

// 前向声明
class CSolutionDBs;

struct PipeMsg;
class CSolutionDBServer
{
public:
	CSolutionDBServer(HANDLE hRequestPipe, HANDLE hResponsePipe);
	~CSolutionDBServer();

	void Run();

	bool IsDisconnected() const { return _isDisconnected.load(); }
	std::chrono::steady_clock::time_point GetDisconnectedTime() const 
	{ 
		std::lock_guard<std::mutex> lock(_disconnectMutex);
		return _lastDisconnectedTime; 
	}

private:
	bool SendMessage(const PipeMsg& msg, unsigned int requestId);

	bool _QueryNameItems(const SolutionDBMsg_QueryNameItems &request,SolutionDBMsg_NameItems& result);
	int _CalculateNameScore(const std::string& text, const std::string& query);
	void _SortAndLimitResults(std::vector<SolutionDBMsg_NameItems::Item>& items, int maxCount);

	void _CollectRefs(const SolutionDBMsg_CollectRefs& request, SolutionDBMsg_Refs& result);
	void _FindSymbolDefine(const SolutionDBMsg_FindSymbolDefine& request, SolutionDBMsg_SymbolDefines& result);
	void _FindInFiles(const SolutionDBMsg_FindInFiles& request, SolutionDBMsg_FindInFilesResults& result);
	void _SearchFile(const SolutionDBMsg_SearchFile& request, SolutionDBMsg_SearchFileResult& result);
	void _SetEmbeddingModel(const SolutionDBMsg_SetEmbeddingModel& request, SolutionDBMsg_EmbeddingModelSet& response);
	void _ActivateFiles(const SolutionDBMsg_ActivateFiles& request, SolutionDBMsg_ActivateFilesResult& response);

	HANDLE _hRequestPipe;
	HANDLE _hResponsePipe;
	std::thread _thread;
	std::mutex _pipeMutex; // 添加互斥锁以保护管道写入
	
	std::atomic<bool> _isDisconnected{false};
	mutable std::mutex _disconnectMutex;
	std::chrono::steady_clock::time_point _lastDisconnectedTime;
};

