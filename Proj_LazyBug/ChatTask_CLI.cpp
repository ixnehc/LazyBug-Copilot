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
	: _outputBuffer(16000, 100), _outputBufferSimple(2000, 100)
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

void CChatTask_CLI::_SetThreadResult(const std::string& result, const std::string& resultSimple, const std::string& message, bool success)
{
	std::lock_guard<std::mutex> lock(_resultMutex);
	_threadResult = result;
	_threadResultSimple = resultSimple;
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

extern std::string local_to_utf8(const std::string& ansi_str);

CChatTask_CLI::COutputBuffer::COutputBuffer(size_t headLimit, size_t tailLimit)
	: _headLimit(headLimit), _tailLimit(tailLimit),
	_totalUtf8BytesProcessed(0), _omittedBytesCount(0),
	_dotCounter(0), _dotsPrintedInLine(0),
	_isEncodingDecided(false), _isUtf8(false)
{
}

void CChatTask_CLI::COutputBuffer::Reset()
{
	std::lock_guard<std::mutex> lock(_mutex);
	_totalUtf8BytesProcessed = 0;
	_omittedBytesCount = 0;
	_dotCounter = 0;
	_dotsPrintedInLine = 0;
	_headBuffer.clear();
	_tailBuffer.clear();
	_rawInputBuffer.clear();
	_incrementalBuffer.clear();
	_isEncodingDecided = false;
	_isUtf8 = false;
}

void CChatTask_CLI::COutputBuffer::SetHeadLimit(size_t headLimit)
{
	std::lock_guard<std::mutex> lock(_mutex);
	_headLimit = headLimit;
}

void CChatTask_CLI::COutputBuffer::_ProcessUtf8Data(const std::string& utf8Data)
{
	if (utf8Data.empty()) return;

	size_t currentLen = utf8Data.length();
	std::string toAppendInc;

	for (size_t i = 0; i < currentLen; ++i)
	{
		char c = utf8Data[i];
		_totalUtf8BytesProcessed++;

		if (_totalUtf8BytesProcessed <= _headLimit)
		{
			_headBuffer.push_back(c);
			toAppendInc.push_back(c);
		}
		else
		{
			// 超出部分进入尾部缓冲区
			_tailBuffer.push_back(c);

			// 如果超出则统计 omitted
			if (_totalUtf8BytesProcessed > _headLimit + _tailLimit)
			{
				_omittedBytesCount++;
				_dotCounter++;

				// 保持尾部缓冲区不超过 _tailLimit
				// 但为了不截断 utf8 字符，我们只进行粗略截断，等 Finish() 时再做精确裁剪
				// 也可以逐字符剔除，这里我们简单使用 erase 并在 Finish 修复
				if (_tailBuffer.length() > _tailLimit + 1024)
				{
					_tailBuffer.erase(0, 1024);
				}

				if (_dotCounter >= 1000)
				{
					_dotCounter = 0;
					toAppendInc.push_back('.');
					_dotsPrintedInLine++;
					if (_dotsPrintedInLine >= 10)
					{
						toAppendInc.push_back('\n');
						_dotsPrintedInLine = 0;
					}
				}
			}
		}
	}

	if (!toAppendInc.empty())
	{
		_incrementalBuffer += toAppendInc;
	}
}

void CChatTask_CLI::COutputBuffer::Append(const char* data, size_t length)
{
	std::lock_guard<std::mutex> lock(_mutex);

	_rawInputBuffer.append(data, length);
	std::string processStr;

	size_t i = 0;
	while (i < _rawInputBuffer.length())
	{
		unsigned char c = static_cast<unsigned char>(_rawInputBuffer[i]);

		// 如果还没决定编码，并且遇到了非 ASCII 字符
		if (!_isEncodingDecided && c > 0x7F)
		{
			// 尝试判断是否是合法的 UTF-8 序列起始
			if ((c & 0xE0) == 0xC0) // 2-byte
			{
				if (i + 1 < _rawInputBuffer.length())
				{
					unsigned char next = static_cast<unsigned char>(_rawInputBuffer[i + 1]);
					if ((next & 0xC0) == 0x80)
					{
						_isUtf8 = true;
						_isEncodingDecided = true;
					}
					else
					{
						_isUtf8 = false;
						_isEncodingDecided = true;
					}
				}
				else
				{
					// 数据不够，等待下次 append
					break;
				}
			}
			else if ((c & 0xF0) == 0xE0) // 3-byte
			{
				if (i + 2 < _rawInputBuffer.length())
				{
					unsigned char next1 = static_cast<unsigned char>(_rawInputBuffer[i + 1]);
					unsigned char next2 = static_cast<unsigned char>(_rawInputBuffer[i + 2]);
					if ((next1 & 0xC0) == 0x80 && (next2 & 0xC0) == 0x80)
					{
						_isUtf8 = true;
						_isEncodingDecided = true;
					}
					else
					{
						_isUtf8 = false;
						_isEncodingDecided = true;
					}
				}
				else
				{
					// 数据不够，等待下次 append
					break;
				}
			}
			else if ((c & 0xF8) == 0xF0) // 4-byte
			{
				if (i + 3 < _rawInputBuffer.length())
				{
					unsigned char next1 = static_cast<unsigned char>(_rawInputBuffer[i + 1]);
					unsigned char next2 = static_cast<unsigned char>(_rawInputBuffer[i + 2]);
					unsigned char next3 = static_cast<unsigned char>(_rawInputBuffer[i + 3]);
					if ((next1 & 0xC0) == 0x80 && (next2 & 0xC0) == 0x80 && (next3 & 0xC0) == 0x80)
					{
						_isUtf8 = true;
						_isEncodingDecided = true;
					}
					else
					{
						_isUtf8 = false;
						_isEncodingDecided = true;
					}
				}
				else
				{
					// 数据不够，等待下次 append
					break;
				}
			}
			else
			{
				// 非法的 UTF-8 起始字节（比如 10xxxxxx 或者 11111xxx），必然是 local
				_isUtf8 = false;
				_isEncodingDecided = true;
			}
		}

		// 判断当前字符长度
		size_t charLen = 1;
		if (c <= 0x7F)
		{
			charLen = 1; // ASCII
		}
		else if (_isEncodingDecided)
		{
			if (_isUtf8)
			{
				if ((c & 0xE0) == 0xC0) charLen = 2;
				else if ((c & 0xF0) == 0xE0) charLen = 3;
				else if ((c & 0xF8) == 0xF0) charLen = 4;
				else charLen = 1; // 容错：非法字节当单字节处理
			}
			else // local (MBCS, e.g. GBK)
			{
				// 对于本地 MBCS 编码，大于 0x7F 的字节通常是前导字节，占用 2 字节
				// 此处使用 IsDBCSLeadByte 确保在当前 Windows locale 下的准确性
				if (IsDBCSLeadByte(c))
					charLen = 2;
				else
					charLen = 1; // 有些单字节扩展字符或非法字节
			}
		}
		else
		{
			// c > 0x7F 但没决定编码（上面 break 了，说明数据不够），跳出等待
			break;
		}

		// 检查是否有足够的字节构成完整的字符
		if (i + charLen > _rawInputBuffer.length())
		{
			break; // 不完整，保留在缓冲区
		}

		// 如果确定了是 local，且当前处理的是非 ASCII
		if (_isEncodingDecided && !_isUtf8 && c > 0x7F)
		{
			// 取出这个多字节序列并转换
			std::string mbcsChar = _rawInputBuffer.substr(i, charLen);
			processStr += local_to_utf8(mbcsChar);
		}
		else
		{
			// UTF-8 或 ASCII，直接追加
			processStr.append(_rawInputBuffer, i, charLen);
		}

		i += charLen;
	}

	// 从 buffer 中移除已处理的部分
	_rawInputBuffer.erase(0, i);

	if (!processStr.empty())
	{
		_ProcessUtf8Data(processStr);
	}
}

bool CChatTask_CLI::COutputBuffer::Fetch(std::string& output)
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_incrementalBuffer.empty())
		return false;

	output = _incrementalBuffer;
	_incrementalBuffer.clear();
	return true;
}

void CChatTask_CLI::COutputBuffer::Finish()
{
	std::lock_guard<std::mutex> lock(_mutex);

	// 处理剩余的 raw buffer
	if (!_rawInputBuffer.empty())
	{
		std::string processStr;
		if (!_isEncodingDecided)
		{
			// 如果到最后也没决定编码，尝试判断（此时可能只有很少或残缺字节）
			_isUtf8 = Utils::is_valid_utf8(_rawInputBuffer);
			_isEncodingDecided = true;
		}

		if (_isUtf8)
		{
			processStr = _rawInputBuffer; // 已经是最后了，不管完整与否，直接加入
		}
		else
		{
			processStr = local_to_utf8(_rawInputBuffer);
		}
		_rawInputBuffer.clear();
		_ProcessUtf8Data(processStr);
	}

	// 修正 _tailBuffer 到精确的 _tailLimit 并保持 UTF-8 完整性
	if (_tailBuffer.length() > _tailLimit)
	{
		size_t excess = _tailBuffer.length() - _tailLimit;
		// 寻找 excess 之后第一个合法的 UTF-8 起始字符
		while (excess < _tailBuffer.length() && (_tailBuffer[excess] & 0xC0) == 0x80)
		{
			excess++;
		}
		_tailBuffer.erase(0, excess);
	}

	// 如果发生了截断
	if (_totalUtf8BytesProcessed > _headLimit + _tailBuffer.length())
	{
		size_t actualOmitted = _totalUtf8BytesProcessed - _headLimit - _tailBuffer.length();
		std::string omitMsg = "\n\n... (" + std::to_string(actualOmitted) + " bytes truncated) ...\n\n";

		_incrementalBuffer += omitMsg;
		_incrementalBuffer += _tailBuffer;
	}
	else if (!_tailBuffer.empty())
	{
		// 如果没有截断，但有 tail（例如刚好长度等于 limit+tail 甚至更小），
		// 这里在 _ProcessUtf8Data 中其实 _incrementalBuffer 已经输出了
		// 但由于我们在 ProcessUtf8Data 里如果超出 headLimit 就把后续的直接放入了 _tailBuffer，
		// 所以即使没有省略，如果总长度大于 headLimit，超出的部分并未发给 UI，
		// 这里要把它们补上。
		if (_totalUtf8BytesProcessed > _headLimit)
		{
			_incrementalBuffer += _tailBuffer;
		}
	}
}

std::string CChatTask_CLI::COutputBuffer::GetFullResult() const
{
	std::lock_guard<std::mutex> lock(_mutex);
	if (_totalUtf8BytesProcessed > _headLimit + _tailBuffer.length())
	{
		size_t actualOmitted = _totalUtf8BytesProcessed - _headLimit - _tailBuffer.length();
		std::string omitMsg = "\n\n... (" + std::to_string(actualOmitted) + " bytes truncated) ...\n\n";
		return _headBuffer + omitMsg + _tailBuffer;
	}
	return _headBuffer + _tailBuffer; // 如果未超出 headLimit，_tailBuffer 是空的
}

void CChatTask_CLI::_ThreadFunc()
{
	// 获取命令参数
	std::string command;
	if (!_toolCall.GetStringParam("command", command))
	{
		_SetThreadResult("Error: Missing command parameter", "", "", false);
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

	// 检查是否被中断
	if (_shouldStop)
	{
		_SetThreadResult("Task interrupted", "", "", false);
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
		_SetThreadResult("Error: Failed to create output pipe", "", "", false);
		return;
	}

	// 确保输出管道读取端不被继承
	SetHandleInformation(hReadPipe, HANDLE_FLAG_INHERIT, 0);

	// 创建输入管道
	if (!CreatePipe(&hReadInputPipe, &_hWriteInput, &saAttr, 0))
	{
		CloseHandle(hReadPipe);
		CloseHandle(hWritePipe);
		_SetThreadResult("Error: Failed to create input pipe", "", "", false);
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
			_SetThreadResult("Error: cmd.exe does not support multi-line commands", "", "", false);
			return;
		}

		//可能需要加上 /u,使输出unicode字符,避免某些cmd命令(比如dir)无法正确在英文windows上输出中文时出现"???"的问题
		//需要同时在COutputBuffer里添加检测unicode编码的逻辑
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

		_SetThreadResult("Error: Failed to execute command: '" + command + "'", "", "", false);
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

			// 处理最终输出和截断（中断时也需要输出已有内容）
			_outputBuffer.Finish();
			_outputBufferSimple.Finish();
			std::string fullResult = _outputBuffer.GetFullResult();
			std::string simpleResult = _outputBufferSimple.GetFullResult();

			_SetThreadResult(fullResult, simpleResult, "", false);
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
			// 使用 _outputBuffer 处理增量和截断
			_outputBuffer.Append(buffer, bytesRead);
			_outputBufferSimple.Append(buffer, bytesRead);  // 同时写入简化版 buffer

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

	// 处理最终输出和截断
	_outputBuffer.Finish();
	_outputBufferSimple.Finish();

	std::string fullResult = _outputBuffer.GetFullResult();
	std::string simpleResult = _outputBufferSimple.GetFullResult();

	// 构建返回结果
	std::string resultStr;
	std::string messageStr;

	if (exitCode == 0)
	{
		resultStr = fullResult;
		messageStr = "Successfully executed command: \"" + command + "\"";
		_threadSuccess = true;
	}
	else
	{
		resultStr = "Command failed with exit code " + std::to_string(exitCode) + "\nOutput:\n" + fullResult;
		messageStr = "Command failed: \"" + command + "\" (exit code: " + std::to_string(exitCode) + ")";
		_threadSuccess = false;
	}

	// 清理临时文件
	if (useTempFile && !tempFilePath.empty())
	{
		DeleteFileW(tempFilePath.c_str());
	}

	// 保存结果
	_SetThreadResult(resultStr, simpleResult, messageStr, _threadSuccess);
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

	_outputBuffer.Reset();
	_outputBufferSimple.Reset();
	_threadResultSimple.clear();

	// 解析maxOutput参数（可选，默认16000）
	if (_toolCall.ExistParam("maxOutput"))
	{
		int maxOutput = 16000;
		_toolCall.GetIntParam("maxOutput", maxOutput);
		if (maxOutput < 100)
			maxOutput = 100; // 最小100
		else if (maxOutput > 1000000)
			maxOutput = 1000000; // 最大100万
		_outputBuffer.SetHeadLimit(static_cast<size_t>(maxOutput));
	}

	_outputBufferSimple.SetHeadLimit(2000);

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

			// 把可能的残余界面输出先发送出来
			std::string chunksToProcess;
			if (_outputBuffer.Fetch(chunksToProcess) && !chunksToProcess.empty())
			{
				_AppendOutputToDisplay(chunksToProcess);
			}

			// 获取之前的输出结果，构建最终结果
			std::string previousResult;
			std::string previousResultSimple;
			{
				std::lock_guard<std::mutex> lock(_resultMutex);
				previousResult = _threadResult;
				previousResultSimple = _threadResultSimple;
			}
			
			std::string finalResult = previousResult.empty()
				? "Command was stopped by user"
				: previousResult + "\n\nCommand was stopped by user";
			
			std::string finalResultSimple = previousResultSimple.empty()
				? "Command was stopped by user"
				: previousResultSimple + "\n\nCommand was stopped by user";

			// 在输出区域显示停止信息
			_AppendOutputToDisplay("\nCommand was stopped by user");

			// 发送工具调用结果（包含之前的输出）
			_SendToolCallResult(finalResult.c_str(), finalResultSimple.c_str());

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
		std::string chunksToProcess;
		_outputBuffer.Fetch(chunksToProcess);

		// 处理增量输出（现在chunksToProcess为完整的UTF-8字符串或为空）
		if (!chunksToProcess.empty())
		{
			_AppendOutputToDisplay(chunksToProcess);

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
		// 确保将最后可能的剩余输出刷新到界面
		std::string chunksToProcess;
		if (_outputBuffer.Fetch(chunksToProcess) && !chunksToProcess.empty())
		{
			_AppendOutputToDisplay(chunksToProcess);
		}

		// 等待线程结束并清理
		_CleanupWorkerThread();

		// 发送工具调用结果（供LLM使用）
		{
			std::lock_guard<std::mutex> lock(_resultMutex);
			_SendToolCallResult(_threadResult.c_str(), _threadResultSimple.c_str());
		}

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

	// 清理工作线程
	_CleanupWorkerThread();

	// 设置为 None 状态
	if (_context && _context->chatUi && !_cliId.empty())
	{
		_context->chatUi->SetCliStatus(_cliId, CliStatus::None);
	}

	_CompleteCliAndCleanup(-1);

	_status = TaskStatus::Failure;
}
