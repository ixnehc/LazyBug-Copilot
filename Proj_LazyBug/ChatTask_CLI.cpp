#include "stdh.h"
#include "ChatTask_CLI.h"
#include "Utils.h"
#include "LlmChat.h"
#include "LlmLib.h"
#include "ChatAgent.h"
#include "ChatOpsCtrl.h"
#include "CliWhitelist.h"
#include "stringparser/stringparser.h"
#include <sstream>
#include <cstdio>
#include <array>

#include <windows.h>
#include <io.h>
#include <fcntl.h>
#define popen _popen
#define pclose _pclose

// 创建临时文件并写入内容，返回文件路径
// 如果成功，返回 true 且 outFilePath 包含完整路径
// prefix: 临时文件前缀（如 "ps" 或 "sh"）
// extension: 文件扩展名（如 ".ps1" 或 ".sh"）
// content: 要写入的文件内容
// 将 Windows 路径转换为 WSL bash 可识别的 POSIX 路径
// 例如: C:\Users\foo\bar.sh -> /mnt/c/Users/foo/bar.sh
static std::string WindowsPathToPosix(const std::string& winPath)
{
	if (winPath.size() < 2)
		return winPath;

	std::string result = winPath;

	// 将所有反斜杠替换为正斜杠
	for (char& c : result)
	{
		if (c == '\\')
			c = '/';
	}

	// 将盘符 "C:/" 转换为 "/mnt/c/"（WSL 风格）
	if (result.size() >= 3 && std::isalpha((unsigned char)result[0]) && result[1] == ':' && result[2] == '/')
	{
		char driveLetter = std::tolower((unsigned char)result[0]);
		result = std::string("/mnt/") + driveLetter + result.substr(2); // 去掉 "C:"，变成 "/mnt/c/..."
	}

	return result;
}

static bool CreateTempScriptFile(const wchar_t* prefix, const wchar_t* extension, const std::string& content, std::wstring& outFilePath)
{
	wchar_t tempPath[MAX_PATH];
	wchar_t tempFileName[MAX_PATH];
	
	if (!GetTempPathW(MAX_PATH, tempPath))
		return false;
	
	if (!GetTempFileNameW(tempPath, prefix, 0, tempFileName))
		return false;
	
	outFilePath = tempFileName;
	
	// 添加指定的扩展名
	std::wstring finalPath = outFilePath + extension;
	if (!MoveFileW(outFilePath.c_str(), finalPath.c_str()))
	{
		DeleteFileW(outFilePath.c_str());
		return false;
	}
	outFilePath = finalPath;
	
	// 写入文件内容
	HANDLE hFile = CreateFileW(outFilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE)
	{
		DeleteFileW(outFilePath.c_str());
		return false;
	}
	
	DWORD written;
	BOOL writeResult = WriteFile(hFile, content.c_str(), (DWORD)content.length(), &written, NULL);
	CloseHandle(hFile);
	
	if (!writeResult || written != content.length())
	{
		DeleteFileW(outFilePath.c_str());
		return false;
	}
	
	return true;
}

CChatTask_CLI::CChatTask_CLI(const std::string& shellType)
{
	_workerThread = nullptr;
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
	_waitingForInput = false;
	_processStarted = false;
	_lastOutputTime = 0;
	_inputAreaShown = false;
	_isPending = false;           // 默认为非 pending 状态
	_executionStarted = false;    // 尚未启动执行
	_shellType = shellType;       // 由外部传入的 shell 类型
	_processHandle = nullptr;
	_hWriteInput = nullptr;
}

CChatTask_CLI::~CChatTask_CLI()
{
	Interrupt();
}

bool CChatTask_CLI::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("CLI"))
		return false;
	return true;
}

void CChatTask_CLI::_Fail()
{
	_status = TaskStatus::Failure;
}

void CChatTask_CLI::_Succeed()
{
	_status = TaskStatus::Success;
}

// === 公用辅助方法实现 ===

void CChatTask_CLI::_SetThreadResult(const std::string& result, const std::string& message, bool success)
{
	std::lock_guard<std::mutex> lock(_resultMutex);
	_threadResult = result;
	_threadMessage = message;
	_threadSuccess = success;
	_threadFinished = true;
}

std::string CChatTask_CLI::_GetThreadResult()
{
	std::lock_guard<std::mutex> lock(_resultMutex);
	return _threadResult;
}

void CChatTask_CLI::_TerminateProcessAndCleanupHandles()
{
	std::lock_guard<std::mutex> lock(_resultMutex);
	if (_processHandle)
	{
		TerminateProcess(_processHandle, 1);
		_processHandle = NULL;
	}
	if (_hWriteInput)
	{
		CloseHandle(_hWriteInput);
		_hWriteInput = NULL;
	}
}

void CChatTask_CLI::_CleanupWorkerThread()
{
	if (_workerThread && _workerThread->joinable())
	{
		_workerThread->join();
	}
	if (_workerThread)
	{
		delete _workerThread;
		_workerThread = nullptr;
	}
}

void CChatTask_CLI::_CompleteCliAndCleanup(int exitCode)
{
	if (_context && _context->chatOpsCtrl)
	{
		_context->chatOpsCtrl->CompleteCliDisplay(_cliId, exitCode);
	}
	if (_context && _context->chatUi && !_cliId.empty())
	{
		_context->chatUi->RemovePendingCli(_cliId);
	}
}

void CChatTask_CLI::_AppendOutputToDisplay(const std::string& output)
{
	if (_context && _context->chatOpsCtrl && _context->chatAgent)
	{
		std::wstring messageId = _context->chatAgent->GetCurrentAIMessageId();
		_context->chatOpsCtrl->AppendOutputToLastCliDisplay(messageId, output);
	}
}

void CChatTask_CLI::_ThreadFunc()
{
	// 获取命令参数
	std::string command;
	if (!_toolCall.GetStringParam("command", command))
	{
		_SetThreadResult("Error: Missing command parameter", "", false);
		return;
	}

	// 使用成员变量 _shellType（已在 Start() 中初始化）
	const std::string& shellType = _shellType;

	// 获取工作目录（可选）
	std::string workingDir;
	if (_toolCall.ExistParam("workingDir"))
	{
		_toolCall.GetStringParam("workingDir", workingDir);
	}

	// 获取超时时间（可选，默认30秒）
	int timeout = 30000;
	if (_toolCall.ExistParam("timeout"))
	{
		_toolCall.GetIntParam("timeout", timeout);
		if (timeout < 1000)
			timeout = 1000; // 最小1秒
		else if (timeout > 300000)
			timeout = 300000; // 最大5分钟
	}

	// 检查是否被中断
	if (_shouldStop)
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Task interrupted";
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}

	// 执行命令 - Windows 平台使用 CreateProcess
	std::string output;
	int exitCode = 0;
	bool processCreated = false;

	// 创建管道用于重定向输出和输入
	SECURITY_ATTRIBUTES saAttr;
	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL;

	HANDLE hReadPipe = NULL;
	HANDLE hWritePipe = NULL;
	HANDLE hReadInputPipe = NULL;  // 用于读取发送给子进程的输入

	// 创建输出管道
	if (!CreatePipe(&hReadPipe, &hWritePipe, &saAttr, 0))
	{
		_SetThreadResult("Error: Failed to create output pipe", "", false);
		return;
	}

	// 确保输出管道读取端不被继承
	SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

	// 创建输入管道
	if (!CreatePipe(&hReadInputPipe, &_hWriteInput, &saAttr, 0))
	{
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
		_SetThreadResult("Error: Failed to create input pipe", "", false);
		return;
	}

	// 确保输入管道写入端不被继承
	SetHandleInformation(_hWriteInput, HANDLE_FLAG_INHERIT, 0);

	// 设置启动信息
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	si.hStdOutput = hWritePipe;
	si.hStdError = hWritePipe;
	si.hStdInput = hReadInputPipe;  // 设置标准输入
	si.wShowWindow = SW_HIDE;

	ZeroMemory(&pi, sizeof(pi));

	// 临时文件路径（如果需要）
	std::wstring tempFilePath;
	bool useTempFile = false;

	// 根据 shell 类型构建命令行
	std::string cmdLine;
	if (shellType == "powershell.exe")
	{
		// 将命令写入临时文件并执行
		if (CreateTempScriptFile(L"ps", L".ps1", command, tempFilePath))
		{
			useTempFile = true;
			cmdLine = "powershell.exe -ExecutionPolicy Bypass -File \"" + widechar_to_utf8(tempFilePath.c_str()) + "\"";
		}
		else
		{
			cmdLine = "powershell.exe -Command " + command;
		}
	}
	else if (shellType == "bash.exe")
	{
		// 将命令写入临时 shell 脚本并执行
		// 去除可能存在的 \r，确保 bash 可正确解析（WSL bash 不接受 CRLF 换行）
		std::string cleanCommand = command;
		cleanCommand.erase(std::remove(cleanCommand.begin(), cleanCommand.end(), '\r'), cleanCommand.end());
		std::string scriptContent = "#!/bin/bash\n" + cleanCommand + "\n";
		if (CreateTempScriptFile(L"sh", L".sh", scriptContent, tempFilePath))
		{
			useTempFile = true;
			// bash.exe 是 WSL 环境，不认识 Windows 路径，需要转换为 /mnt/c/ 风格的 POSIX 路径
			std::string posixPath = WindowsPathToPosix(widechar_to_utf8(tempFilePath.c_str()));
			cmdLine = "bash.exe \"" + posixPath + "\"";
		}
		else
		{
			cmdLine = "bash.exe -c \"" + command + "\"";
		}
	}
	else if (shellType == "python.exe")
	{
		// 将脚本内容写入临时 .py 文件并执行
		if (CreateTempScriptFile(L"py", L".py", command, tempFilePath))
		{
			useTempFile = true;
			cmdLine = "cmd.exe /c set PYTHONIOENCODING=utf8 && python.exe \"" + widechar_to_utf8(tempFilePath.c_str()) + "\"";
		}
		else
		{
			cmdLine = "cmd.exe /c python.exe -c \"" + command + "\"";
		}
	}
	else
	{
		// cmd.exe - 检查是否有多行命令
		if (command.find('\n') != std::string::npos)
		{
			_SetThreadResult("Error: cmd.exe does not support multi-line commands", "", false);
			return;
		}
		cmdLine = "cmd.exe /c " + command;
	}
	char* pCmdLine = &cmdLine[0];

	// 获取工作目录
	const char* pWorkingDir = workingDir.empty() ? NULL : workingDir.c_str();

	// 创建进程
	if (!CreateProcessA(
		NULL,           // 应用程序名（使用命令行）
		pCmdLine,       // 命令行
		NULL,           // 进程安全属性
		NULL,           // 线程安全属性
		TRUE,           // 继承句柄
		CREATE_NO_WINDOW, // 不创建控制台窗口
		NULL,           // 使用父进程的环境变量
		pWorkingDir,    // 工作目录
		&si,            // 启动信息
		&pi))           // 进程信息
	{
		// 创建进程失败
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
		CloseHandle(hReadInputPipe);
		CloseHandle(_hWriteInput);
		_hWriteInput = NULL;
		
		// 清理临时文件
		if (useTempFile && !tempFilePath.empty())
		{
			DeleteFileW(tempFilePath.c_str());
		}

		_SetThreadResult("Error: Failed to execute command: '" + command + "'", "", false);
		return;
	}

	// 保存进程句柄用于中断
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_processHandle = pi.hProcess;
	}

	processCreated = true;
	
	// 设置进程已启动标志（在 Update 中显示输入框）
	_processStarted = true;

	// 关闭写入端（子进程已经有了一份）
	CloseHandle(hWritePipe);
	hWritePipe = NULL;
	
	// 关闭读取输入端（子进程已经有了一份）
	CloseHandle(hReadInputPipe);
	hReadInputPipe = NULL;

	// 读取输出
	char buffer[128];
	DWORD bytesRead;
	while (true)
	{
		if (_shouldStop)
		{
			// 终止进程
			if (_processHandle)
			{
				TerminateProcess(_processHandle, 1);
			}
			
			CloseHandle(hReadPipe);
			if (_hWriteInput)
			{
				CloseHandle(_hWriteInput);
				_hWriteInput = NULL;
			}
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
			
			{
				std::lock_guard<std::mutex> lock(_resultMutex);
				_processHandle = NULL;
			}
			
			// 清理临时文件
			if (useTempFile && !tempFilePath.empty())
			{
				DeleteFileW(tempFilePath.c_str());
			}
			
			_SetThreadResult("Task interrupted", "", false);
			return;
		}

		// 检查是否有输入要发送（从 ChatUi 获取）
		if (_context && _context->chatUi && _hWriteInput)
		{
			std::wstring wInput;
			if (_context->chatUi->GetCliInput(_cliId, wInput))
			{
				std::string input = widechar_to_utf8(wInput.c_str());
				input += "\n";  // 添加换行符
				DWORD bytesWritten;
				WriteFile(_hWriteInput, input.c_str(), (DWORD)input.length(), &bytesWritten, NULL);
				_waitingForInput = false;  // 输入已发送，不再等待
			}
		}

		DWORD dwAvailable = 0;
		if (!PeekNamedPipe(hReadPipe, NULL, 0, NULL, &dwAvailable, NULL))
		{
			// 管道已关闭或出错
			break;
		}

		if (dwAvailable == 0)
		{
			// 没有数据，检查进程是否仍在运行
			DWORD exitCode;
			if (GetExitCodeProcess(pi.hProcess, &exitCode) && exitCode != STILL_ACTIVE)
			{
				// 进程已结束
				break;
			}
			
			// 短暂休眠避免忙等待
			Sleep(10);
			continue;
		}

		// 有数据可读
		if (ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL) && bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			
			// 将增量输出添加到队列
			{
				std::lock_guard<std::mutex> lock(_outputMutex);
				_outputChunks.push_back(std::string(buffer, bytesRead));
			}
			
			output.append(buffer, bytesRead);
			
			// 更新最后输出时间戳
			_lastOutputTime = GetAbsTick();
			
			// 检测输出中是否包含输入提示符（如 "?"、":" 等）
			// 这里设置一个标志，表示可能需要用户输入
			// 可以根据实际需求改进检测逻辑
			std::string lastChars(buffer, bytesRead);
			if (!lastChars.empty())
			{
				char lastChar = lastChars.back();
				if (lastChar == '?' || lastChar == ':' || lastChar == '>')
				{
					_waitingForInput = true;
				}
			}
		}
		else
		{
			// 读取失败或管道关闭
			break;
		}
	}

	// 关闭管道
	CloseHandle(hReadPipe);
	if (_hWriteInput)
	{
		CloseHandle(_hWriteInput);
		_hWriteInput = NULL;
	}

	// 等待进程结束
	WaitForSingleObject(pi.hProcess, INFINITE);

	// 获取退出码
	DWORD dwExitCode = 0;
	GetExitCodeProcess(pi.hProcess, &dwExitCode);
	exitCode = (int)dwExitCode;

	// 清理进程句柄
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_processHandle = NULL;
		if (_hWriteInput)
		{
			CloseHandle(_hWriteInput);
			_hWriteInput = NULL;
		}
	}
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	// 处理输出编码：尝试判断编码并转换为 UTF-8
	extern std::string local_to_utf8(const std::string& ansi_str);
	
	if (!output.empty())
	{
		if (!Utils::is_valid_utf8(output))
		{
			// 不是有效的 UTF-8，假设是本地编码 (GBK)，转换为 UTF-8
			output = local_to_utf8(output);
		}
	}

	// 构建返回结果
	std::string resultStr;
	std::string messageStr;

	if (exitCode == 0)
	{
		resultStr = output;
		messageStr = "Successfully executed command: \"" + command + "\"";
		_threadSuccess = true;
	}
	else
	{
		resultStr = "Command failed with exit code " + std::to_string(exitCode) + "\nOutput:\n" + output;
		messageStr = "Command failed: \"" + command + "\" (exit code: " + std::to_string(exitCode) + ")";
		_threadSuccess = false;
	}

	// 清理临时文件
	if (useTempFile && !tempFilePath.empty())
	{
		DeleteFileW(tempFilePath.c_str());
	}

	// 保存结果
	std::lock_guard<std::mutex> lock(_resultMutex);
	_threadResult = resultStr;
	_threadMessage = messageStr;
	_threadFinished = true;
}

void CChatTask_CLI::Start()
{
	_status = TaskStatus::Running;

	// 重置状态
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
	_threadResult.clear();
	_threadMessage.clear();
	_processHandle = NULL;
	_hWriteInput = NULL;
	_waitingForInput = false;
	_lastOutputTime = 0;
	_inputAreaShown = false;
	_executionStarted = false;

	// 检查是否有 pending 参数
	g_cliWhitelist.UpdateReload();
	_isPending = true;

	// 获取命令参数
	std::string command;
	if (_toolCall.GetStringParam("command", command))
	{
		
		// 获取描述参数（可选）
		std::wstring wDesc;
		if (_toolCall.ExistParam("desc"))
		{
			std::string desc;
			_toolCall.GetStringParam("desc", desc);
			wDesc = utf8_to_widechar(desc);
		}

		if (_shellType == "cmd.exe" || _shellType == "bash.exe")
		{
			if (g_cliWhitelist.Check(command.c_str()))
			{
				_isPending = false;
			}
		}
		
		// 创建 CLI display，传递 isPending 参数和 shellType
		if (_context && _context->chatOpsCtrl && _context->chatAgent)
		{
			std::wstring messageId = _context->chatAgent->GetCurrentAIMessageId();
			_cliId = _context->chatOpsCtrl->AddCliDisplay(messageId, command, wDesc, _isPending, _shellType);
		}

		// 如果不是 pending 状态，立即启动执行
		if (!_isPending)
		{
			_StartExecution();
		}

	}
	else
	{
		_SendToolCallResult("Invalid command!");
		_Fail();
	}
}

void CChatTask_CLI::_StartExecution()
{
	if (_executionStarted)
		return;  // 已经启动过了

	_executionStarted = true;

	// 启动工作线程
	_workerThread = new std::thread(&CChatTask_CLI::_ThreadFunc, this);
}

void CChatTask_CLI::Update()
{
	if (_status != TaskStatus::Running)
		return;

	// 如果是 pending 状态，检查状态变化
	if (_isPending && !_executionStarted)
	{
		// 检查 CChatUi 中该 CLI ID 的状态
		if (_context && _context->chatUi)
		{
			CliStatus status = _context->chatUi->GetCliStatus(_cliId);
			
			if (status == CliStatus::Accept)
			{
				// 用户点击播放按钮，开始执行
				_StartExecution();
			}
			else if (status == CliStatus::Reject)
			{
				// 用户点击禁止按钮，发送拒绝结果并取消执行
				_SendToolCallResult("Command execution was rejected by user");
				_AppendOutputToDisplay("Command execution was rejected by user");
				_CompleteCliAndCleanup(-1);
				_status = TaskStatus::Failure;
				return;
			}
			// 如果状态仍然是 Pending，继续等待
		}
		// 还没有被启动，继续等待
		return;
	}

	// 如果执行尚未开始（非 pending 状态下的异常情况），直接返回
	if (!_executionStarted)
		return;

	// 检查用户是否点击了停止按钮
	if (_context && _context->chatUi)
	{
		CliStatus status = _context->chatUi->GetCliStatus(_cliId);
		if (status == CliStatus::Stop)
		{
			// 用户点击停止按钮，终止进程
			_shouldStop = true;
			_TerminateProcessAndCleanupHandles();
			
			// 等待线程结束并清理
			_CleanupWorkerThread();
			
			// 获取之前的输出结果，构建最终结果
			std::string previousResult = _GetThreadResult();
			std::string finalResult = previousResult.empty() 
				? "Command was stopped by user" 
				: previousResult + "\n\nCommand was stopped by user";
			
			// 在输出区域显示停止信息
			_AppendOutputToDisplay("\nCommand was stopped by user");
			
			// 发送工具调用结果（包含之前的输出）
			_SendToolCallResult(finalResult.c_str());
			
			// 完成 CLI 显示并清理状态
			_CompleteCliAndCleanup(-1);
			
			_status = TaskStatus::Failure;
			return;
		}
	}

	// 检查进程是否刚启动，需要初始化时间戳
	if (_processStarted)
	{
		_processStarted = false;  // 重置标志
		_lastOutputTime = GetAbsTick();  // 记录启动时间
	}

	// 处理增量输出
	if (_context && _context->chatOpsCtrl && _context->chatAgent)
	{
		std::vector<std::string> chunksToProcess;
		
		// 从队列中取出所有待处理的输出块
		{
			std::lock_guard<std::mutex> lock(_outputMutex);
			if (!_outputChunks.empty())
			{
				chunksToProcess = std::move(_outputChunks);
				_outputChunks.clear();
			}
		}
		
		// 处理增量输出
		if (!chunksToProcess.empty())
		{
			std::wstring messageId = _context->chatAgent->GetCurrentAIMessageId();
			
			for (const auto& chunk : chunksToProcess)
			{
				if (!chunk.empty())
				{
					_context->chatOpsCtrl->AppendOutputToLastCliDisplay(messageId, chunk);
				}
			}
			
			// 收到新输出，如果输入框已显示，则隐藏它
			if (_inputAreaShown)
			{
				if (_context && _context->chatOpsCtrl)
				{
					_context->chatOpsCtrl->ShowCliInputArea(_cliId, false);
				}
				_inputAreaShown = false;
			}
		}
		
		// 检查是否需要显示输入框（持续1秒无输出）
		__int64 currentTime = GetAbsTick();
		if (!_inputAreaShown && _lastOutputTime > 0 && (currentTime - _lastOutputTime) >= 1000)
		{
			// 超过1秒没有输出，显示输入框
			if (_context && _context->chatOpsCtrl)
			{
				_context->chatOpsCtrl->ShowCliInputArea(_cliId, true);
			}
			_inputAreaShown = true;
		}
	}

	// 检查线程是否完成
	if (_threadFinished)
	{
		// 等待线程结束并清理
		_CleanupWorkerThread();

		// 发送工具调用结果（供LLM使用）
		_SendToolCallResult(_GetThreadResult().c_str());

		// 完成 CLI 显示并清理状态
		int exitCode = _threadSuccess ? 0 : 1;
		_CompleteCliAndCleanup(exitCode);

		// 设置最终状态
		if (_threadSuccess)
			_Succeed();
		else
			_Fail();
	}
}

void CChatTask_CLI::Interrupt()
{
	// 设置停止标志
	_shouldStop = true;

	// 终止进程并清理句柄
	_TerminateProcessAndCleanupHandles();

	// 设置为 None 状态
	if (_context && _context->chatUi && !_cliId.empty())
	{
		_context->chatUi->SetCliStatus(_cliId, CliStatus::None);
	}

	_CompleteCliAndCleanup(-1);

	// 清理工作线程
	_CleanupWorkerThread();

	_status = TaskStatus::Failure;
}
