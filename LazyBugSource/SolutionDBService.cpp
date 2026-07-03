#include "stdh.h"
#include "SolutionDBService.h"
#include "SolutionDBs.h"
#include "SolutionDBMsgs.h"
// #include "CppSymbol.h"
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"
#include <vector>

extern CSolutionDBs g_solutionDBs;

const char* GetCurModuleFolderPath_utf8()
{
	static std::string pathUtf8;
	if (pathUtf8.empty())
	{
		wchar_t buffer[512];
		GetModuleFileNameW(NULL, buffer, 500);
		std::wstring wstr(buffer);
		// 找到最后一个反斜杠，截断得到目录路径
		size_t lastSlash = wstr.rfind(L'\\');
		if (lastSlash != std::wstring::npos)
		{
			wstr = wstr.substr(0, lastSlash);
		}
		pathUtf8 = widechar_to_utf8(wstr.c_str());
	}
	return pathUtf8.c_str();
}


//////////////////////////////////////////////////////////////////////////
//CSolutionDBService

CSolutionDBService::CSolutionDBService() : _hRequestPipe(INVALID_HANDLE_VALUE), _hResponsePipe(INVALID_HANDLE_VALUE), _isRunning(false)
{
	_requestPipeName = SOLUTIONDB_SERVICE_PIPE_NAME_REQUEST;
	_responsePipeName = SOLUTIONDB_SERVICE_PIPE_NAME_RESPONSE;
}

CSolutionDBService::~CSolutionDBService()
{
}

void CSolutionDBService::Start()
{
	if (_isRunning) return;
	_startTime = std::chrono::steady_clock::now();
	_isRunning = true;
	_serverThread = std::thread(&CSolutionDBService::ServerLoop, this);
}

void CSolutionDBService::Stop()
{
	if (!_isRunning)
		return;

	_isRunning = false;

	// Unblock the listener by connecting to the pipes
    HANDLE hRequestPipe = CreateFileW(
        _requestPipeName.c_str(),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (hRequestPipe != INVALID_HANDLE_VALUE)
    {
		CancelIoEx(hRequestPipe,NULL);
        CloseHandle(hRequestPipe);
    }

    HANDLE hResponsePipe = CreateFileW(
        _responsePipeName.c_str(),
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (hResponsePipe != INVALID_HANDLE_VALUE)
    {
		CancelIoEx(hResponsePipe,NULL);
        CloseHandle(hResponsePipe);
    }

	if (_serverThread.joinable())
	{
		_serverThread.join();
	}

	_servers.clear(); // This will call destructors of CSolutionDBServer

	g_solutionDBs.CloseAll();
}

void CSolutionDBService::ServerLoop()
{
	while (_isRunning)
	{
		// 创建请求管道（服务器读取，客户端写入）
		_hRequestPipe = CreateNamedPipeW(
			_requestPipeName.c_str(),
			PIPE_ACCESS_INBOUND,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			1024 * 256, // Increased buffer size for larger messages
			1024 * 256,
			0, NULL);

		if (_hRequestPipe == INVALID_HANDLE_VALUE)
		{
			// Error handling
			//CLog::GetInstance()->Log(L"Failed to create request named pipe.");
			Sleep(1000);
			continue;
		}

		// 创建响应管道（服务器写入，客户端读取）
		_hResponsePipe = CreateNamedPipeW(
			_responsePipeName.c_str(),
			PIPE_ACCESS_OUTBOUND,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
			PIPE_UNLIMITED_INSTANCES,
			1024 * 256, // Increased buffer size for larger messages
			1024 * 256,
			0, NULL);

		if (_hResponsePipe == INVALID_HANDLE_VALUE)
		{
			// Error handling
			//CLog::GetInstance()->Log(L"Failed to create response named pipe.");
			CloseHandle(_hRequestPipe);
			Sleep(1000);
			continue;
		}

		// 等待客户端连接到请求管道
		if (ConnectNamedPipe(_hRequestPipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED)
		{
			if (!_isRunning)
			{
				CloseHandle(_hRequestPipe);
				CloseHandle(_hResponsePipe);
				break;
			}

			// 等待客户端连接到响应管道
			if (ConnectNamedPipe(_hResponsePipe, NULL) || GetLastError() == ERROR_PIPE_CONNECTED)
			{
				if (!_isRunning)
				{
					CloseHandle(_hRequestPipe);
					CloseHandle(_hResponsePipe);
					break;
				}

				std::lock_guard<std::mutex> lock(_clientsMutex);
				auto server = std::make_unique<CSolutionDBServer>(_hRequestPipe, _hResponsePipe);
				server->Run();
				_servers.push_back(std::move(server));
			}
			else
			{
				CloseHandle(_hRequestPipe);
				CloseHandle(_hResponsePipe);
			}
		}
		else
		{
			CloseHandle(_hRequestPipe);
			CloseHandle(_hResponsePipe);
		}
	}
}

bool CSolutionDBService::Update()
{
	g_solutionDBs.Update();

	const auto timeout = std::chrono::seconds(3); // 3秒无客户端即停止
	auto now = std::chrono::steady_clock::now();

	std::lock_guard<std::mutex> lock(_clientsMutex);

	bool allDisconnected = true;
	for (const auto& server : _servers)
	{
		if (!server->IsDisconnected())
		{
			allDisconnected = false;
			break;
		}

		auto disconnectedTime = server->GetDisconnectedTime();
		if (now - disconnectedTime < timeout)
		{
			allDisconnected = false;
			break;
		}
	}

	if (now < _startTime + timeout)
		allDisconnected = false;

	if (allDisconnected)
	{
		Stop();
		return false;
	}

	return true;
}
