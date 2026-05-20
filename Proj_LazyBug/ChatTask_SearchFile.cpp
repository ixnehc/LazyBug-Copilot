#include "stdh.h"
#include "ChatTask_SearchFile.h"
#include <algorithm>
#include <cstring>
#include "Utils.h"

#include "LlmChat.h"

#include "LlmLib.h"

#include "SolutionDBApi.h"
#include <sstream>

extern const char* GetOpenedDBFolderPath_utf8();

// 辅助函数：分割字符串
static std::vector<std::string> SplitKeywords(const std::string& str, char delimiter)
{
	std::vector<std::string> tokens;
	std::stringstream ss(str);
	std::string token;
	while (std::getline(ss, token, delimiter))
	{
		// 去除首尾空格
		size_t start = token.find_first_not_of(" \t\r\n");
		size_t end = token.find_last_not_of(" \t\r\n");
		if (start != std::string::npos && end != std::string::npos)
		{
			tokens.push_back(token.substr(start, end - start + 1));
		}
		else if (start != std::string::npos)
		{
			tokens.push_back(token.substr(start));
		}
	}
	return tokens;
}

CChatTask_SearchFile::CChatTask_SearchFile()
{
	_workerThread = nullptr;
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
}

CChatTask_SearchFile::~CChatTask_SearchFile()
{
	Interrupt();
}

bool CChatTask_SearchFile::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("SearchFile"))
		return false;

	return true;
}

void CChatTask_SearchFile::_Fail()
{
	_status = TaskStatus::Failure;
}

void CChatTask_SearchFile::_Succeed()
{
	_status = TaskStatus::Success;
}

void CChatTask_SearchFile::_ThreadFunc()
{
	if (_dbFolderPath.empty())
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: No solution opened";
		_threadMessage = "Found 0 file path as solution is not opened!";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}
	// 获取关键字
	std::string keyword;
	if (!_toolCall.GetStringParam("keywords", keyword))
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: Missing keyword parameter";
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}
	
	// 获取最大结果数（可选）
	int maxResult = 120;

	// 解析多个关键字（以"|"分隔）
	std::vector<std::string> keywords = SplitKeywords(keyword, '|');
	if (keywords.empty())
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: No valid keywords provided";
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}
	
	// 存储每个关键字的结果
	struct KeywordResult
	{
		std::string keyword;
		SearchFileResult results;
	};
	std::vector<KeywordResult> allResults;
	
	// 对每个关键字进行查找
	for (const auto& kw : keywords)
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
		
		// 调用SolutionDB接口搜索文件
		SolutionDBMsg_SearchFileResult result;
		SolutionDB_SearchFile(_dbFolderPath.c_str(), kw.c_str(), maxResult, result);
		
		KeywordResult kr;
		kr.keyword = kw;
		kr.results = result.results;
		allResults.push_back(std::move(kr));
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
	
	// 构建返回结果 - 每个关键字独立显示
	std::string resultStr;
	bool hasAnyResult = false;
	size_t totalFilesAll = 0;
	
	for (size_t i = 0; i < allResults.size(); ++i)
	{
		const auto& kr = allResults[i];
		
		// 添加分隔符
		if (i > 0)
		{
			resultStr += "\n";
			resultStr += "========================================\n";
			resultStr += "\n";
		}
		
		// 添加关键字标题
		resultStr += "[Search Keyword: \"";
		resultStr += kr.keyword;
		resultStr += "\"]\n";
		resultStr += "----------------------------------------\n";
		
		// 添加该关键字的结果
		if (kr.results.fileInfos.empty())
		{
			resultStr += "No file path found for keyword: \"";
			resultStr += kr.keyword;
			resultStr += "\"\n";
		}
		else
		{
			hasAnyResult = true;
			for (const auto& fileInfo : kr.results.fileInfos)
			{
				resultStr += fileInfo.filePath;
				resultStr += "\n";
			}
			
			totalFilesAll += kr.results.fileInfos.size();
		}
	}
	
	// 构建总结消息
	std::string keywordsDisplay;
	for (size_t i = 0; i < keywords.size(); ++i)
	{
		if (i > 0)
		{
			keywordsDisplay += " | ";
		}
		keywordsDisplay += "\"" + keywords[i] + "\"";
	}
	
	if (keywords.size() == 1)
	{
		// 单个关键字的显示格式
		const auto& kr = allResults[0];
		if (kr.results.fileInfos.empty())
		{
			_threadMessage = "Found 0 file path for keyword \"" + kr.keyword + "\"!";
		}
		else
		{
			size_t fileCount = kr.results.fileInfos.size();
			_threadMessage = "Found " + std::to_string(fileCount) + " file path(s) for keyword \"" + kr.keyword + "\"!";
		}
	}
	else
	{
		// 多个关键字的显示格式
		if (!hasAnyResult)
		{
			_threadMessage = "Found 0 file path for keyword(s) " + keywordsDisplay + "!";
		}
		else
		{
			_threadMessage = "Found total " + std::to_string(totalFilesAll) + " file path(s) for keyword(s) " + keywordsDisplay + "!";
		}
	}
	
	// 保存结果
	std::lock_guard<std::mutex> lock(_resultMutex);
	_threadResult = resultStr;
	_threadSuccess = true;
	_threadFinished = true;
}

void CChatTask_SearchFile::Start()
{
	_status = TaskStatus::Running;
	
	// 重置状态
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
	_threadResult.clear();

	_dbFolderPath = GetOpenedDBFolderPath_utf8();

	// 启动工作线程
	_workerThread = new std::thread(&CChatTask_SearchFile::_ThreadFunc, this);
}

void CChatTask_SearchFile::Update()
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
			_SendToolCallResult(_threadResult.c_str());
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

void CChatTask_SearchFile::Interrupt()
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
