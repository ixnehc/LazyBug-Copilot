#pragma once

#include <string>
#include <thread>
#include <vector>
#include <mutex>

#include "SolutionDBMsgs.h"

#include "SolutionDBServer.h"

// 前向声明
class CSolutionDBs;

struct PipeMsg;

class CSolutionDBService
{
public:
	CSolutionDBService();
	~CSolutionDBService();

	void Start();
	bool Update();//返回false表示service已经结束了

private:
	void Stop();
	void ServerLoop();

	std::wstring _requestPipeName;
	std::wstring _responsePipeName;
	HANDLE _hRequestPipe;
	HANDLE _hResponsePipe;
	bool _isRunning;
	std::thread _serverThread;
	std::vector<std::unique_ptr<CSolutionDBServer>> _servers;
	std::mutex _clientsMutex;

	std::chrono::steady_clock::time_point _startTime;
};
