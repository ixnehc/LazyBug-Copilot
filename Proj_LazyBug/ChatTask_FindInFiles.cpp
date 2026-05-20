#include "stdh.h"
#include "ChatTask_FindInFiles.h"
#include <algorithm>
#include <cstring>
#include "Utils.h"

#include "LlmChat.h"

#include "LlmLib.h"

#include "SolutionDBApi.h"
#include <sstream>
#include "nlohmann/json.hpp"
#include "Utils_FindInFile.h"

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

CChatTask_FindInFiles::CChatTask_FindInFiles()
{
	_workerThread = nullptr;
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
}

CChatTask_FindInFiles::~CChatTask_FindInFiles()
{
	Interrupt();
}

bool CChatTask_FindInFiles::DependsOn(CChatTask* task0)
{
	if (!task0->CheckType("FindInFiles"))
		return false;

	return true;
}

void CChatTask_FindInFiles::_Fail()
{
	_status = TaskStatus::Failure;
}

void CChatTask_FindInFiles::_Succeed()
{
	_status = TaskStatus::Success;
}

void CChatTask_FindInFiles::_TestCases()
{
	int maxResult = 120;
	{
		AbsTick tStart = GetAbsTick();
		SolutionDBMsg_FindInFilesResults result;
		SolutionDB_FindInFiles(_dbFolderPath.c_str(), "float aTimeOffset, BP_WaypointType aWaypointType", maxResult, result);
		int c = result.results.GetTotalResults();
		AbsTick dur = GetAbsTick() - tStart;
		int v = 0;
		v++;//2,2
	}

	{
		AbsTick tStart = GetAbsTick();
		SolutionDBMsg_FindInFilesResults result;
		SolutionDB_FindInFiles(_dbFolderPath.c_str(), ".GetCommands().", maxResult, result);
		int c = result.results.GetTotalResults();
		AbsTick dur = GetAbsTick() - tStart;
		int v = 0;
		v++;//27,4
	}

	{
		AbsTick tStart = GetAbsTick();
		SolutionDBMsg_FindInFilesResults result;
		SolutionDB_FindInFiles(_dbFolderPath.c_str(), "myIsLoopStart", maxResult, result);
		int c = result.results.GetTotalResults();
		AbsTick dur = GetAbsTick() - tStart;
		int v = 0;
		v++;//10,5
	}

	{
		AbsTick tStart = GetAbsTick();
		SolutionDBMsg_FindInFilesResults result;
		SolutionDB_FindInFiles(_dbFolderPath.c_str(), "const RShared_Agent* myAgent", maxResult, result);
		int c = result.results.GetTotalResults();
		AbsTick dur = GetAbsTick() - tStart;
		int v = 0;
		v++;//11,11
	}

	{
		AbsTick tStart = GetAbsTick();
		SolutionDBMsg_FindInFilesResults result;
		SolutionDB_FindInFiles(_dbFolderPath.c_str(), "pathIntegrator", maxResult, result);
		int c = result.results.GetTotalResults();
		AbsTick dur = GetAbsTick() - tStart;
		int v = 0;
		v++;//9,4
	}

	{
		AbsTick tStart = GetAbsTick();
		SolutionDBMsg_FindInFilesResults result;
		SolutionDB_FindInFiles(_dbFolderPath.c_str(), "\"Agent %s does not have valid animation metrics data, this will cause it to use default values! Please fill the AnimationMetrics field in the \"", maxResult, result);
		int c = result.results.GetTotalResults();
		AbsTick dur = GetAbsTick() - tStart;
		int v = 0;
		v++;//2,2
	}

	{
		AbsTick tStart = GetAbsTick();
		SolutionDBMsg_FindInFilesResults result;
		SolutionDB_FindInFiles(_dbFolderPath.c_str(), "\"Commands/BP_IAnimationCommandHandler.h\"", maxResult, result);
		int c = result.results.GetTotalResults();
		AbsTick dur = GetAbsTick() - tStart;
		int v = 0;
		v++;//4,4
	}

	{
		AbsTick tStart = GetAbsTick();
		SolutionDBMsg_FindInFilesResults result;
		SolutionDB_FindInFiles(_dbFolderPath.c_str(), "using Embedded server (= 'Offline mode').\"", maxResult, result);
		int c = result.results.GetTotalResults();
		AbsTick dur = GetAbsTick() - tStart;
		int v = 0;
		v++;//2,2
	}

	{
		AbsTick tStart = GetAbsTick();
		SolutionDBMsg_FindInFilesResults result;
		SolutionDB_FindInFiles(_dbFolderPath.c_str(), "0.0005f", maxResult, result);
		int c = result.results.GetTotalResults();
		AbsTick dur = GetAbsTick() - tStart;
		int v = 0;
		v++;//10,xx
	}

	{
		AbsTick tStart = GetAbsTick();
		SolutionDBMsg_FindInFilesResults result;
		SolutionDB_FindInFiles(_dbFolderPath.c_str(), "\"Assigned to:\\n\"", maxResult, result);
		int c = result.results.GetTotalResults();
		AbsTick dur = GetAbsTick() - tStart;
		int v = 0;
		v++;//1,xx
	}

}

void CChatTask_FindInFiles::_ThreadFunc()
{
	nlohmann::json resultJson;
	
	if (_dbFolderPath.empty())
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		Utils::BuildFindInFilesErrorJson(resultJson, "No solution opened");
		_threadResult = resultJson.dump();
		_threadMessage = "Found 0 match as solution is not opened!";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}
	// 获取关键字
	std::string keyword;
	if (!_toolCall.GetStringParam("keywords", keyword))
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		Utils::BuildFindInFilesErrorJson(resultJson, "Missing keyword parameter");
		_threadResult = resultJson.dump();
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
		Utils::BuildFindInFilesErrorJson(resultJson, "No valid keywords provided");
		_threadResult = resultJson.dump();
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}
	
	// 存储每个关键字的结果
	std::unordered_map<std::string, FindInFileResults> resultsMap;
	
	// 对每个关键字进行查找
	for (const auto& kw : keywords)
	{
		// 检查是否被中断
		if (_shouldStop)
		{
			std::lock_guard<std::mutex> lock(_resultMutex);
			Utils::BuildFindInFilesErrorJson(resultJson, "Task interrupted");
			_threadResult = resultJson.dump();
			_threadMessage = "";
			_threadSuccess = false;
			_threadFinished = true;
			return;
		}
		
		// 调用SolutionDB接口在文件中查找
		SolutionDBMsg_FindInFilesResults result;
		SolutionDB_FindInFiles(_dbFolderPath.c_str(), kw.c_str(), maxResult, result);
		
		resultsMap[kw] = std::move(result.results);
	}

	// 检查是否被中断
	if (_shouldStop)
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		Utils::BuildFindInFilesErrorJson(resultJson, "Task interrupted");
		_threadResult = resultJson.dump();
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}
	
	// 使用BuildFindInFilesResultJson构建结果JSON
	Utils::BuildFindInFilesResultJson(resultJson, resultsMap, maxResult);
	
	// 从JSON中直接提取总计数
	size_t totalMatchesAll = resultJson["totalMatches"].get<size_t>();
	size_t totalFilesAll = resultJson["totalFiles"].get<size_t>();
	
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
	
	bool hasAnyResult = totalMatchesAll > 0;
	
	if (keywords.size() == 1)
	{
		// 单个关键字的显示格式
		const auto& results = resultsMap.begin()->second;
		const std::string& kw = resultsMap.begin()->first;
		if (results.fileInfos.empty())
		{
			_threadMessage = "Found 0 match(es) for keyword \"" + kw + "\" in files!";
		}
		else
		{
			size_t totalMatches = results.GetTotalResults();
			size_t fileCount = results.fileInfos.size();
			_threadMessage = "Found " + std::to_string(totalMatches) + " match(es) for keyword \"" + kw + 
			                 "\" in " + std::to_string(fileCount) + " file(s)!";
		}
	}
	else
	{
		// 多个关键字的显示格式
		if (!hasAnyResult)
		{
			_threadMessage = "Found 0 match(es) for keyword(s) " + keywordsDisplay + " in files!";
		}
		else
		{
			_threadMessage = "Found total " + std::to_string(totalMatchesAll) + " match(es) for keyword(s) " + keywordsDisplay + 
			                 " in " + std::to_string(totalFilesAll) + " file(s)!";
		}
	}
	
	// 保存结果
	std::lock_guard<std::mutex> lock(_resultMutex);
	_threadResult = resultJson.dump();
	_threadSuccess = true;
	_threadFinished = true;
}

void CChatTask_FindInFiles::Start()
{
	_status = TaskStatus::Running;
	
	// 重置状态
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
	_threadResult.clear();

	_dbFolderPath = GetOpenedDBFolderPath_utf8();

// 	std::string keyword;
// 	if (_toolCall.GetStringParam("keyword", keyword))
// 	{
// 		std::string message = "Finding \"";
// 		message += keyword;
// 		message += "\"...";
// 		_SendToolCallMessage(message.c_str());
// 	}

// 	_SendToolCallMessage("aaaa");
// 	_SendToolCallMessage("bbbb");

	// 启动工作线程
	_workerThread = new std::thread(&CChatTask_FindInFiles::_ThreadFunc, this);
}

void CChatTask_FindInFiles::Update()
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

void CChatTask_FindInFiles::Interrupt()
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

