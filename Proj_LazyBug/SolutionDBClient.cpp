#include "stdh.h"
#include "SolutionDBClient.h"
#include "SolutionDBMsgs.h"
#include "datapacket/DataPacket.h"
#include <iostream>
#include <vector>

CSolutionDBClient::CSolutionDBClient() 
    : _hRequestPipe(INVALID_HANDLE_VALUE), _hResponsePipe(INVALID_HANDLE_VALUE), _stopReader(false), _nextRequestId(1)
{
	_requestPipeName = SOLUTIONDB_SERVICE_PIPE_NAME_REQUEST;
	_responsePipeName = SOLUTIONDB_SERVICE_PIPE_NAME_RESPONSE;

	_isConnected = false;
}

CSolutionDBClient::~CSolutionDBClient()
{
	Disconnect();
}

bool CSolutionDBClient::Connect()
{
	if (_hRequestPipe != INVALID_HANDLE_VALUE && _hResponsePipe != INVALID_HANDLE_VALUE)
	{
		return true; // Already connected
	}

	// 连接到请求管道（客户端写入，服务器读取）
	_hRequestPipe = CreateFileW(
		_requestPipeName.c_str(),
		GENERIC_WRITE,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (_hRequestPipe == INVALID_HANDLE_VALUE)
	{
		return false;
	}

	// 连接到响应管道（客户端读取，服务器写入）
	_hResponsePipe = CreateFileW(
		_responsePipeName.c_str(),
		GENERIC_READ,
		0,
		NULL,
		OPEN_EXISTING,
		0,
		NULL);

	if (_hResponsePipe == INVALID_HANDLE_VALUE)
	{
		CloseHandle(_hRequestPipe);
		_hRequestPipe = INVALID_HANDLE_VALUE;
		return false;
	}

	_isConnected = true;

	_stopReader = false;
	_readerThread = std::thread(&CSolutionDBClient::ReaderLoop, this);

	return true;
}

void CSolutionDBClient::Disconnect()
{
	if (_stopReader == false)
	{
		_stopReader = true;

		if (_hRequestPipe != INVALID_HANDLE_VALUE)
		{
			CancelIoEx(_hRequestPipe, NULL);
			CloseHandle(_hRequestPipe);
			_hRequestPipe = INVALID_HANDLE_VALUE;
		}

		if (_hResponsePipe != INVALID_HANDLE_VALUE)
		{
			CancelIoEx(_hResponsePipe, NULL);
			CloseHandle(_hResponsePipe);
			_hResponsePipe = INVALID_HANDLE_VALUE;
		}

		if (_readerThread.joinable())
		{
			_readerThread.join();
		}
	}
	_isConnected = false;
}

FuturePipeMsg CSolutionDBClient::SendMessage(const PipeMsg& request)
{
	if (_hRequestPipe == INVALID_HANDLE_VALUE)
	{
		_isConnected = false;
		return FuturePipeMsg();   // 无效返回值
	}

	unsigned int requestId;
	auto promise = std::make_unique<std::promise<PipeMsgPtr>>();
	auto future = promise->get_future();

	{
		std::lock_guard<std::mutex> lock(_mutex);
		requestId = _nextRequestId++;
		_pendingRequests[requestId] = std::move(*promise);
	}

	std::vector<BYTE> payloadBuffer;
	DP_BeginSave(dp, payloadBuffer)
	{
		dp.Data_WriteSimple(requestId);
		dp.Data_WriteSimple(request.GetType());
		request.Save(dp);
	}
	DP_EndSave();

	DWORD bytesWritten = 0;
	{
		std::lock_guard<std::mutex> pipeLock(_pipeMutex);
		if (!WriteFile(_hRequestPipe, payloadBuffer.data(), static_cast<DWORD>(payloadBuffer.size()), &bytesWritten, NULL))
		{
			_isConnected = false;

			// 写失败：把 promise 设为 nullptr 并返回无效 FuturePipeMsg
			std::lock_guard<std::mutex> lock(_mutex);
			auto it = _pendingRequests.find(requestId);
			if (it != _pendingRequests.end())
			{
				it->second.set_value(nullptr);
				_pendingRequests.erase(it);
			}
			return FuturePipeMsg();
		}
	}

	return FuturePipeMsg(std::move(future));
}

void CSolutionDBClient::SendMessageNoReturn(const PipeMsg& request)
{
	if (_hRequestPipe == INVALID_HANDLE_VALUE)
	{
		_isConnected = false;
		return;   // 无效，直接返回
	}

	// 序列化数据 (只写消息类型和内容, 不分配 requestId)
	std::vector<BYTE> payloadBuffer;
	DP_BeginSave(dp, payloadBuffer)
	{
		unsigned int dummyRequestId = 0; // 用 0 作为无返回消息的 requestId
		dp.Data_WriteSimple(dummyRequestId);
		dp.Data_WriteSimple(request.GetType());
		request.Save(dp);
	}
	DP_EndSave();

	DWORD bytesWritten = 0;
	{
		std::lock_guard<std::mutex> pipeLock(_pipeMutex);
		if (!WriteFile(_hRequestPipe, payloadBuffer.data(), static_cast<DWORD>(payloadBuffer.size()), &bytesWritten, NULL))
		{
			_isConnected = false;
		}
	}
}


void CSolutionDBClient::ReaderLoop()
{
	const int BUFFER_SIZE = 256 * 1024; // 256k
	std::vector<char> buffer(BUFFER_SIZE);
	DWORD bytesRead = 0;
	 
	while (!_stopReader && ReadFile(_hResponsePipe, buffer.data(), buffer.size(), &bytesRead, NULL))
	{
		if (bytesRead == 0)
		{
			continue;
		}

		if (bytesRead >= BUFFER_SIZE)
		{
			// Message truncated, this is an error condition.
			break;
		}

		CDataPacket dp;
		dp.SetDataBufferPointer((unsigned char*)buffer.data());

		unsigned int requestId;
		dp.Data_ReadSimple(requestId);

		PipeMsgType msgId;
		dp.Data_ReadSimple(msgId);

		std::lock_guard<std::mutex> lock(_mutex);
		auto it = _pendingRequests.find(requestId);
		if (it != _pendingRequests.end())
		{
			auto msg = CreateSolutionDBMsg(msgId);
			if (msg)
			{
				msg->Load(dp);
				it->second.set_value(std::move(msg));
			}
			else
			{
				// Unknown message, fulfill promise with nullptr
				it->second.set_value(nullptr);
			}
			_pendingRequests.erase(it);
		}
	}
}

