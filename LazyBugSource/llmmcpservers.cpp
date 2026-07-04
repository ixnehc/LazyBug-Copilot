#include "stdh.h"
#include "LlmMcpServers.h"
#include "LlmMcps.h"
#include "stringparser/stringparser.h"
#include <algorithm>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <curl/curl.h>

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
	// 请求所有server停止，等待线程退出并销毁
	for (auto& kv : _servers)
	{
		_RequestStop(*kv.second);
	}
	for (auto& s : _removing)
	{
		_RequestStop(*s);
	}
	for (auto& kv : _servers)
	{
		_JoinAndDestroy(*kv.second);
	}
	for (auto& s : _removing)
	{
		_JoinAndDestroy(*s);
	}
	_servers.clear();
	_removing.clear();
}

bool CLlmMcpServers::_NeedSync() const
{
	return _syncedVer != g_llmMcps._ver;
}

bool CLlmMcpServers::UpdateSync()
{
	// 1. 回收已结束线程的server，收集状态变化
	bool isServerModified = _ReapFinished();

	// 2. 版本变化时做增量同步（非阻塞，仅做diff和线程调度）
	if (_NeedSync())
	{
		_Sync();
		isServerModified = true;
	}

	return isServerModified;
}

bool CLlmMcpServers::_ReapFinished()
{
	bool modified = false;

	// 回收迁出到_removing的旧server（已请求停止）
	for (auto it = _removing.begin(); it != _removing.end(); )
	{
		Server& server = **it;
		if (server.done.load())
		{
			_JoinAndDestroy(server);
			it = _removing.erase(it);
			modified = true;
		}
		else
		{
			++it;
		}
	}

	for (auto it = _servers.begin(); it != _servers.end(); )
	{
		Server& server = *it->second;

		if (server.done.load())
		{
			if (server.pendingRemove)
			{
				// 已请求停止：join并销毁，从map移除
				_JoinAndDestroy(server);
				it = _servers.erase(it);
				modified = true;
				continue;
			}
			else if (server.thread.joinable())
			{
				// 线程刚完成创建（成功或失败）：join回收线程，状态视为已变化
				server.thread.join();
				modified = true;
			}
		}
		++it;
	}

	return modified;
}

void CLlmMcpServers::_Sync()
{
	// 1. 收集目标servers
	std::vector<Target> targets;
	_CollectTargetServers(targets);

	std::unordered_set<WUID> targetUids;
	for (const auto& t : targets)
		targetUids.insert(t.mcpUid);

	// 2. 停止不再需要的server（不在目标列表中）
	for (auto& kv : _servers)
	{
		Server& server = *kv.second;
		if (server.pendingRemove)
			continue;
		if (targetUids.find(server.mcpUid) == targetUids.end())
		{
			_RequestStop(server);
		}
	}

	// 3. 处理目标servers：新建或重建
	for (const auto& target : targets)
	{
		auto it = _servers.find(target.mcpUid);
		if (it != _servers.end() && !it->second->pendingRemove)
		{
			// 已存在：fileTime未变化则保留，变化则停止旧的并重建
			if (CompareFileTime(&it->second->fileTime, &target.fileTime) == 0)
				continue;
			_RequestStop(*it->second);
		}

		// 需要创建新的server
		auto server = std::make_unique<Server>();
		server->mcpUid = target.mcpUid;
		server->connect = target.connect;
		server->fileTime = target.fileTime;

		_StartServer(std::move(server));
	}

	_syncedVer = g_llmMcps._ver;
}

void CLlmMcpServers::_StartServer(std::unique_ptr<Server> server)
{
	WUID uid = server->mcpUid;

	// 创建中断事件（手动复位，初始未置位）
	server->hCancelEvent = CreateEvent(nullptr, TRUE, FALSE, nullptr);
	server->state.store(State::Starting);
	server->done.store(false);

	Server* raw = server.get();

	// 若map中已存在该uid（旧的正在停止中），为避免key冲突，
	// 将正在停止的旧server迁出到_removing保管，待其线程退出后由_ReapFinished回收
	auto existing = _servers.find(uid);
	if (existing != _servers.end())
	{
		_RequestStop(*existing->second);
		_removing.push_back(std::move(existing->second));
		_servers.erase(existing);
	}

	raw->thread = std::thread(&CLlmMcpServers::_ServerThreadFunc, this, raw);
	_servers[uid] = std::move(server);
}

void CLlmMcpServers::_ServerThreadFunc(Server* server)
{
	bool ok = _CreateServer(*server, server->hCancelEvent);
	server->state.store(ok ? State::Ready : State::Failed);
	server->toolsFetched = ok;
	server->done.store(true);
}

void CLlmMcpServers::_RequestStop(Server& server)
{
	server.pendingRemove = true;
	if (server.hCancelEvent)
		SetEvent(server.hCancelEvent);
}

void CLlmMcpServers::_JoinAndDestroy(Server& server)
{
	if (server.hCancelEvent)
		SetEvent(server.hCancelEvent);
	if (server.thread.joinable())
		server.thread.join();
	_DestroyServer(server);
	if (server.hCancelEvent)
	{
		CloseHandle(server.hCancelEvent);
		server.hCancelEvent = nullptr;
	}
}


bool CLlmMcpServers::_CreateServer(Server& server, HANDLE hCancel)
{
	// 起步前检查是否已被请求中断
	if (hCancel && WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
	{
		server.AppendOutput("Cancelled before start");
		return false;
	}

	// HTTP 模式：url 非空，跳过进程创建
	if (server.connect.IsHttpMode())
	{
		server.AppendOutput("Using HTTP transport: " + server.connect.url);

		// 发送MCP初始化请求
		if (!_SendMcpInitialize(server, hCancel))
		{
			_DestroyServer(server);
			return false;
		}

		// 获取工具列表
		if (!_FetchTools(server, hCancel))
		{
			_DestroyServer(server);
			return false;
		}

		server.toolsFetched = true;
		return true;
	}

	// stdio 模式：command 非空，创建子进程
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
	std::string cmdLine = server.connect.command;
	for (const auto& arg : server.connect.args)
	{
		cmdLine += " ";
		cmdLine += arg;
	}

	// 3. 构建环境块
	// 如果 server.connect.env 有自定义环境变量，需要创建自定义环境块
	std::wstring envBlock;
	LPVOID lpEnv = NULL;
	if (!server.connect.env.empty())
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

			// 合入 server.connect.env（覆盖同名变量）
			for (const auto& kv : server.connect.env)
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
	if (!_SendMcpInitialize(server, hCancel))
	{
		std::string pipeOutput = _FlushPipeRead(server, hCancel);
		if (!pipeOutput.empty())
			server.AppendOutput("Process output: " + pipeOutput);
		_DestroyServer(server);
		return false;
	}

	// 8. 获取工具列表
	if (!_FetchTools(server, hCancel))
	{
		std::string pipeOutput = _FlushPipeRead(server, hCancel);
		if (!pipeOutput.empty())
			server.AppendOutput("Process output: " + pipeOutput);
		_DestroyServer(server);
		return false;
	}

	server.toolsFetched = true;
//	server.output.clear();  // 成功时清空输出信息
	return true;
}

std::string CLlmMcpServers::_FlushPipeRead(Server& server, HANDLE hCancel)
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
		// 被请求中断则立即返回
		if (hCancel && WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
			break;

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

bool CLlmMcpServers::_SendMcpRequest(Server& server, HANDLE hCancel, const std::string& request, std::string& response, int timeoutMs, bool outputResponse)
{
	// HTTP 模式
	if (server.connect.IsHttpMode())
	{
		return _SendMcpRequestHttp(server, hCancel, request, response, timeoutMs, outputResponse);
	}

	// stdio 模式
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
		// 被请求中断则立即放弃
		if (hCancel && WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
		{
			server.AppendOutput("MCP request cancelled");
			return false;
		}

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
			// 用cancel事件等待，被置位则下一轮循环立即退出
			if (hCancel)
				WaitForSingleObject(hCancel, 10);
			else
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

// curl 写入回调，收集响应数据
static size_t _McpHttpWriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
{
	size_t totalSize = size * nmemb;
	std::string* str = static_cast<std::string*>(userp);
	str->append(static_cast<char*>(contents), totalSize);
	return totalSize;
}

// curl 进度回调，检查取消事件
static int _McpHttpProgressCallback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
	HANDLE hCancel = static_cast<HANDLE>(clientp);
	if (hCancel && WaitForSingleObject(hCancel, 0) == WAIT_OBJECT_0)
		return 1;  // 返回非零值中止传输
	return 0;
}

bool CLlmMcpServers::_SendMcpRequestHttp(Server& server, HANDLE hCancel, const std::string& request, std::string& response, int timeoutMs, bool outputResponse)
{
	// 初始化 curl
	CURL* curl = curl_easy_init();
	if (!curl)
	{
		server.AppendOutput("Failed to initialize CURL for MCP HTTP");
		return false;
	}

	// 设置请求头
	struct curl_slist* headers = nullptr;
	headers = curl_slist_append(headers, "Content-Type: application/json");
	headers = curl_slist_append(headers, "Accept: application/json, text/event-stream");

	// 解析请求中的 id，用于匹配响应
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

	// 收集响应数据
	std::string responseData;

	// 设置 curl 选项
	curl_easy_setopt(curl, CURLOPT_URL, server.connect.url.c_str());
	curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
	curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, _McpHttpWriteCallback);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);
	curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0L);
	curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, _McpHttpProgressCallback);
	curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, hCancel);
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeoutMs / 1000);

	// 发送请求
	CURLcode res = curl_easy_perform(curl);

	// 清理
	curl_slist_free_all(headers);
	curl_easy_cleanup(curl);

	// 检查结果
	if (res != CURLE_OK)
	{
		if (res == CURLE_ABORTED_BY_CALLBACK)
			server.AppendOutput("MCP HTTP request cancelled");
		else
			server.AppendOutput("MCP HTTP request failed: " + std::string(curl_easy_strerror(res)));
		return false;
	}

	if (outputResponse)
		server.AppendOutput(responseData);

	// 解析响应（可能是 NDJSON 或 SSE 格式）
	// SSE 格式：以空行分隔事件，事件内 "id:" / "event:" / "data:" 等字段行
	//   同一事件可有多个 "data:" 行，其值用 '\n' 拼接成完整消息
	// 这里用偏移量做零拷贝切分，避免对巨大响应反复 substr/erase 拷贝
	// 每个事件的 data 行只记录 [起始, 长度]，需要处理时才拼接

	const char* buf = responseData.data();
	const size_t total = responseData.size();

	// 处理一条完整消息，命中返回 true 并写入 response
	// 单行时直接用 responseData 内部数据传给 parse（零拷贝）
	// 多行时拼接 '\n' 后再 parse
	auto tryHandleEvent = [&](const std::vector<std::pair<size_t, size_t>>& dataLines, bool& matched) -> void
	{
		matched = false;
		if (dataLines.empty())
			return;

		if (dataLines.size() == 1)
		{
			// 单行：直接用指向 responseData 的裸数据
			size_t beg = dataLines[0].first;
			size_t len = dataLines[0].second;
			try
			{
				auto j = nlohmann::json::parse(buf + beg, buf + beg + len);

				if (!hasExpectId)
				{
					response.assign(buf + beg, len);
					matched = true;
					return;
				}
				if (j.contains("id") && j["id"].is_number_integer() &&
					j["id"].get<long long>() == expectId)
				{
					response.assign(buf + beg, len);
					matched = true;
				}
			}
			catch (...)
			{
			}
		}
		else
		{
			// 多行：按 SSE 规范用 '\n' 拼接
			std::string msg;
			for (size_t i = 0; i < dataLines.size(); ++i)
			{
				if (i > 0)
					msg.push_back('\n');
				msg.append(buf + dataLines[i].first, dataLines[i].second);
			}
			try
			{
				auto j = nlohmann::json::parse(msg);

				if (!hasExpectId)
				{
					response = msg;
					matched = true;
					return;
				}
				if (j.contains("id") && j["id"].is_number_integer() &&
					j["id"].get<long long>() == expectId)
				{
					response = msg;
					matched = true;
				}
			}
			catch (...)
			{
			}
		}
	};

	// 当前事件的 data 行偏移列表（[起始偏移, 长度]）
	std::vector<std::pair<size_t, size_t>> dataLines;

	size_t lineStart = 0;
	while (lineStart <= total)
	{
		size_t newlinePos = responseData.find('\n', lineStart);
		size_t lineEnd = (newlinePos == std::string::npos) ? total : newlinePos;

		// 当前行范围 [lineBeg, lineLen)，指向 responseData 内部，零拷贝
		size_t lineBeg = lineStart;
		size_t lineLen = lineEnd - lineStart;
		lineStart = lineEnd + 1;

		// 去除行尾的 \r 及空白（仅调整长度，零拷贝）
		while (lineLen > 0)
		{
			char c = buf[lineBeg + lineLen - 1];
			if (c == '\r' || c == ' ' || c == '\t')
				--lineLen;
			else
				break;
		}

		// 空行：事件结束，处理累积的 data
		if (lineLen == 0)
		{
			if (!dataLines.empty())
			{
				bool matched = false;
				tryHandleEvent(dataLines, matched);
				if (matched)
					return true;
				dataLines.clear();
			}
			if (newlinePos == std::string::npos)
				break;
			continue;
		}

		// data: 行 —— 记录值在 responseData 中的偏移和长度
		if (lineLen >= 5 && memcmp(buf + lineBeg, "data:", 5) == 0)
		{
			lineBeg += 5;
			lineLen -= 5;
			// 去除 data: 后可能的前导空格（宽松处理，跳过所有空白）
			while (lineLen > 0 && (buf[lineBeg] == ' ' || buf[lineBeg] == '\t'))
			{
				++lineBeg;
				--lineLen;
			}
			dataLines.push_back(std::make_pair(lineBeg, lineLen));
		}
		// 其它 SSE 字段行（id:/event: 等）跳过

		// 末尾无换行的最后一行已处理完毕
		if (newlinePos == std::string::npos)
			break;
	}

	// 流末尾未以空行结束时，处理最后累积的事件
	if (!dataLines.empty())
	{
		bool matched = false;
		tryHandleEvent(dataLines, matched);
		if (matched)
			return true;
	}

	server.AppendOutput("MCP HTTP response parse failed: no matching response");
	return false;
}


bool CLlmMcpServers::_SendMcpInitialize(Server& server, HANDLE hCancel)
{
	// MCP initialize 请求
	std::string request = R"({"jsonrpc":"2.0","id":1,"method":"initialize","params":{"protocolVersion":"2025-11-25","capabilities":{},"clientInfo":{"name":"LazyBug","version":"1.0.0"}}})";

	std::string response;
	if (!_SendMcpRequest(server, hCancel, request, response, 600000,true))
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

	// 发送 initialized 通知
	// 对于 HTTP 模式，通知不需要等待响应；对于 stdio 模式，使用 WriteFile
	std::string notification = R"({"jsonrpc":"2.0","method":"notifications/initialized"})";

	if (server.connect.IsHttpMode())
	{
		// HTTP 模式：发送通知但不等待响应
		std::string dummyResponse;
		_SendMcpRequestHttp(server, hCancel, notification, dummyResponse, 5000, false);
		// 通知不需要响应，忽略错误
	}
	else
	{
		// stdio 模式
		std::string notifMessage = notification + "\n";
		DWORD bytesWritten;
		if (!WriteFile(server.hPipeWrite, notifMessage.c_str(), static_cast<DWORD>(notifMessage.length()), &bytesWritten, NULL))
		{
			server.AppendOutput("WriteFile (initialized) failed, error: " + std::to_string(GetLastError()));
			return false;
		}
	}

	return true;
}

bool CLlmMcpServers::_FetchTools(Server& server, HANDLE hCancel)
{
	// 获取工具列表请求
	std::string request = R"({"jsonrpc":"2.0","id":2,"method":"tools/list","params":{}})";

	std::string response;
	if (!_SendMcpRequest(server, hCancel, request, response, 5000,true))
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
	for (auto& kv : _servers)
	{
		Server& server = *kv.second;
		// 正在停止的server不回填
		if (server.pendingRemove)
			continue;

		// 查找对应的 mcp，通过 uid 匹配
		for (auto& mcp : g_llmMcps._mcps)
		{
			if (mcp.uid == server.mcpUid)
			{
				State st = server.state.load();
				if (st == State::Ready)
				{
					mcp.tools = server.tools;
					mcp.toolsLoaded = true;
					mcp.lastError = server.output;
				}
				else if (st == State::Failed)
				{
					mcp.toolsLoaded = false;
					mcp.lastError = server.output;
				}
				else // Starting
				{
					mcp.toolsLoaded = false;
					mcp.lastError = "";
				}
				break;
			}
		}
	}
}

void CLlmMcpServers::_CollectTargetServers(std::vector<Target>& outTargets)
{
	outTargets.clear();

	// 收集所有启用的 mcp（不去重）
	for (const auto& mcp : g_llmMcps._mcps)
	{
		if (!mcp.enable)
			continue;

		Target t;
		t.mcpUid = mcp.uid;
		t.connect = mcp.connect;
		t.fileTime = mcp.fileTime;
		outTargets.push_back(std::move(t));
	}
}

CLlmMcpServers::Server* CLlmMcpServers::_FindServerByToolName(const std::string& toolName)
{
	for (auto& kv : _servers)
	{
		Server& server = *kv.second;
		if (server.pendingRemove)
			continue;
		if (server.state.load() != State::Ready)
			continue;
		for (const auto& tool : server.tools)
		{
			if (tool.name == toolName)
				return &server;
		}
	}
	return nullptr;
}

bool CLlmMcpServers::CallTool(const std::string& mcpName, const std::string& arguments, std::string& result, HANDLE hCancel, int timeoutMs)
{
	// 通过别名查找真实的MCP和tool名称
	const auto* aliasInfo = g_llmMcps.FindToolByAlias(mcpName);
	if (!aliasInfo)
	{
		result = "Error: Tool alias not found '" + mcpName + "'";
		return false;
	}

	// 通过mcpUid查找对应的server
	auto it = _servers.find(aliasInfo->mcpUid);
	if (it == _servers.end() || it->second->state.load() != State::Ready || it->second->pendingRemove)
	{
		result = "Error: MCP server not ready for tool '" + mcpName + "' (uid=" + std::to_string(aliasInfo->mcpUid) + ")";
		return false;
	}
	Server* server = it->second.get();

	// 生成唯一请求ID
	long long requestId = _nextRequestId.fetch_add(1);

	// 构建tools/call请求（使用真实tool名称）
	json params = json::object();
	params["name"] = aliasInfo->realToolName;

	// 解析arguments为JSON对象
	if (!arguments.empty() && json::accept(arguments))
		params["arguments"] = json::parse(arguments);
	else
		params["arguments"] = json::object();

	json request = json{
		{"jsonrpc", "2.0"},
		{"id", requestId},
		{"method", "tools/call"},
		{"params", params}
	};

	std::string response;
	if (!_SendMcpRequest(*server, hCancel, request.dump(), response, timeoutMs, false))
	{
		result = "Error: MCP tools/call request failed (timeout, cancelled, or communication error)";
		return false;
	}

	// 解析响应
	try
	{
		auto j = json::parse(response);
		if (j.contains("error"))
		{
			result = "Error: " + j["error"].dump();
			return false;
		}
		if (j.contains("result"))
		{
			// MCP tools/call 的 result 通常是 {"content":[{"type":"text","text":"..."}]}
			// 直接返回 result 的 dump, 由调用方解析
			result = j["result"].dump();
			return true;
		}
		result = "Error: MCP response missing result";
		return false;
	}
	catch (...)
	{
		result = "Error: MCP response parse failed";
		return false;
	}
}
