#include "stdh.h"
#include "LlmMcpServers.h"
#include "LlmMcps.h"
#include "stringparser/stringparser.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>

// 使用项目中已有的json库
using json = nlohmann::ordered_json;

CLlmMcpServers g_llmMcpServers;

// 优先级: Project > Global
static int _Priority(CLlmMcps::Mcp::Type tp)
{
	switch (tp)
	{
	case CLlmMcps::Mcp::Type::Project:
		return 2;
	case CLlmMcps::Mcp::Type::Global:
		return 1;
	default:
		return 0;
	}
}

CLlmMcpServers::~CLlmMcpServers()
{
	// 等待worker线程结束
	if (_workerThread.joinable())
	{
		_workerThread.join();
	}

	// 销毁所有server
	for (auto& server : _servers)
	{
		_DestroyServer(server);
	}
	_servers.clear();

	// 销毁pending servers
	for (auto& server : _pendingServers)
	{
		_DestroyServer(server);
	}
	_pendingServers.clear();
}

bool CLlmMcpServers::_NeedSync() const
{
	return _syncedVer != g_llmMcps._ver;
}

bool CLlmMcpServers::UpdateSync()
{
	// 1. 检查 worker 状态
	if (_workerRunning)
	{
		bool isServerModified = false;

		// worker 正在运行，检查是否完成
		if (_workerDone)
		{
			// worker 已完成
			std::lock_guard<std::mutex> lock(_mutex);

			// 清理 worker 状态
			_workerDone = false;
			_workerRunning = false;
			if (_workerThread.joinable())
				_workerThread.detach();

			// 检查版本是否仍然匹配
			if (_pendingVer == g_llmMcps._ver)
			{
				// 版本匹配，同步成功
				// 使用 _pendingRemoveUids 销毁不再需要的旧servers
				for (auto& old : _servers)
				{
					if (_pendingRemoveUids.find(old.mcpUid) != _pendingRemoveUids.end())
					{
						_DestroyServer(old);
					}
				}

				// 移除已销毁的servers，保留仍然需要的
				_servers.erase(
					std::remove_if(_servers.begin(), _servers.end(),
						[this](const Server& s) { return _pendingRemoveUids.find(s.mcpUid) != _pendingRemoveUids.end(); }),
					_servers.end());

				// 添加新创建的servers
				for (auto& pending : _pendingServers)
				{
					_servers.push_back(std::move(pending));
				}

				_pendingRemoveUids.clear();
				_pendingServers.clear();
				_syncedVer = _pendingVer;

				isServerModified = true;
			}
			else
			{
				// 版本已变化，丢弃 _pendingServers
				for (auto& pending : _pendingServers)
				{
					_DestroyServer(pending);
				}
				_pendingServers.clear();
			}
			// _NeedSync() 会返回 true，重新启动 worker
		}
		return isServerModified;
	}

	// 2. 检查是否需要同步
	if (!_NeedSync())
		return false;

	// 3. 启动后台同步线程
	_StartWorker();

	return false;
}

void CLlmMcpServers::_StartWorker()
{
	_workerRunning = true;
	_workerDone = false;
	_pendingVer = g_llmMcps._ver;

	// 收集现有servers的信息（uid 和 fileTime）
	std::vector<std::pair<WUID, FILETIME>> existingServers;
	for (const auto& s : _servers)
	{
		existingServers.push_back({ s.mcpUid, s.fileTime });
	}

	_workerThread = std::thread(&CLlmMcpServers::_WorkerFunc, this, _pendingVer, std::move(existingServers));
}

void CLlmMcpServers::_WorkerFunc(int targetVer, const std::vector<std::pair<WUID, FILETIME>>& existingServers)
{
	// 1. 收集目标 servers
	std::vector<Server> targetServers;
	_CollectTargetServers(targetServers);

	// 2. 构建需要移除的server集合（existingServers中不在targetServers里的，或fileTime变化的）
	std::unordered_set<WUID> targetUids;
	for (const auto& target : targetServers)
		targetUids.insert(target.mcpUid);

	std::unordered_set<WUID> removeUids;
	for (const auto& existing : existingServers)
	{
		auto it = targetUids.find(existing.first);
		if (it == targetUids.end())
		{
			// 不在目标列表中，需要移除
			removeUids.insert(existing.first);
		}
	}

	// 3. 只创建需要新创建的servers（uid不存在 或 fileTime变化）
	std::vector<Server> newServers;
	for (const auto& target : targetServers)
	{
		// 检查是否已存在且fileTime未变化
		bool needCreate = true;
		for (const auto& existing : existingServers)
		{
			if (existing.first == target.mcpUid)
			{
				// uid 匹配，检查 fileTime
				if (CompareFileTime(&existing.second, &target.fileTime) == 0)
				{
					// fileTime 未变化，不需要重新创建
					needCreate = false;
				}
				else
				{
					// fileTime 已变化，需要移除旧的并重建
					removeUids.insert(target.mcpUid);
				}
				break;
			}
		}

		if (needCreate)
		{
			// 需要创建新的server
			Server newServer;
			newServer.mcpUid = target.mcpUid;
			newServer.command = target.command;
			newServer.args = target.args;
			newServer.env = target.env;
			newServer.fileTime = target.fileTime;

			_CreateServer(newServer);  // 无论成功失败都添加，以便回填错误信息
			newServers.push_back(std::move(newServer));
		}
	}

	// 4. 更新结果
	{
		std::lock_guard<std::mutex> lock(_mutex);
		_pendingServers = std::move(newServers);
		_pendingRemoveUids = std::move(removeUids);
		_workerDone = true;
	}
}

bool CLlmMcpServers::_CreateServer(Server& server)
{
	// 1. 创建管道用于与子进程通信
	SECURITY_ATTRIBUTES sa;
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = nullptr;

	HANDLE hChildStdInRead = nullptr;
	HANDLE hChildStdInWrite = nullptr;
	HANDLE hChildStdOutRead = nullptr;
	HANDLE hChildStdOutWrite = nullptr;

	// 创建标准输入管道
	if (!CreatePipe(&hChildStdInRead, &hChildStdInWrite, &sa, 0))
	{
		server.AppendOutput("Failed to create stdin pipe");
		return false;
	}

	// 创建标准输出管道
	if (!CreatePipe(&hChildStdOutRead, &hChildStdOutWrite, &sa, 0))
	{
		CloseHandle(hChildStdInRead);
		CloseHandle(hChildStdInWrite);
		server.AppendOutput("Failed to create stdout pipe");
		return false;
	}

	// 确保父进程不继承子进程端的句柄
	if (!SetHandleInformation(hChildStdInWrite, HANDLE_FLAG_INHERIT, 0))
	{
		CloseHandle(hChildStdInRead);
		CloseHandle(hChildStdInWrite);
		CloseHandle(hChildStdOutRead);
		CloseHandle(hChildStdOutWrite);
		server.AppendOutput("Failed to set handle information for stdin");
		return false;
	}

	if (!SetHandleInformation(hChildStdOutRead, HANDLE_FLAG_INHERIT, 0))
	{
		CloseHandle(hChildStdInRead);
		CloseHandle(hChildStdInWrite);
		CloseHandle(hChildStdOutRead);
		CloseHandle(hChildStdOutWrite);
		server.AppendOutput("Failed to set handle information for stdout");
		return false;
	}

	// 2. 构建命令行
	// 注意: 不能无差别地给每个参数加引号。例如 cmd 的开关 "/c" 一旦被引号
	// 包裹成 "/c", cmd.exe 就不会将其识别为开关, 导致后续命令解析错乱
	// (npx/npm 会推算出错误的 prefix 路径并崩溃)。
	// 因此仅对包含空格或为空的参数加引号。
	std::string cmdLine = server.command;
	for (const auto& arg : server.args)
	{
		cmdLine += " ";
		cmdLine += arg;
	}

	// 3. 构建环境块
	// 如果 server.env 有自定义环境变量，需要创建自定义环境块
	std::wstring envBlock;
	LPVOID lpEnv = NULL;
	if (!server.env.empty())
	{
		// 获取当前进程的环境块
		LPWCH currentEnv = GetEnvironmentStringsW();
		if (currentEnv)
		{
			// 解析当前环境到 map（key 大小写不敏感）
			std::unordered_map<std::wstring, std::wstring> envMap;
			LPWCH p = currentEnv;
			while (*p)
			{
				std::wstring entry(p);
				size_t eqPos = entry.find(L'=');
				if (eqPos != std::wstring::npos && eqPos > 0)
				{
					std::wstring key = entry.substr(0, eqPos);
					std::wstring value = entry.substr(eqPos + 1);
					envMap[key] = value;
				}
				p += entry.length() + 1;
			}
			FreeEnvironmentStringsW(currentEnv);

			// 合入 server.env（覆盖同名变量）
			for (const auto& kv : server.env)
			{
				envMap[utf8_to_widechar(kv.first)] = utf8_to_widechar(kv.second);
			}

			// 序列化为环境块（格式: NAME=VALUE\0...最后额外一个\0）
			for (const auto& kv : envMap)
			{
				envBlock += kv.first + L'=' + kv.second + L'\0';
			}
			envBlock += L'\0';  // 结尾额外的 \0
			lpEnv = (LPVOID)envBlock.c_str();
		}
	}

	// 4. 设置启动信息
	STARTUPINFOW si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.hStdError = hChildStdOutWrite;
	si.hStdOutput = hChildStdOutWrite;
	si.hStdInput = hChildStdInRead;
	si.dwFlags |= STARTF_USESTDHANDLES;

	ZeroMemory(&pi, sizeof(pi));

	// 命令行转为宽字符
	std::wstring wCmdLine = utf8_to_widechar(cmdLine);

	// 5. 创建进程
	if (!CreateProcessW(
		NULL,                           // 应用程序名称
		(LPWSTR)wCmdLine.c_str(),       // 命令行
		NULL,                           // 进程安全属性
		NULL,                           // 线程安全属性
		TRUE,                           // 继承句柄
		CREATE_NO_WINDOW | CREATE_UNICODE_ENVIRONMENT,  // 创建标志（宽字符环境块需要此标志）
		lpEnv,                          // 环境块（NULL 表示继承父进程环境）
		NULL,                           // 当前目录
		&si,                            // 启动信息
		&pi))                           // 进程信息
	{
		CloseHandle(hChildStdInRead);
		CloseHandle(hChildStdInWrite);
		CloseHandle(hChildStdOutRead);
		CloseHandle(hChildStdOutWrite);
		server.AppendOutput("Failed to create process: " + cmdLine);
		return false;
	}

	// 6. 保存句柄
	server.hProcess = pi.hProcess;
	server.hThread = pi.hThread;

	// 关闭子进程端的句柄（子进程已经持有）
	CloseHandle(hChildStdInRead);
	CloseHandle(hChildStdOutWrite);

	// 保存通信管道
	server.hPipeWrite = hChildStdInWrite;
	server.hPipeRead = hChildStdOutRead;

	// 7. 发送MCP初始化请求
	if (!_SendMcpInitialize(server))
	{
		std::string pipeOutput = _FlushPipeRead(server);
		if (!pipeOutput.empty())
			server.AppendOutput("Process output: " + pipeOutput);
		_DestroyServer(server);
		return false;
	}

	// 8. 获取工具列表
	if (!_FetchTools(server))
	{
		std::string pipeOutput = _FlushPipeRead(server);
		if (!pipeOutput.empty())
			server.AppendOutput("Process output: " + pipeOutput);
		_DestroyServer(server);
		return false;
	}

	server.toolsFetched = true;
//	server.output.clear();  // 成功时清空输出信息
	return true;
}

std::string CLlmMcpServers::_FlushPipeRead(Server& server)
{
	std::string result;
	if (!server.hPipeRead) return result;

	char buffer[4096];
	DWORD bytesRead;
	DWORD available = 0;
	DWORD lastReadTime = GetTickCount();
	const DWORD waitMs = 500;  // 持续无输出的等待时间

	while (GetTickCount() - lastReadTime < waitMs)
	{
		available = 0;
		if (PeekNamedPipe(server.hPipeRead, NULL, 0, NULL, &available, NULL) && available > 0)
		{
			if (!ReadFile(server.hPipeRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL) || bytesRead == 0)
				break;
			result.append(buffer, bytesRead);
			lastReadTime = GetTickCount();  // 有数据则重置计时
		}
		else
		{
			Sleep(10);
		}
	}

	return result;
}

void CLlmMcpServers::_DestroyServer(Server& server)
{
	// 关闭管道
	if (server.hPipeRead)
	{
		CloseHandle(server.hPipeRead);
		server.hPipeRead = nullptr;
	}
	if (server.hPipeWrite)
	{
		CloseHandle(server.hPipeWrite);
		server.hPipeWrite = nullptr;
	}

	// 终止进程
	if (server.hProcess)
	{
		TerminateProcess(server.hProcess, 0);
		CloseHandle(server.hProcess);
		server.hProcess = nullptr;
	}

	// 关闭线程句柄
	if (server.hThread)
	{
		CloseHandle(server.hThread);
		server.hThread = nullptr;
	}

	server.toolsFetched = false;
	server.tools.clear();
}

bool CLlmMcpServers::_SendMcpRequest(Server& server, const std::string& request, std::string& response, int timeoutMs, bool outputResponse)
{
	if (!server.hPipeWrite || !server.hPipeRead)
		return false;

	// 解析请求中的 id，用于匹配响应（MCP STDIO 允许 server 主动推送无 id 的通知）
	long long expectId = -1;
	bool hasExpectId = false;
	try
	{
		auto reqJson = nlohmann::json::parse(request);
		if (reqJson.contains("id") && reqJson["id"].is_number_integer())
		{
			expectId = reqJson["id"].get<long long>();
			hasExpectId = true;
		}
	}
	catch (...)
	{
	}

	// 构建MCP消息（MCP STDIO 使用行分隔 JSON，每条消息单独一行，以 \n 结尾，不带任何头部）
	std::string message = request + "\n";

	// 发送请求
	DWORD bytesWritten;
	if (!WriteFile(server.hPipeWrite, message.c_str(), static_cast<DWORD>(message.length()), &bytesWritten, NULL))
	{
		server.AppendOutput("WriteFile failed, error: " + std::to_string(GetLastError()));
		return false;
	}

	// 读取响应（带超时）
	std::string buffer;
	char readBuffer[4096];
	DWORD bytesRead;

	DWORD startTime = GetTickCount();
	while (true)
	{
		// 检查超时
		if (GetTickCount() - startTime > (DWORD)timeoutMs)
		{
			server.AppendOutput("MCP request timeout");
			return false;
		}

		// 检查是否有数据可读
		DWORD available = 0;
		if (!PeekNamedPipe(server.hPipeRead, NULL, 0, NULL, &available, NULL))
		{
// 			Sleep(10);
// 			continue;
			server.AppendOutput("PeekNamedPipe failed, error: " + std::to_string(GetLastError()));
			return false;
		}

		if (available == 0)
		{
			Sleep(10);
			continue;
		}

		// 读取数据
		if (!ReadFile(server.hPipeRead, readBuffer, sizeof(readBuffer), &bytesRead, NULL))
		{
			server.AppendOutput("ReadFile failed, error: " + std::to_string(GetLastError()));
			return false;
		}

		if (bytesRead == 0)
		{
			Sleep(10);
			continue;
		}

		buffer.append(readBuffer, bytesRead);
		if (outputResponse) server.AppendOutput(std::string(readBuffer, bytesRead));

		// 按行（\n）切分逐条解析（NDJSON）
		size_t lineStart = 0;
		size_t newlinePos;
		while ((newlinePos = buffer.find('\n', lineStart)) != std::string::npos)
		{
			std::string line = buffer.substr(lineStart, newlinePos - lineStart);
			lineStart = newlinePos + 1;

			// 去除可能的 \r 及首尾空白
			while (!line.empty() && (line.back() == '\r' || line.back() == ' ' || line.back() == '\t'))
				line.pop_back();
			if (line.empty())
				continue;

			try
			{
				auto j = nlohmann::json::parse(line);

			// 没有 id 的是 server 主动推送的通知，跳过
				if (!hasExpectId)
				{
					// 调用方不期待特定 id，返回首条合法 JSON 即可
					response = line;
					return true;
				}

				if (j.contains("id") && j["id"].is_number_integer() &&
					j["id"].get<long long>() == expectId)
				{
					response = line;
					return true;
				}
				// 其它消息（如通知、其它 id 的响应）忽略，继续读
			}
			catch (...)
			{
				// 非完整或非法 JSON 行，忽略
			}
		}

		// 移除已处理的完整行，保留尾部不完整数据
		if (lineStart > 0)
			buffer.erase(0, lineStart);
	}

	return false;
}


bool CLlmMcpServers::_SendMcpInitialize(Server& server)
{
	// MCP initialize 请求
	std::string request = R"({"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-11-25","capabilities":{},"clientInfo":{"name":"LazyBug","version":"1.0.0"}}})";

	std::string response;
	if (!_SendMcpRequest(server, request, response, 60000,true))
	{
		return false;
	}

	// 校验 initialize 响应：必须是带 result 的成功响应，不能是 error
	try
	{
		auto j = nlohmann::json::parse(response);
		if (j.contains("error"))
		{
			server.AppendOutput("MCP initialize returned error: " + j["error"].dump());
			return false;
		}
		if (!j.contains("result") || !j["result"].is_object())
		{
			server.AppendOutput("MCP initialize response missing result");
			return false;
		}
	}
	catch (...)
	{
		server.AppendOutput("MCP initialize response parse failed");
		return false;
	}

	// 发送 initialized 通知（NDJSON：单行 JSON + \n，无头部）
	std::string notification = R"({"jsonrpc":"2.0","method":"notifications/initialized"})";
	std::string notifMessage = notification + "\n";

	DWORD bytesWritten;
	if (!WriteFile(server.hPipeWrite, notifMessage.c_str(), static_cast<DWORD>(notifMessage.length()), &bytesWritten, NULL))
	{
		server.AppendOutput("WriteFile (initialized) failed, error: " + std::to_string(GetLastError()));
		return false;
	}

	return true;
}

bool CLlmMcpServers::_FetchTools(Server& server)
{
	// 获取工具列表请求
	std::string request = R"({"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}})";

	std::string response;
	if (!_SendMcpRequest(server, request, response, 5000,true))
	{
		return false;
	}

	// 解析响应
	try
	{
		auto j = nlohmann::json::parse(response);

		if (!j.contains("result") || !j["result"].is_object())
			return false;

		const auto& result = j["result"];
		if (!result.contains("tools") || !result["tools"].is_array())
			return false;

		for (const auto& tool : result["tools"])
		{
			CLlmMcps::Mcp::Tool t;
			if (tool.contains("name") && tool["name"].is_string())
				t.name = tool["name"].get<std::string>();
			if (tool.contains("description") && tool["description"].is_string())
				t.description = tool["description"].get<std::string>();
			if (tool.contains("inputSchema"))
				t.inputSchema = tool["inputSchema"].dump();

			server.tools.push_back(std::move(t));
		}

		return !server.tools.empty();
	}
	catch (...)
	{
		return false;
	}
}

void CLlmMcpServers::LoadToolsToMcps()
{
	// 先清除所有 mcp 的 tools
	for (auto& mcp : g_llmMcps._mcps)
	{
		mcp.tools.clear();
		mcp.toolsLoaded = false;
		mcp.lastError.clear();
	}

	// 回填 tools 到对应的 mcp
	for (auto& server : _servers)
	{
		// 查找对应的 mcp，通过 uid 匹配
		for (auto& mcp : g_llmMcps._mcps)
		{
			if (mcp.uid == server.mcpUid)
			{
				mcp.tools = server.tools;
				mcp.toolsLoaded = server.toolsFetched;
				mcp.lastError = server.output;
				break;
			}
		}
	}
}

void CLlmMcpServers::_CollectTargetServers(std::vector<Server>& outServers)
{
	outServers.clear();

	// 同名 mcp 按优先级去重
	std::unordered_map<std::string, size_t> nameToIndex;

	for (const auto& mcp : g_llmMcps._mcps)
	{
		if (!mcp.enable)
			continue;

		auto it = nameToIndex.find(mcp.name);
		if (it == nameToIndex.end())
		{
			nameToIndex[mcp.name] = outServers.size();
			Server s;
			s.mcpUid = mcp.uid;
			s.command = mcp.command;
			s.args = mcp.args;
			s.env = mcp.env;
			s.fileTime = mcp.fileTime;
			outServers.push_back(std::move(s));
		}
		else
		{
			// 检查优先级
			if (_Priority(mcp.tp) > _Priority(g_llmMcps._mcps[nameToIndex[mcp.name]].tp))
			{
				outServers[it->second].mcpUid = mcp.uid;
				outServers[it->second].command = mcp.command;
				outServers[it->second].args = mcp.args;
				outServers[it->second].env = mcp.env;
				outServers[it->second].fileTime = mcp.fileTime;
			}
		}
	}
}
