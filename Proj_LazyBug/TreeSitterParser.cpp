#include "stdh.h"
#include <fstream>
#include <sstream>

#include "TreeSitterParser.h"
#include "Utils.h"
#include "datapacket/DataPacket.h"

// TreeSitter C接口
#include "treesitter_api.h"

TreeSitterSymbol_Begin

//////////////////////////////////////////////////////////////////////////
// CTreeSitterParser

CTreeSitterParser::CTreeSitterParser()
{
	_running = false;
	_threadPriority = ThreadPriority::NORMAL;
	_discardId = ParseRequestId_Invalid;
	_activeCount = 0;
}

CTreeSitterParser::~CTreeSitterParser()
{
	Close();
}

void CTreeSitterParser::Init(int numThreads, ThreadPriority priority)
{
	if (_running)
		return;
	
	_running = true;
	_threadPriority = priority;
	_discardId = ParseRequestId_Invalid;
	_activeCount = 0;
	
	// 启动工作线程
	for (int i = 0; i < numThreads; i++)
	{
		_threads.push_back(std::thread(&CTreeSitterParser::WorkerThread, this));
	}
}

void CTreeSitterParser::Close()
{
	if (!_running)
		return;
	
	_running = false;
	
	// 通知所有线程退出
	_requestCV.notify_all();
	
	// 等待所有线程结束
	for (std::thread& thread : _threads)
	{
		if (thread.joinable())
			thread.join();
	}
	
	_threads.clear();
	
	// 清空队列
	{
		std::lock_guard<std::mutex> lock(_requestMutex);
		_requestQueue.clear();
	}
	
	{
		std::lock_guard<std::mutex> lock(_resultMutex);
		_resultQueue.clear();
	}
}

bool CTreeSitterParser::Request(ParseRequest& request)
{
	if (!_running)
		return false;
	
	{
		std::lock_guard<std::mutex> lock(_requestMutex);
		_requestQueue.push_back(ParseRequest());
		_requestQueue.back().MoveFrom(request);
		_activeCount++;
	}
	
	_requestCV.notify_one();
	return true;
}

bool CTreeSitterParser::FetchResult(ParseResult& result)
{
	std::lock_guard<std::mutex> lock(_resultMutex);
	
	if (_resultQueue.empty())
		return false;
	
	// 检查是否被丢弃
	ParseResult& front = _resultQueue.front();
	if (front.requestId == _discardId)
	{
		_resultQueue.pop_front();
		return false;
	}
	
	result = std::move(front);
	_resultQueue.pop_front();
	_activeCount--;
	return true;
}

void CTreeSitterParser::DiscardAll(ParseRequestId requestId)
{
	_discardId = requestId;
	
	// 清空请求队列
	{
		std::lock_guard<std::mutex> lock(_requestMutex);
		_requestQueue.clear();
	}
}

bool CTreeSitterParser::SetThreadPriority(ThreadPriority priority)
{
	HANDLE threadHandle = GetCurrentThread();
	int winPriority;
	
	switch (priority)
	{
	case ThreadPriority::LOWEST:
		winPriority = THREAD_PRIORITY_LOWEST;
		break;
	case ThreadPriority::BELOW_NORMAL:
		winPriority = THREAD_PRIORITY_BELOW_NORMAL;
		break;
	case ThreadPriority::NORMAL:
		winPriority = THREAD_PRIORITY_NORMAL;
		break;
	case ThreadPriority::ABOVE_NORMAL:
		winPriority = THREAD_PRIORITY_ABOVE_NORMAL;
		break;
	case ThreadPriority::HIGHEST:
		winPriority = THREAD_PRIORITY_HIGHEST;
		break;
	case ThreadPriority::TIME_CRITICAL:
		winPriority = THREAD_PRIORITY_TIME_CRITICAL;
		break;
	default:
		return false;
	}
	
	return ::SetThreadPriority(threadHandle, winPriority) != FALSE;
}

void CTreeSitterParser::WorkerThread()
{
	// 设置线程优先级
	SetThreadPriority(_threadPriority);
	
	while (_running)
	{
		ParseRequest request;
		
		// 等待请求
		{
			std::unique_lock<std::mutex> lock(_requestMutex);
			_requestCV.wait(lock, [this] {
				return !_running || !_requestQueue.empty();
			});
			
			if (!_running)
				break;
			
			if (_requestQueue.empty())
				continue;
			
			request = std::move(_requestQueue.front());
			_requestQueue.pop_front();
		}
		
		// 处理请求
		ParseResult result;
		result.requestId = request.requestId;
		result.parseFilePath = request.lowerCasedParseFilePath;
		
		// 检查是否被丢弃
		if (request.requestId == _discardId)
		{
			result.discarded = true;
			result.success = false;
		}
		else
		{
			// 执行解析
			result.success = ParseWithTreeSitter(request, result);
		}
		
		// 将结果放入结果队列
		{
			std::lock_guard<std::mutex> lock(_resultMutex);
			_resultQueue.push_back(std::move(result));
		}
	}
}

bool CTreeSitterParser::ParseWithTreeSitter(const ParseRequest& request, ParseResult& result)
{
#ifdef ENABLE_TREESITTER_PARSER_DEBUG
	result.AddDebugTime("ParseWithTreeSitter_Start");
#endif
	
	// 检查语言是否有效
	if (request.language == Language::Unknown)
	{
		return false;
	}
	
	// 获取语言支持
	auto languageSupport = CLanguageFactory::Instance().GetLanguageSupport(request.language);
	if (!languageSupport)
	{
		return false;
	}
	
#ifdef ENABLE_TREESITTER_PARSER_DEBUG
	result.AddDebugTime("GetLanguageSupport");
#endif
	
	// 读取文件内容
	std::string content;
	if (!Utils::LoadFileContent(request.lowerCasedParseFilePath.c_str(), content))
	{
		return false;
	}
	
#ifdef ENABLE_TREESITTER_PARSER_DEBUG
	result.AddDebugTime("LoadFileContent");
#endif
	
	// 解析文件内容
	bool success = ParseFileContent(
		request.lowerCasedParseFilePath,
		content,
		request.language,
		result
	);
	
#ifdef ENABLE_TREESITTER_PARSER_DEBUG
	result.AddDebugTime("ParseFileContent");
#endif
	
	return success;
}

bool CTreeSitterParser::ParseFileContent(
	const std::string& filePath,
	const std::string& content,
	Language language,
	ParseResult& result)
{
	// 获取语言支持
	auto languageSupport = CLanguageFactory::Instance().GetLanguageSupport(language);
	if (!languageSupport)
	{
		return false;
	}
	
	// 创建TreeSitter解析器
	TSParser* parser = ts_parser_new();
	if (!parser)
	{
		return false;
	}
	
	// 设置语言
	const TSLanguage* tsLanguage = languageSupport->GetTSLanguage();
	if (!tsLanguage)
	{
		ts_parser_delete(parser);
		return false;
	}
	
	ts_parser_set_language(parser, tsLanguage);
	
	// 解析代码
	TSTree* tree = ts_parser_parse_string(
		parser,
		nullptr,
		content.c_str(),
		(uint32_t)content.length()
	);
	
	if (!tree)
	{
		ts_parser_delete(parser);
		return false;
	}
	
	// 提取符号
	std::vector<RawSymbolDefine> symbols;
	bool success = ExtractSymbolsFromTree(parser, tree, content, language, symbols);
	
	if (success)
	{
		result.definesByFile[filePath] = std::move(symbols);
		result.fileTimes[filePath] = Utils::GetFileTimeT(filePath.c_str());
	}
	
	// 清理
	ts_tree_delete(tree);
	ts_parser_delete(parser);
	
	return success;
}

bool CTreeSitterParser::ExtractSymbolsFromTree(
	TSParser* parser,
	TSTree* tree,
	const std::string& sourceCode,
	Language language,
	std::vector<RawSymbolDefine>& symbols)
{
	if (!tree)
		return false;
	
	// 获取根节点
	TSNode rootNode = ts_tree_root_node(tree);
	
	// 获取语言支持
	auto languageSupport = CLanguageFactory::Instance().GetLanguageSupport(language);
	if (!languageSupport)
		return false;
	
	// 遍历语法树
	TraverseNode(rootNode, sourceCode, languageSupport, symbols, "");
	
	return true;
}

void CTreeSitterParser::TraverseNode(
	TSNode node,
	const std::string& sourceCode,
	std::shared_ptr<ILanguageSupport> languageSupport,
	std::vector<RawSymbolDefine>& symbols,
	const std::string& parentName)
{
	std::string currentPrefix = parentName;

	if (true)
	{
		std::string nodeName = languageSupport->GetNodeName(node, sourceCode);
		const char* nodeType = ts_node_type(node);
		if (nodeName == "AddApiConfiguration")
		{
			int v = 0;
			v++;
		}
	}


	// 检查是否是符号定义
	if (languageSupport->IsSymbolDefinition(node))
	{
		RawSymbolDefine symbol;
		std::string nodeName = languageSupport->GetNodeName(node, sourceCode);
		
		if (!parentName.empty() && !nodeName.empty()) {
			symbol.name = parentName + "." + nodeName;
		} else {
			symbol.name = nodeName;
		}
		
		if (!nodeName.empty()) {
			currentPrefix = symbol.name;
		}

		symbol.showName = languageSupport->GetNodeDisplayName(node, sourceCode);
		symbol.kind = languageSupport->GetSymbolKind(node);
		symbol.language = languageSupport->GetLanguage();
		symbol.lineRange = languageSupport->GetNodeLineRange(node, sourceCode);
		
		// 设置位置
		TSPoint nameStart, nameEnd;
		if (languageSupport->GetNameNodeRange(node, nameStart, nameEnd))
		{
			symbol.lineLoc.line = nameStart.row;
			symbol.lineLoc.startColumn = nameStart.column;
			symbol.lineLoc.endColumn = nameEnd.column;
		}
		else
		{
			TSPoint startPoint = ts_node_start_point(node);
			symbol.lineLoc.line = startPoint.row;
			symbol.lineLoc.startColumn = startPoint.column;
			symbol.lineLoc.endColumn = startPoint.column + nodeName.length();
		}

		if (symbol.lineRange.start > symbol.lineLoc.line)
			symbol.lineRange.start = symbol.lineLoc.line;
		
		// 添加到符号列表
		if (!symbol.name.empty())
		{
			symbols.push_back(symbol);
		}
	}
	
	// 递归遍历子节点
	uint32_t childCount = ts_node_child_count(node);
	for (uint32_t i = 0; i < childCount; i++)
	{
		TSNode child = ts_node_child(node, i);
		TraverseNode(child, sourceCode, languageSupport, symbols, currentPrefix);
	}
}

TreeSitterSymbol_End
