#include "stdh.h"
#include "ChatTask_FindSymbolDefine.h"
#include <algorithm>
#include <cstring>
#include "Utils.h"

#include "LlmChat.h"

#include "LlmLib.h"

#include "SolutionDBApi.h"
#include "Utils.h"
#include <sstream>
#include <vector>

extern const char* GetOpenedDBFolderPath_utf8();

// 辅助函数：生成简化版代码内容（只保留头尾各3行）
static std::string _MakeSimplifiedCode(const std::string& codeContent)
{
	// 按行分割
	std::vector<std::string> lines;
	std::istringstream iss(codeContent);
	std::string line;
	while (std::getline(iss, line))
	{
		lines.push_back(line);
	}
	
	// 如果行数不超过10行，直接返回原内容
	if (lines.size() <= 10)
		return codeContent;
	
	// 构建简化版本：头3行 + 省略提示 + 尾3行
	std::string result;
	size_t omittedCount = lines.size() - 6;
	
	for (size_t i = 0; i < 3; ++i)
	{
		result += lines[i] + "\n";
	}
	result += "...";
	result += std::to_string(omittedCount);
	result += " lines omitted...\n";
	for (size_t i = lines.size() - 3; i < lines.size(); ++i)
	{
		result += lines[i] + "\n";
	}
	
	return result;
}

CChatTask_FindSymbolDefine::CChatTask_FindSymbolDefine()
{
	_workerThread = nullptr;
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
}

CChatTask_FindSymbolDefine::~CChatTask_FindSymbolDefine()
{
	Interrupt();
}

bool CChatTask_FindSymbolDefine::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("FindSymbolDefine"))
		return false;

	return true;
}

void CChatTask_FindSymbolDefine::_Fail()
{
	_status = TaskStatus::Failure;
}

void CChatTask_FindSymbolDefine::_Succeed()
{
	_status = TaskStatus::Success;
}

void CChatTask_FindSymbolDefine::_ThreadFunc()
{
	// 获取符号名称（支持多个以"|"分隔的符号）
	std::string symbolsParam;
	if (!_toolCall.GetStringParam("symbols", symbolsParam))
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: Missing symbols parameter";
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}
	
	// 按"|"分割多个符号
	std::vector<std::string> symbolList;
	std::stringstream ss(symbolsParam);
	std::string item;
	while (std::getline(ss, item, '|'))
	{
		// 去除前后空格
		item.erase(0, item.find_first_not_of(" \t"));
		item.erase(item.find_last_not_of(" \t") + 1);
		if (!item.empty())
			symbolList.push_back(item);
	}
	
	if (symbolList.empty())
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: No valid symbols provided";
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}
	
	// 构建返回结果
	std::string resultStr;
	std::string resultStrSimple;
	std::string messageStr;
	int totalFound = 0;
	bool hasAnySuccess = false;
	
	for (size_t s = 0; s < symbolList.size(); ++s)
	{
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
		
		const std::string& symbolNameOrg = symbolList[s];
		std::string symbolName = Utils::NormalizeSymbolName(symbolNameOrg);
		
		// 调用SolutionDB接口查找符号定义
		SolutionDBMsg_SymbolDefines result;
		SolutionDB_FindSymbolDefines(_dbFolderPath.c_str(), symbolName.c_str(), 32, result);
		
		// 检查结果
		if (result.locations.empty())
		{
			std::string notFoundMsg;
			if (symbolList.size() > 1)
			{
				notFoundMsg = "Symbol '";
				notFoundMsg += symbolNameOrg;
				notFoundMsg += "': No definitions found\n\n";
			}
			else
			{
				notFoundMsg = "No definitions found for symbol: '";
				notFoundMsg += symbolNameOrg;
				notFoundMsg += "'";
			}
			resultStr += notFoundMsg;
			resultStrSimple += notFoundMsg;
			
			if (!messageStr.empty())
				messageStr += "\n";
			messageStr += "Failed to find defination for symbol \"";
			messageStr += symbolNameOrg;
			messageStr += "\"";
		}
		else
		{
			hasAnySuccess = true;
			totalFound += (int)result.locations.size();
			
			if (symbolList.size() > 1)
			{
				std::string symbolHeader = "========================================\nSymbol: ";
				symbolHeader += symbolNameOrg;
				symbolHeader += "\n========================================\n";
				resultStr += symbolHeader;
				resultStrSimple += symbolHeader;
			}
			
			{
				std::string foundMsg = "Found ";
				foundMsg += std::to_string(result.locations.size());
				foundMsg += " definition(s) for symbol: '";
				foundMsg += symbolNameOrg;
				foundMsg += "'\n\n";
				resultStr += foundMsg;
				resultStrSimple += foundMsg;
			}
			
			for (size_t i = 0; i < result.locations.size(); ++i)
			{
				const auto& loc = result.locations[i];
				
				std::string defHeader = "Definition #";
				defHeader += std::to_string(i + 1);
				defHeader += ":\nFile: ";
				defHeader += loc.filePath.c_str();
				defHeader += "\nLine Range: ";
				defHeader += std::to_string(loc.lineRange.start);
				defHeader += "~";
				defHeader += std::to_string(loc.lineRange.end + 1);
				defHeader += "\n";
				resultStr += defHeader;
				resultStrSimple += defHeader;
				
				// 尝试读取定义处的代码内容
				std::string codeContent;
				Utils::FileContentCodingFormat codingFmt;
				int start = loc.lineRange.start-10;
				int end = loc.lineRange.end + 1 + 10;
				if (start < 0)
					start = 0;
				if (Utils::GetFilePartIntoUTF8(loc.filePath.c_str(), start, end, codeContent, codingFmt))
				{
					resultStr += "Code:\n";
					resultStr += codeContent;
					resultStr += "\n";
					
					resultStrSimple += "Code:\n";
					resultStrSimple += _MakeSimplifiedCode(codeContent);
					resultStrSimple += "\n";
				}
				
				if (i < result.locations.size() - 1)
				{
					resultStr += "\n";
					resultStrSimple += "\n";
				}
			}
			
			if (!messageStr.empty())
				messageStr += "\n";
			messageStr += "Succesfully read the defination for symbol \"";
			messageStr += symbolName;
			messageStr += "\"";
			
			if (s < symbolList.size() - 1)
			{
				resultStr += "\n\n";
				resultStrSimple += "\n\n";
			}
		}
	}
	
	// 保存结果
	std::lock_guard<std::mutex> lock(_resultMutex);
	_threadResult = resultStr;
	_threadResultSimple = resultStrSimple;
	_threadSuccess = true;
	_threadMessage = messageStr;
	_threadFinished = true;
}

void CChatTask_FindSymbolDefine::Start()
{
	_status = TaskStatus::Running;
	
	// 重置状态
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
	_threadResult.clear();
	_threadResultSimple.clear();

	_dbFolderPath = GetOpenedDBFolderPath_utf8();
	
	// 启动工作线程
	_workerThread = new std::thread(&CChatTask_FindSymbolDefine::_ThreadFunc, this);
}

void CChatTask_FindSymbolDefine::Update()
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
			_SendToolCallResult(_threadResult.c_str(), _threadResultSimple.c_str());
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

void CChatTask_FindSymbolDefine::Interrupt()
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
