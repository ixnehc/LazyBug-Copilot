#include "stdh.h"
#include "ChatTask_ResolveSymbolLinks.h"
#include <algorithm>
#include <cstring>
#include <cctype>
#include "Utils.h"
#include "stringparser/stringparser.h"

#include "LlmChat.h"

#include "LlmLib.h"

#include "SolutionDBApi.h"
#include "Utils.h"

#include "ChatUi.h"

#include <sstream>
#include <vector>
#include <map>
#include <set>

extern const char* GetOpenedDBFolderPath_utf8();

// 辅助函数：判断字符串是否可能是文件路径（包含路径分隔符或文件扩展名）
static bool IsLikelyFilePath(const std::string& text)
{
	// 包含路径分隔符
	if (text.find('/') != std::string::npos || text.find('\\') != std::string::npos)
		return true;
	
	// 检查是否有文件扩展名（最后一个点后面是常见的代码文件后缀）
	size_t dotPos = text.rfind('.');
	if (dotPos != std::string::npos && dotPos < text.length() - 1)
	{
		std::string ext = text.substr(dotPos + 1);
		// 转小写
		for (size_t i = 0; i < ext.length(); i++)
			ext[i] = std::tolower(ext[i]);
		
		// 常见代码文件扩展名
		if (ext == "cpp" || ext == "h" || ext == "c" || ext == "hpp" || ext == "inl" ||
			ext == "cs" || ext == "py" || ext == "java" || ext == "js" ||
			ext == "ts" || ext == "go" || ext == "rs" || ext == "swift" ||
			ext == "kt" || ext == "cc" || ext == "cxx" || ext == "ixx" ||
			ext == "m" || ext == "mm" || ext == "vb" || ext == "fs")
			return true;
	}
	
	return false;
}

// 辅助函数：判断字符串是否以某后缀结尾（不区分大小写）
static bool EndsWithIgnoreCase(const std::string& str, const std::string& suffix)
{
	if (suffix.length() > str.length())
		return false;
	std::string strEnd = str.substr(str.length() - suffix.length());
	return StringEqualNoCase(strEnd.c_str(), suffix.c_str());
}

// 辅助函数：提取符号的最后一部分（用于降级匹配）
static std::string ExtractLastSymbolPart(const std::string& symbol)
{
	// 尝试按 :: 分割
	size_t pos = symbol.rfind("::");
	if (pos != std::string::npos)
		return symbol.substr(pos + 2);
	
	// 尝试按 . 分割
	pos = symbol.rfind('.');
	if (pos != std::string::npos)
		return symbol.substr(pos + 1);
	
	return symbol;
}

// 辅助函数：判断字符串是否包含符号分隔符
static bool ContainsSymbolSeparator(const std::string& text)
{
	return text.find("::") != std::string::npos || text.find('.') != std::string::npos;
}

CChatTask_ResolveSymbolLinks::CChatTask_ResolveSymbolLinks()
{
	_workerThread = nullptr;
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
}

CChatTask_ResolveSymbolLinks::CChatTask_ResolveSymbolLinks(const std::vector<SymbolLinkItem>& symbolLinks)
{
	_workerThread = nullptr;
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
	_symbolLinks = symbolLinks;
}

CChatTask_ResolveSymbolLinks::~CChatTask_ResolveSymbolLinks()
{
	Interrupt();
}

bool CChatTask_ResolveSymbolLinks::DependsOn(CChatTask* task0)
{
	// ResolveSymbolLinks 不依赖于其他任务
	return false;
}

void CChatTask_ResolveSymbolLinks::SetSymbolLinks(const std::vector<SymbolLinkItem>& symbolLinks)
{
	_symbolLinks = symbolLinks;
}

void CChatTask_ResolveSymbolLinks::_Fail()
{
	_status = TaskStatus::Failure;
}

void CChatTask_ResolveSymbolLinks::_Succeed()
{
	_status = TaskStatus::Success;
}

// 文本预处理：专门用于文件路径（只去除空白和引号）
static std::string PreprocessFilePathText(const std::string& rawText)
{
	std::string text = rawText;
	
	// 1. 去除两端的空白字符
	RemoveBlank(text);
	if (text.empty())
		return text;
	
	// 2. 去除两端的反引号和引号
	while (!text.empty() && (text.front() == '`' || text.front() == '"' || text.front() == '\''))
		text.erase(0, 1);
	while (!text.empty() && (text.back() == '`' || text.back() == '"' || text.back() == '\''))
		text.pop_back();
	
	// 再次去除两端空白
	RemoveBlank(text);
	
	return text;
}

// 文本预处理：专门用于代码符号（去除语言修饰符、参数、泛型等）
static std::string PreprocessSymbolText(const std::string& rawSymbolText)
{
	// 先使用文件路径的预处理
	std::string text = PreprocessFilePathText(rawSymbolText);
	if (text.empty())
		return text;
	
	// 3. 去除语言修饰符 (class, struct, enum, interface, def, func 等)
	static const char* prefixes[] = { "class ", "struct ", "enum ", "interface ", "def ", "func ", "function ", "method ", "var ", "let ", "const " };
	for (const char* prefix : prefixes)
	{
		size_t prefixLen = strlen(prefix);
		if (text.length() > prefixLen)
		{
			bool match = true;
			for (size_t i = 0; i < prefixLen; i++)
			{
				if (std::tolower(text[i]) != prefix[i])
				{
					match = false;
					break;
				}
			}
			if (match)
			{
				text = text.substr(prefixLen);
				break;
			}
		}
	}
	
	// 4. 去除参数和泛型部分（仅用于符号）
	size_t parenPos = text.find('(');
	if (parenPos != std::string::npos)
		text = text.substr(0, parenPos);
	
	size_t anglePos = text.find('<');
	if (anglePos != std::string::npos)
		text = text.substr(0, anglePos);
	
	// 再次去除两端空白
	RemoveBlank(text);
	
	return text;
}

// 当作文件路径搜索
// 返回结果数量 <= 2 时有效，> 2 或 0 时返回空数组
static std::vector<SymbolResolveResult> ResolveAsFilePath(const std::string& dbFolderPath, const std::string& rawSymbolText)
{
	std::vector<SymbolResolveResult> results;
	
	// 使用专门的文件路径预处理（不会去除括号等）
	std::string text = PreprocessFilePathText(rawSymbolText);
	if (text.empty())
		return results;
	
	// 判断是否只有后缀（例如 ".txt"），如果是则无效
	if (text[0] == '.' && text.find('\\') == std::string::npos && text.find('/') == std::string::npos)
		return results;
	
	// 判断是否可能是文件路径
	if (!IsLikelyFilePath(text))
		return results;
	
	// 搜索文件
	SolutionDBMsg_SearchFileResult fileResult;
	SolutionDB_SearchFile(dbFolderPath.c_str(), text.c_str(), 10, fileResult);
	
	// 严格过滤结果
	for (auto& fi : fileResult.results.fileInfos)
	{
		// 必须以 text 结尾
		if (!EndsWithIgnoreCase(fi.filePath, text))
			continue;
		
		// 截掉 text 部分后，剩余部分必须以 "\\" 结尾（或者剩余部分为空，表示完全匹配）
		std::string prefix = fi.filePath.substr(0, fi.filePath.length() - text.length());
		if (!prefix.empty() && prefix.back() != '\\')
			continue;
		
		SymbolResolveResult r;
		r.filePath = fi.filePath;
		r.lineNumber = -1;
		results.push_back(r);
	}
	
	// 返回结果数量 <= 2 时有效
	if (results.size() >= 1 && results.size() <= 2)
		return results;
	
	// 否则返回空数组
	results.clear();
	return results;
}

// 当作代码符号搜索
// 返回结果数量 <= 2 时有效，或牵涉到的文件数 <= 1 时有效，否则返回空数组
static std::vector<SymbolResolveResult> ResolveAsSymbol(const std::string& dbFolderPath, const std::string& rawSymbolText)
{
	std::vector<SymbolResolveResult> results;

	// 预处理
	std::string text = PreprocessSymbolText(rawSymbolText);
	if (text.empty())
		return results;

	text = Utils::NormalizeSymbolName(text);
	
	// 第一次搜索：直接搜索符号
	SolutionDBMsg_SymbolDefines symbolResult;
	SolutionDB_FindSymbolDefines(dbFolderPath.c_str(), text.c_str(), 10, symbolResult);
	
	// 统计牵涉到的不同文件数量
	std::set<std::string> uniqueFiles;
	for (auto& loc : symbolResult.locations)
	{
		uniqueFiles.insert(loc.filePath);
	}
	
	// 如果结果数在 1-2 之间，或牵涉的文件数 <= 2，则接受
	if ((symbolResult.locations.size() >= 1 && symbolResult.locations.size() <= 2) || uniqueFiles.size() <= 2)
	{
		for (auto& loc : symbolResult.locations)
		{
			SymbolResolveResult r;
			r.filePath = loc.filePath;
			r.lineNumber = loc.lineRange.start;
			results.push_back(r);
		}
		return results;
	}
	else
	{
		// 牵涉文件太多，直接放弃
		return results;
	}
// 	
// 	// 结果为 0，尝试降级匹配：提取符号的最后一部分
// 	if (ContainsSymbolSeparator(text))
// 	{
// 		std::string lastPart = ExtractLastSymbolPart(text);
// 		if (!lastPart.empty() && lastPart != text)
// 		{
// 			SolutionDBMsg_SymbolDefines symbolResult2;
// 			SolutionDB_FindSymbolDefines(dbFolderPath.c_str(), lastPart.c_str(), 50, symbolResult2);
// 			
// 			// 按数量限制返回前2个
// 			std::vector<SymbolResolveResult> filteredResults;
// 			for (auto& loc : symbolResult2.locations)
// 			{
// 				SymbolResolveResult r;
// 				r.filePath = loc.filePath;
// 				r.lineNumber = loc.lineRange.lineStart;
// 				filteredResults.push_back(r);
// 			}
// 			
// 			if (filteredResults.size() >= 1 && filteredResults.size() <= 2)
// 				return filteredResults;
// 			else if (filteredResults.size() > 2)
// 			{
// 				// 只返回前2个
// 				results.push_back(filteredResults[0]);
// 				results.push_back(filteredResults[1]);
// 				return results;
// 			}
// 		}
// 	}
	
	return results;
}

// 模糊查询符号位置的主函数
std::vector<SymbolResolveResult> CChatTask_ResolveSymbolLinks::FuzzyResolveSymbol(const std::string& dbFolderPath, const std::string& rawSymbolText)
{
	std::vector<SymbolResolveResult> results;

	// 第一步：优先当做文件搜索（使用文件路径预处理）
	results = ResolveAsFilePath(dbFolderPath, rawSymbolText);
	if (!results.empty())
		return results;
	
	// 第二步：当做代码符号搜索（使用符号预处理）
	results = ResolveAsSymbol(dbFolderPath, rawSymbolText);
	if (!results.empty())
		return results;
	
	// 第三步：最终降级兜底（纯单词当做文件名搜索）
	// 使用文件路径预处理，因为这里是文件搜索
	std::string text = PreprocessFilePathText(rawSymbolText);
	if (text.empty())
		return results;
	
	bool isPureWord = !text.empty();
	for (char c : text)
	{
		if (!std::isalnum(c) && c != '_' && c != '-')
		{
			isPureWord = false;
			break;
		}
	}
	
	if (isPureWord)
	{
		SolutionDBMsg_SearchFileResult fileResult;
		SolutionDB_SearchFile(dbFolderPath.c_str(), text.c_str(), 10, fileResult);
		
		if (fileResult.results.fileInfos.size() >= 1 && fileResult.results.fileInfos.size() <= 2)
		{
			for (auto& fi : fileResult.results.fileInfos)
			{
				SymbolResolveResult r;
				r.filePath = fi.filePath;
				r.lineNumber = -1;
				results.push_back(r);
			}
			return results;
		}
		else if (fileResult.results.fileInfos.size() > 2)
		{
			// 尝试严格匹配：文件名（不含扩展名）完全等于 text
			std::vector<SymbolResolveResult> strictResults;
			for (auto& fi : fileResult.results.fileInfos)
			{
				std::string fileName = GetFileName(fi.filePath);
				// 去除扩展名
				size_t dotPos = fileName.rfind('.');
				if (dotPos != std::string::npos)
					fileName = fileName.substr(0, dotPos);
				
				if (fileName == text)
				{
					SymbolResolveResult r;
					r.filePath = fi.filePath;
					r.lineNumber = -1;
					strictResults.push_back(r);
				}
			}
			if (strictResults.size() >= 1 && strictResults.size() <= 2)
				return strictResults;
		}
	}
	
	return results;
}

void CChatTask_ResolveSymbolLinks::_ThreadFunc()
{
	if (_symbolLinks.empty())
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_threadResult = "Error: No symbol links provided";
		_threadMessage = "";
		_threadSuccess = false;
		_threadFinished = true;
		return;
	}
	
	int totalFound = 0;
	int totalProcessed = 0;
	
	for (size_t i = 0; i < _symbolLinks.size(); ++i)
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
		
		const SymbolLinkItem& item = _symbolLinks[i];
		totalProcessed++;
		
		// 转换 symbol 为窄字符串
		std::string symbolNameNarrow = widechar_to_utf8(item.symbol.c_str());
		
		// 使用模糊查询函数查找符号位置
		std::vector<SymbolResolveResult> results = FuzzyResolveSymbol(_dbFolderPath, symbolNameNarrow);
		
		// 如果找到了定义（结果数量 1~2 个），则添加到已解析队列
		if (!results.empty())
		{
			// 构建 results 的 JSON 字符串
			std::wstring resultsJson = L"[";
			for (size_t j = 0; j < results.size(); j++)
			{
				if (j > 0)
					resultsJson += L",";
				
				std::wstring filePathW = utf8_to_widechar(results[j].filePath.c_str());
				resultsJson += L"{\"filePath\":\"" + EscapeJsonString(filePathW) + L"\"";
				resultsJson += L",\"lineNumber\":" + std::to_wstring(results[j].lineNumber);
				resultsJson += L"}";
			}
			resultsJson += L"]";
			
			std::lock_guard<std::mutex> lock(_resolvedQueueMutex);
			_resolvedQueue.push_back(std::make_tuple(item.messageId, item.symbol, resultsJson));
			totalFound++;
		}
	}
	
	// 构建返回结果
	std::string resultStr = "Processed " + std::to_string(totalProcessed) + " symbol links, found " + std::to_string(totalFound) + " definitions";
	std::string messageStr = "Successfully resolved " + std::to_string(totalFound) + "/" + std::to_string(totalProcessed) + " symbol links";
	
	// 保存结果
	std::lock_guard<std::mutex> lock(_resultMutex);
	_threadResult = resultStr;
	_threadSuccess = true;
	_threadMessage = messageStr;
	_threadFinished = true;
}

void CChatTask_ResolveSymbolLinks::Start()
{
	_status = TaskStatus::Running;
	
	// 重置状态
	_shouldStop = false;
	_threadFinished = false;
	_threadSuccess = false;
	_threadResult.clear();

	_dbFolderPath = GetOpenedDBFolderPath_utf8();
	
	// 启动工作线程
	_workerThread = new std::thread(&CChatTask_ResolveSymbolLinks::_ThreadFunc, this);
}

void CChatTask_ResolveSymbolLinks::Update()
{
	if (_status != TaskStatus::Running)
		return;
	
	// 处理解析队列中的符号（在主线程中调用 ApplySymbolLinks）
	{
		std::lock_guard<std::mutex> lock(_resolvedQueueMutex);
		if (!_resolvedQueue.empty())
		{
			// 按 messageId 分组
			std::map<std::wstring, std::vector<std::pair<std::wstring, std::wstring>>> messageToSymbols;
			for (auto& item : _resolvedQueue)
			{
				const std::wstring& messageId = std::get<0>(item);
				const std::wstring& symbol = std::get<1>(item);
				const std::wstring& resultsJson = std::get<2>(item);
				messageToSymbols[messageId].push_back(std::make_pair(symbol, resultsJson));
			}
			_resolvedQueue.clear();
			
			// 调用 ApplySymbolLinks 为每个消息标记可链接的符号
			for (auto& pair : messageToSymbols)
			{
				const std::wstring& messageId = pair.first;
				const auto& symbolsWithResults = pair.second;
				
				// UI 操作，继续使用 chatCtrl
				if (_context && _context->chatUi)
				{
					_context->chatUi->ApplySymbolLinks(messageId, symbolsWithResults);
				}
			}
		}
	}
		
	// 检查线程是否完成
	if (_threadFinished)
	{
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
		
		// 设置最终状态
		if (_threadSuccess)
			_Succeed();
		else
			_Fail();
	}
}

void CChatTask_ResolveSymbolLinks::Interrupt()
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


