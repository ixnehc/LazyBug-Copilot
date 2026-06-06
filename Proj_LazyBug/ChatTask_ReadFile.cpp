#include "stdh.h"
#include "ChatTask_ReadFile.h"
#include "Utils.h"

#include "LlmChat.h"

#include "LlmLib.h"

#include <sstream>

// 辅助函数：生成简化版代码内容（只保留前20行）
static std::string _MakeSimplifiedCode(const std::string& codeContent, int& outLineCount)
{
	outLineCount = 0;
	
	// 按行分割
	std::vector<std::string> lines;
	std::istringstream iss(codeContent);
	std::string line;
	while (std::getline(iss, line))
	{
		lines.push_back(line);
	}
	
	// 如果行数不超过20行，直接返回原内容
	if (lines.size() <= 20)
	{
		outLineCount = (int)lines.size();
		return codeContent;
	}
	
	// 只保留前20行
	std::string result;
	for (size_t i = 0; i < 20 && i < lines.size(); ++i)
	{
		result += lines[i] + "\n";
	}
	
	outLineCount = 20;
	return result;
}

CChatTask_ReadFile::CChatTask_ReadFile()
{
	_workerThread = nullptr;
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
	_threadSimpleStartLine = 1;
	_threadSimpleEndLine = 1;
}

CChatTask_ReadFile::~CChatTask_ReadFile()
{
	Interrupt();
}

bool CChatTask_ReadFile::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("ReadFile"))
		return false;

	return true;
}

void CChatTask_ReadFile::_Fail()
{
	_status = TaskStatus::Failure;
}

void CChatTask_ReadFile::_Succeed()
{
	_status = TaskStatus::Success;
}

void CChatTask_ReadFile::_ThreadFunc()
{
	// 获取文件路径
	std::string filePath;
	if (!_toolCall.GetStringParam("filePath", filePath))
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: Missing filePath parameter";
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}

	FixSlashInPath_Utf8((char*)filePath.c_str());
	
	// 检查路径是否为完全路径
	if (!IsFullPath(filePath.c_str()))
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: filePath must be a full path: '" + filePath + "'";
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}
	
	// 获取起始行（可选）
	int startLine = 1;
	if (_toolCall.ExistParam("startLine"))
	{
		_toolCall.GetIntParam("startLine", startLine);
		if (startLine < 1)
			startLine = 1;
	}
	
	// 获取结束行（可选）
	int endLine = -1;
	if (_toolCall.ExistParam("endLine"))
	{
		_toolCall.GetIntParam("endLine", endLine);
	}
	
	// 检查startLine和endLine的有效性
	if (endLine != -1 && startLine > endLine)
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: startLine cannot be greater than endLine (startLine=" + std::to_string(startLine) + ", endLine=" + std::to_string(endLine) + ")";
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
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
	
	// 使用Utils::GetFilePartIntoUTF8读取文件内容
	std::string fileContent;
	Utils::FileContentCodingFormat codingFmt;
	bool readSuccess = false;
	
	if (endLine == -1)
	{
		// 如果没有指定结束行，读取整个文件
		readSuccess = Utils::GetFileContentIntoUTF8(filePath.c_str(), fileContent, codingFmt);
	}
	else
	{
		// 读取指定行范围的内容
		readSuccess = Utils::GetFilePartIntoUTF8(filePath.c_str(), startLine - 1, endLine - 1, fileContent, codingFmt);
	}
	
	// 再次检查是否被中断
	if (_shouldStop)
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Task interrupted";
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}
	
	// 构建返回结果
	std::string resultStr;
	std::string resultStrSimple;
	std::string messageStr;
	
	// 初始化 simple result 的行范围
	int simpleStartLine = startLine;
	int simpleEndLine = startLine;
	
	// 构建行号范围描述
	std::string lineRangeStr;
	if (endLine != -1)
	{
		std::ostringstream oss;
		oss << " (lines " << startLine << "-" << endLine << ")";
		lineRangeStr = oss.str();
	}
	
	if (!readSuccess)
	{
		resultStr = "Error: Failed to read file: '" + filePath + "'";
		resultStrSimple = resultStr;
		messageStr = "Failed to read file: \"" + filePath + "\"" + lineRangeStr;
	}
	else
	{
		if (fileContent.empty())
		{
			resultStr = "File is empty: '" + filePath + "'";
			resultStrSimple = resultStr;
			messageStr = "Read empty file: \"" + filePath + "\"" + lineRangeStr;
		}
		else
		{
			resultStr = fileContent;
			int simpleLineCount = 0;
			resultStrSimple = _MakeSimplifiedCode(fileContent, simpleLineCount);
			messageStr = "Successfully read file: \"" + filePath + "\"" + lineRangeStr;
			
			// 计算 simple result 的实际行范围
			simpleStartLine = startLine;
			simpleEndLine = startLine + simpleLineCount - 1;
		}
	}
	
	// 保存结果
	std::lock_guard<std::mutex> lock(_resultMutex);
	_threadResult = resultStr;
	_threadResultSimple = resultStrSimple;
	_threadMessage = messageStr;
	_threadSimpleStartLine = simpleStartLine;
	_threadSimpleEndLine = simpleEndLine;
	_threadSuccess = readSuccess;
	_threadFinished = true;
}

void CChatTask_ReadFile::Start()
{
	_status = TaskStatus::Running;
	
	// 重置状态
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
	_threadResult.clear();
	_threadResultSimple.clear();
	_threadMessage.clear();
	_threadSimpleStartLine = 1;
	_threadSimpleEndLine = 1;
	
	// 启动工作线程
	_workerThread = new std::thread(&CChatTask_ReadFile::_ThreadFunc, this);
}

void CChatTask_ReadFile::Update()
{
	if (_status != TaskStatus::Running)
		return;
		
	// 检查线程是否完成
	if (_threadFinished)
	{
		// 等待线程结束
		if (_workerThread && _workerThread->joinable())
		{
			_workerThread->join();
		}
		
		// 获取结果并发送
		{
			std::lock_guard<std::mutex> lock(_resultMutex);
			// 创建 simple result 的 tool call，修改 startLine 和 endLine
			LlmToolCall toolCallSimple = _toolCall;
			toolCallSimple.params_int["startLine"] = _threadSimpleStartLine;
			toolCallSimple.params_int["endLine"] = _threadSimpleEndLine;
			_SendToolCallResult(_threadResult.c_str(), _threadResultSimple.c_str(), &toolCallSimple);
			_SendToolCallMessage(_threadMessage.c_str());
		}
		
		// 清理线程
		if (_workerThread)
		{
			delete _workerThread;
			_workerThread = nullptr;
		}
		
		// 设置最终状态
		if (_threadSuccess)
			_Succeed();
		else
			_Fail();
	}
}

void CChatTask_ReadFile::Interrupt()
{
	// 设置停止标志
	_shouldStop = true;
	
	// 等待线程结束
	if (_workerThread && _workerThread->joinable())
	{
		_workerThread->join();
	}
	
	// 清理线程
	if (_workerThread)
	{
		delete _workerThread;
		_workerThread = nullptr;
	}
	
	_status = TaskStatus::Failure;
}
