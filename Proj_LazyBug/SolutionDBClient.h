#pragma once

#include <Windows.h>
#include <string>
#include <future>
#include <thread>
#include <mutex>
#include <unordered_map>
#include <atomic>
#include "PipeMsg.h"

class CSolutionDBClient
{
public:
	CSolutionDBClient();
	~CSolutionDBClient();

	bool Connect();
	void Disconnect();

	FuturePipeMsg SendMessage(const PipeMsg& request);
	void SendMessageNoReturn(const PipeMsg& request);
	bool IsConnected()	{		return _isConnected;	}

private:
	void ReaderLoop();

	bool _isConnected;

	HANDLE _hRequestPipe;
	HANDLE _hResponsePipe;
	std::wstring _requestPipeName;
	std::wstring _responsePipeName;

	std::thread _readerThread;
	std::atomic<bool> _stopReader;

	std::mutex _mutex;
	std::unordered_map<unsigned int, std::promise<PipeMsgPtr>> _pendingRequests;
	std::atomic<unsigned int> _nextRequestId;
	std::mutex _pipeMutex; // Mutex to protect pipe writes
};
