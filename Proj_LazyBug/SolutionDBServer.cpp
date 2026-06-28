#include "stdh.h"
#include "SolutionDBServer.h"
#include "SolutionDBs.h"
#include "SolutionDBMsgs.h"
#include "CppSymbol.h"
#include "datapacket/DataPacket.h"
#include "stringparser/stringparser.h"
#include "Utils.h"
#include "Utils_SearchFile.h"
#include "utils_skill.h"
#include <vector>
#include <regex>
#include <algorithm>

CSolutionDBs g_solutionDBs;

//////////////////////////////////////////////////////////////////////////
//CSolutionDBServer

CSolutionDBServer::CSolutionDBServer(HANDLE hRequestPipe, HANDLE hResponsePipe) : _hRequestPipe(hRequestPipe), _hResponsePipe(hResponsePipe)
{
}

CSolutionDBServer::~CSolutionDBServer()
{
	// 首先，通过关闭管道句柄来中断工作线程的 ReadFile 调用
	// 这是为了确保工作线程不会永远阻塞，从而允许我们安全地 join 它
	if (_hRequestPipe != INVALID_HANDLE_VALUE)
	{
		CancelIoEx(_hRequestPipe, NULL);

		// DisconnectNamedPipe 确保服务器端连接已断开
		DisconnectNamedPipe(_hRequestPipe);
		CloseHandle(_hRequestPipe);
		// 将句柄设为无效，防止重复关闭
		_hRequestPipe = INVALID_HANDLE_VALUE;
	}

	if (_hResponsePipe != INVALID_HANDLE_VALUE)
	{
		CancelIoEx(_hResponsePipe, NULL);

		// DisconnectNamedPipe 确保服务器端连接已断开
		DisconnectNamedPipe(_hResponsePipe);
		CloseHandle(_hResponsePipe);
		// 将句柄设为无效，防止重复关闭
		_hResponsePipe = INVALID_HANDLE_VALUE;
	}

	// 现在工作线程已经被解除阻塞并即将退出（或已经退出）
	// 我们可以安全地等待它结束
	if (_thread.joinable())
	{
		_thread.join();
	}
}

// 辅助函数：减少 NameItems 结果中的项目数量
static void ReduceNameItems(SolutionDBMsg_NameItems& result, size_t keepCount)
{
	if (keepCount < 1 && !result.items.empty())
		keepCount = 1;
	if (keepCount < result.items.size())
		result.items.resize(keepCount);
}

// 辅助函数：减少 SymbolDefines 结果中的项目数量
static void ReduceSymbolDefines(SolutionDBMsg_SymbolDefines& result, size_t keepCount)
{
	if (keepCount < 1 && !result.locations.empty())
		keepCount = 1;
	if (keepCount < result.locations.size())
		result.locations.resize(keepCount);
}

// 辅助函数：减少 FindInFilesResults 结果中的项目数量
static void ReduceFindInFilesResults(SolutionDBMsg_FindInFilesResults& result, size_t keepCount)
{
	if (keepCount < 1 && !result.results.fileInfos.empty())
		keepCount = 1;
	if (keepCount < result.results.fileInfos.size())
		result.results.fileInfos.resize(keepCount);
}

// 辅助函数：减少 SearchFileResult 结果中的项目数量
static void ReduceSearchFileResult(SolutionDBMsg_SearchFileResult& result, size_t keepCount)
{
	if (keepCount < 1 && !result.results.fileInfos.empty())
		keepCount = 1;
	if (keepCount < result.results.fileInfos.size())
		result.results.fileInfos.resize(keepCount);
}


void CSolutionDBServer::Run()
{
	_thread = std::thread([this]()
	{
		const int BUFFER_SIZE = 16 * 1024; // 256k
		std::vector<char> buffer(BUFFER_SIZE);
		DWORD bytesRead = 0;

		// Loop while the pipe is connected and read operations are successful
		while (ReadFile(_hRequestPipe, buffer.data(), buffer.size(), &bytesRead, NULL))
		{
			if (bytesRead == 0)
			{
				continue; // Should not happen with message pipes, but good practice
			}

			// If ReadFile returns true but bytesRead equals buffer size, 
			// the message might have been truncated. We assume this is an error and break.
			if (bytesRead >= BUFFER_SIZE)
			{
				// Log error: message was likely truncated.
				break;
			}

			// 创建请求数据的副本
			std::vector<char> requestData(buffer.begin(), buffer.begin() + bytesRead);

			// 在新线程中处理请求
			std::thread requestThread([this, requestData = std::move(requestData)]()
			{
				CDataPacket dp;
				dp.SetDataBufferPointer((unsigned char*)requestData.data());

				unsigned int requestId;
				dp.Data_ReadSimple(requestId);

				PipeMsgType msgId;
				dp.Data_ReadSimple(msgId);

				auto msg = CreateSolutionDBMsg(msgId);
				if (!msg)
				{
					return;
				}

				msg->Load(dp);

				SolutionDBMsgType msgType = (SolutionDBMsgType)msg->GetType();
				if (msgType == SolutionDBMsgType::RequestOpen)
				{
					auto* request = static_cast<SolutionDBMsg_RequestOpen*>(msg.get());

					CSolutionDB* db = g_solutionDBs.Obtain(request->dbFolderPath.c_str(), request->slnPath.c_str());

					SolutionDBMsg_Opened response;
					response.success = (db != nullptr && !db->IsEmpty());
					response.dbFolderPath = request->dbFolderPath;

					SendMessage(response, requestId);
				}

				if (msgType == SolutionDBMsgType::QueryNameItems)
				{
					SolutionDBMsg_NameItems result;
					auto* request = static_cast<SolutionDBMsg_QueryNameItems*>(msg.get());
					if (request)
					{
						_QueryNameItems(*request, result);
					}

					// 发送失败时逐渐减少结果数量
					size_t keepCount = result.items.size();
					size_t reduceStep = keepCount / 8;
					if (reduceStep < 1) reduceStep = 1;
					while (!SendMessage(result, requestId) && keepCount > 0)
					{
						keepCount -= reduceStep;
						ReduceNameItems(result, keepCount);
					}
				}

				if (msgType == SolutionDBMsgType::CollectRefs)
				{
					auto* request = static_cast<SolutionDBMsg_CollectRefs*>(msg.get());
					if (request)
					{
						SolutionDBMsg_Refs result;
						_CollectRefs(*request, result);
						SendMessage(result, requestId);
					}
				}

				if (msgType == SolutionDBMsgType::FindSymbolDefine)
				{
					auto* request = static_cast<SolutionDBMsg_FindSymbolDefine*>(msg.get());
					if (request)
					{
						SolutionDBMsg_SymbolDefines result;
						_FindSymbolDefine(*request, result);

						// 发送失败时逐渐减少结果数量
						size_t keepCount = result.locations.size();
						size_t reduceStep = keepCount / 8;
						if (reduceStep < 1) reduceStep = 1;
						while (!SendMessage(result, requestId) && keepCount > 0)
						{
							keepCount -= reduceStep;
							ReduceSymbolDefines(result, keepCount);
						}
					}
				}

				if (msgType == SolutionDBMsgType::FindInFiles)
				{
					auto* request = static_cast<SolutionDBMsg_FindInFiles*>(msg.get());
					if (request)
					{
						SolutionDBMsg_FindInFilesResults result;
						_FindInFiles(*request, result);

						// 发送失败时逐渐减少结果数量
						size_t keepCount = result.results.fileInfos.size();
						size_t reduceStep = keepCount / 8;
						if (reduceStep < 1) reduceStep = 1;
						while (!SendMessage(result, requestId) && keepCount > 0)
						{
							keepCount -= reduceStep;
							ReduceFindInFilesResults(result, keepCount);
						}
					}
				}

				if (msgType == SolutionDBMsgType::SearchFile)
				{
					auto* request = static_cast<SolutionDBMsg_SearchFile*>(msg.get());
					if (request)
					{
						SolutionDBMsg_SearchFileResult result;
						_SearchFile(*request, result);

						// 发送失败时逐渐减少结果数量
						size_t keepCount = result.results.fileInfos.size();
						size_t reduceStep = keepCount / 8;
						if (reduceStep < 1) reduceStep = 1;
						while (!SendMessage(result, requestId) && keepCount > 0)
						{
							keepCount -= reduceStep;
							ReduceSearchFileResult(result, keepCount);
						}
					}
				}

				if (msgType == SolutionDBMsgType::SetEmbeddingModel)
				{
					auto* request = static_cast<SolutionDBMsg_SetEmbeddingModel*>(msg.get());
					if (request)
					{
						SolutionDBMsg_EmbeddingModelSet response;
						_SetEmbeddingModel(*request, response);
						SendMessage(response, requestId);
					}
				}

			});

			// 分离线程，让它独立运行
			requestThread.detach();
		}

		// ReadFile 失败，检查是否是客户端断开
		DWORD error = GetLastError();
		if (error == ERROR_BROKEN_PIPE)
		{
			std::lock_guard<std::mutex> lock(_disconnectMutex);
			_isDisconnected = true;
			_lastDisconnectedTime = std::chrono::steady_clock::now();
		}
	});
}

bool CSolutionDBServer::SendMessage(const PipeMsg& msg, unsigned int requestId)
{
	// Serialize message payload directly into a buffer
	std::vector<BYTE> payloadBuffer;
	DP_BeginSave(dp, payloadBuffer)
	{
		dp.Data_WriteSimple(requestId);
		dp.Data_WriteSimple(msg.GetType());
		msg.Save(dp);
	}
	DP_EndSave();

	const int BUFFER_SIZE = 256 * 1024 - 16; // 256k,与client的接受buffer大小一致 (CSolutionDBClient::ReaderLoop())

	if (payloadBuffer.size() >= BUFFER_SIZE)
		return false;

	// Write the payload buffer directly as a single message
	DWORD bytesWritten;
	std::lock_guard<std::mutex> lock(_pipeMutex);
	WriteFile(_hResponsePipe, payloadBuffer.data(), payloadBuffer.size(), &bytesWritten, NULL);
	return true;
}


int CSolutionDBServer::_CalculateNameScore(const std::string& text, const std::string& query)
{
	if (query.empty())
		return 100;

	if (text.empty())
		return 0;

	// 检查大小写完全匹配的情况（最高分数）
	if (text == query)
		return 2000; // 大小写完全匹配得分最高

	// 检查大小写前缀匹配
	if (text.length() >= query.length() && text.substr(0, query.length()) == query)
		return 1500 + (100 - (int)query.length()); // 大小写前缀匹配得高分

	// 检查大小写包含匹配
	size_t exactPos = text.find(query);
	if (exactPos != std::string::npos)
	{
		return 1200 - (int)exactPos + (100 - (int)query.length()); // 大小写包含匹配
	}

	// 转换为小写进行比较
	std::string lowerText = text;
	std::string lowerQuery = query;
	std::transform(lowerText.begin(), lowerText.end(), lowerText.begin(), ::tolower);
	std::transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

	// 小写完全匹配
	if (lowerText == lowerQuery)
		return 1000;

	// 小写前缀匹配
	if (lowerText.find(lowerQuery) == 0)
		return 800 + (100 - (int)lowerQuery.length());

	// 小写包含匹配
	size_t pos = lowerText.find(lowerQuery);
	if (pos != std::string::npos)
	{
		return 500 - (int)pos + (100 - (int)lowerQuery.length());
	}

	// 模糊匹配（检查是否包含查询字符串的所有字符）
	size_t textPos = 0;
	size_t queryPos = 0;
	int matchedChars = 0;
	std::vector<size_t> matchPositions;

	while (textPos < lowerText.length() && queryPos < lowerQuery.length())
	{
		if (lowerText[textPos] == lowerQuery[queryPos])
		{
			matchedChars++;
			matchPositions.push_back(textPos);
			queryPos++;
		}
		textPos++;
	}

	if (queryPos == lowerQuery.length()) // 所有查询字符都找到了
	{
		// 计算字符间的距离，距离越近分数越高
		int distancePenalty = 0;
		if (matchPositions.size() > 1)
		{
			for (size_t i = 1; i < matchPositions.size(); ++i)
			{
				distancePenalty += (int)(matchPositions[i] - matchPositions[i - 1] - 1);
			}
		}

		return 200 + matchedChars * 10 - distancePenalty;
	}

	return 0; // 不匹配
}

void CSolutionDBServer::_SortAndLimitResults(std::vector<SolutionDBMsg_NameItems::Item>& items, int maxCount)
{
	// 按照相似度分数降序排序
	std::sort(items.begin(), items.end(),
		[](const SolutionDBMsg_NameItems::Item& a, const SolutionDBMsg_NameItems::Item& b) {
		if (a.score != b.score)
			return a.score > b.score; // 分数高的排前面
		return a.name < b.name; // 分数相同时按字母顺序
	});

	// 限制结果数量
	if ((int)items.size() > maxCount)
	{
		items.resize(maxCount);
	}
}

bool CSolutionDBServer::_QueryNameItems(const SolutionDBMsg_QueryNameItems& request, SolutionDBMsg_NameItems& result)
{
	CSolutionDB* db = g_solutionDBs.Obtain(request.dbFolderPath.c_str());
	if (!db)
		return false;

	const std::string query = request.query;

	//File names
	if (true)
	{
		const CSolutionFiles& files = db->GetFiles();
		CSolutionFiles::ReadLock lock(files._filesMutex);

		const int maxFiles = 50;
		int count = 0;

		for (const auto& it : files._lowerCasedFiles)
		{
			const auto& fileInfo = it.second;

			// 计算文件名与query的相似度
			int score = _CalculateNameScore(fileInfo.fileName, query);
			if (score > 0)
			{
				SolutionDBMsg_NameItems::Item item;
				item.name = fileInfo.fileName;
				item.tp = SolutionDBMsg_NameItems::Item::File_;
				item.desc = "";
				item.filePath = fileInfo.filePath;
				item.score = score;

				result.items.push_back(std::move(item));
				count++;

				if (count >= maxFiles)
					break;
			}
		}

		// 如果搜索结果不足，在 skills 目录中继续搜索
		if (count < maxFiles)
		{
			Utils::EnumFilesInSkillFolder(request.dbFolderPath.c_str(),
				[&](const char* filePath, CLlmSkills::Skill::Type type) {
					// 如果已经达到最大结果数，停止遍历
					if (count >= maxFiles) 
						return Utils::EnumFilesInSkillFolderFilter::Stop;
					
					// 提取文件名
					if (!StringEqualNoCase(GetFileName(filePath).c_str(), "skill.md"))
						return Utils::EnumFilesInSkillFolderFilter::Reject;
					std::string fileName = GetFileFolderPath(filePath);
					fileName = GetFileTitle(fileName);
					
					// 计算文件名与query的相似度
					int score = _CalculateNameScore(fileName, query);
					int score2= _CalculateNameScore("skill.md", query);
					if (score2 > score)
						score = score2;
					if (score > 0)
					{
						SolutionDBMsg_NameItems::Item item;
						if (false == Utils::MakeSkillTagName(filePath, item.name))
							item.name = GetFileName(filePath);
						item.tp = SolutionDBMsg_NameItems::Item::File_;
						item.desc = "";
						item.filePath = filePath;
						item.score = score;

						result.items.push_back(std::move(item));
						count++;

						return Utils::EnumFilesInSkillFolderFilter::Accept;
					}
					return Utils::EnumFilesInSkillFolderFilter::Reject;
				});
		}
	}

	//symbols
	if (true)
	{
		std::vector<CppSymbol::SymbolDefineQueryResult> symbolResults;
		db->_symbolDB.QuerySymbolDefine(query.c_str(), 128, symbolResults);

		for (int i = 0;i < symbolResults.size();i++)
		{
			int score = _CalculateNameScore(symbolResults[i].name, query);
			if (score > 0)
			{
				SolutionDBMsg_NameItems::Item item;
				item.name = symbolResults[i].name;
				item.tp = SolutionDBMsg_NameItems::Item::Symbol;
				item.desc = symbolResults[i].desc;
				item.fileLoc = symbolResults[i].location;
				item.symbolKind = (char)symbolResults[i].kind;
				item.score = score;

				result.items.push_back(std::move(item));
			}
		}
	}

	//symbols2
	if (true)
	{
		std::vector<TreeSitterSymbol::SymbolDefineQueryResult> symbolResults;
		db->_symbolDB2.QuerySymbolDefine(query.c_str(), 128, symbolResults);

		for (int i = 0;i < symbolResults.size();i++)
		{
			int score = _CalculateNameScore(symbolResults[i].name, query);
			if (score > 0)
			{
				SolutionDBMsg_NameItems::Item item;
				item.name = symbolResults[i].name;
				item.tp = (SolutionDBMsg_NameItems::Item::Type)20;//Hack, 表示这个symbol在_symbolDB2里
				item.desc = symbolResults[i].desc;
				item.fileLoc = symbolResults[i].location;
				item.symbolKind = (char)symbolResults[i].kind;
				item.score = score;

				result.items.push_back(std::move(item));
			}
		}
	}

	_SortAndLimitResults(result.items, 80);
	for (int i = 0;i < result.items.size();i++)
	{
		if (result.items[i].tp == SolutionDBMsg_NameItems::Item::Symbol)
			db->_symbolDB.GetStr(result.items[i].fileLoc.filePath, result.items[i].filePath);
		if (result.items[i].tp == (SolutionDBMsg_NameItems::Item::Type)20)
		{
			db->_symbolDB2.GetStr(result.items[i].fileLoc.filePath, result.items[i].filePath);
			result.items[i].tp = SolutionDBMsg_NameItems::Item::Symbol;
		}
	}

	result.query = request.query;
	result.dbFolderPath = request.dbFolderPath;

	return true;
}

void CSolutionDBServer::_CollectRefs(const SolutionDBMsg_CollectRefs& request, SolutionDBMsg_Refs& result)
{
	CSolutionDB* db = g_solutionDBs.Obtain(request.dbFolderPath.c_str());
	if (!db)
		return;

	result.success = db->_symbolDB.CollectRefs(request.collectRefParam);
}

void CSolutionDBServer::_FindSymbolDefine(const SolutionDBMsg_FindSymbolDefine& request, SolutionDBMsg_SymbolDefines& result)
{
	result.symbolName = request.symbolName;

	CSolutionDB* db = g_solutionDBs.Obtain(request.dbFolderPath.c_str());
	if (!db)
		return;

	if (true)
	{
		std::vector<CppSymbol::SymbolDefineFindResult> symbolResults;
		db->_symbolDB.FindSymbolDefine(request.symbolName.c_str(), request.maxResult, symbolResults);

		for (const auto& symbolResult : symbolResults)
		{
			SolutionDBMsg_SymbolDefines::Location loc;
			loc.symbolKind = (char)symbolResult.kind;
			loc.fileLoc = symbolResult.location;
			loc.lineRange = symbolResult.lineRange;

			// 获取文件完整路径
			db->_symbolDB.GetStr(symbolResult.location.filePath, loc.filePath);

			result.locations.push_back(std::move(loc));
		}
	}

	if (true)
	{
		std::vector<TreeSitterSymbol::SymbolDefineFindResult> symbolResults;
		int remainingMax = request.maxResult - (int)result.locations.size();
		if (remainingMax > 0)
		{
			db->_symbolDB2.FindSymbolDefine(request.symbolName.c_str(), remainingMax, symbolResults);

			for (const auto& symbolResult : symbolResults)
			{
				SolutionDBMsg_SymbolDefines::Location loc;
				loc.symbolKind = (char)symbolResult.kind;
				loc.fileLoc = symbolResult.location;
				loc.lineRange = symbolResult.lineRange;

				// 获取文件完整路径
				db->_symbolDB2.GetStr(symbolResult.location.filePath, loc.filePath);

				result.locations.push_back(std::move(loc));
			}
		}
	}


}

void _CollectFindInFileResultSymbols(CSolutionDB* db, FindInFileResults& findResults)
{
	std::vector<FindInFileResults::FileInfo> validFileInfos;
	for (auto& fileInfo : findResults.fileInfos)
	{
		if (fileInfo.lineInfos.empty())
			continue;

		StringLower(fileInfo.filePath);

		std::string suffix;
		suffix = GetFileSuffix(fileInfo.filePath);

		bool isCppFile = Utils::IsCppFile(suffix);
		bool isTreeSitterFile = TreeSitterSymbol::GetLanguageFromExtension(suffix) != Language::Unknown;

		if (isCppFile || isTreeSitterFile)
		{

			// 收集所有需要查询的行号（转换为0-based，因为GetSymbolNamesFromLines内部使用0-based行号）
			std::vector<int> lines;
			for (const auto& lineInfo : fileInfo.lineInfos)
			{
				lines.push_back(lineInfo.lineNumber - 1);
			}

			// 获取这些行的symbolName
			if (isCppFile)
			{
				std::vector<StringIndex> symbolNameIndices;
				bool success = db->_symbolDB.GetSymbolNamesFromLines(fileInfo.filePath.c_str(), lines, symbolNameIndices);

				if (success)
				{
					// 将symbolName索引转换为字符串并赋值
					for (size_t i = 0; i < fileInfo.lineInfos.size(); ++i)
					{
						if (i < symbolNameIndices.size() && symbolNameIndices[i] != StringIndex_Null)
						{
							std::string symbolNameStr;
							db->_symbolDB.GetStr(symbolNameIndices[i], symbolNameStr);
							fileInfo.lineInfos[i].symbolName = symbolNameStr;
						}
					}
				}
			}
			else
			{
				if (isTreeSitterFile)
				{
					std::vector<StringIndex> symbolNameIndices;
					bool success = db->_symbolDB2.GetSymbolNamesFromLines(fileInfo.filePath.c_str(), lines, symbolNameIndices);

					if (success)
					{
						// 将symbolName索引转换为字符串并赋值
						for (size_t i = 0; i < fileInfo.lineInfos.size(); ++i)
						{
							if (i < symbolNameIndices.size() && symbolNameIndices[i] != StringIndex_Null)
							{
								std::string symbolNameStr;
								db->_symbolDB2.GetStr(symbolNameIndices[i], symbolNameStr);
								fileInfo.lineInfos[i].symbolName = symbolNameStr;
							}
						}
					}
				}
			}
		}

		// 添加到有效结果列表中
		validFileInfos.push_back(std::move(fileInfo));
	}

	// 用有效的结果替换原始结果
	findResults.fileInfos = std::move(validFileInfos);
}

void CSolutionDBServer::_FindInFiles(const SolutionDBMsg_FindInFiles& request, SolutionDBMsg_FindInFilesResults& result)
{
	result.keyword = request.keyword;
	result.dbFolderPath = request.dbFolderPath;

	CSolutionDB* db = g_solutionDBs.Obtain(request.dbFolderPath.c_str());
	if (!db)
		return;

	if (db->GetSolutionIndexer().Find(request.keyword.c_str(), request.maxResults, result.results))
	{
		_CollectFindInFileResultSymbols(db, result.results);
		return;
	}

	std::vector<std::string> folderPathes;
	db->GetScanner().GetWatcherFolderPathes(folderPathes);

	FindInFileResults findResults;

	// 使用缓存来加速文件过滤判断（只保留最近一次的cache）
	std::string lastFilePath;
	bool lastFilterResult = false;

	std::string lowerCasedPath;

	Utils::FindInFile(request.keyword.c_str(), folderPathes, findResults, request.maxResults,
		[&lastFilePath, &lastFilterResult, &lowerCasedPath, db](const char* filePath) {
		// 检查是否与上一次的文件路径相同
		if (filePath == lastFilePath)
		{
			return lastFilterResult; // 返回缓存的过滤结果（true表示要过滤掉）
		}

		lastFilePath = filePath;

		// 如果文件不在数据库中，则过滤掉（返回true表示忽略该文件）
		lowerCasedPath = filePath;
		StringLower(lowerCasedPath);
		lastFilterResult = !db->_symbolDB.IsFileInDB(lowerCasedPath.c_str());
		return lastFilterResult;
	});

	_CollectFindInFileResultSymbols(db, findResults);

	result.results = std::move(findResults);
}

void CSolutionDBServer::_SearchFile(const SolutionDBMsg_SearchFile& request, SolutionDBMsg_SearchFileResult& result)
{
	result.keyword = request.keyword;
	result.dbFolderPath = request.dbFolderPath;

	CSolutionDB* db = g_solutionDBs.Obtain(request.dbFolderPath.c_str());
	if (!db)
		return;

	const CSolutionFiles& files = db->GetFiles();
	CSolutionFiles::ReadLock lock(files._filesMutex);

	const std::string keyword = request.keyword;
	std::string lowerKeyword = keyword;
	StringLower(lowerKeyword);
	std::replace(lowerKeyword.begin(), lowerKeyword.end(), '/', '\\');

	// 为了避免在 MatchFilePath 循环内部反复拼接字符串，提高匹配性能。
	bool useWildcard = Utils::HasWildcard(lowerKeyword);
	std::regex compiledRegex;

	if (useWildcard)
	{
		if (lowerKeyword.length() < 2 || lowerKeyword[0] != '*' || lowerKeyword[1] != '*')
		{
			lowerKeyword = "**" + lowerKeyword;
		}

		std::string regexStr = Utils::WildcardToRegexString(lowerKeyword);
		// lowerKeyword 已全小写，底层文件路径也使用小写缓存匹配，无需 icase 避免多字节进 tolower 导致崩溃
		compiledRegex = std::regex(regexStr);
	}

	int count = 0;
	for (const auto& it : files._lowerCasedFiles)
	{
		if (count >= request.maxResults)
			break;

		const auto& fileInfo = it.second;
		bool matched = false;

		if (useWildcard)
		{
			// 通配符：预编译的正则表达式匹配 (使用全小写缓存)
			matched = std::regex_match(fileInfo.lowerCasedFilePath, compiledRegex);
		}
		else
		{
			// 无通配符：子串包含匹配 (使用全小写缓存)
			matched = fileInfo.lowerCasedFilePath.find(lowerKeyword) != std::string::npos;
		}

		if (matched)
		{
			SearchFileResult::FileInfo resultFileInfo;
			resultFileInfo.filePath = fileInfo.filePath;
			result.results.fileInfos.push_back(std::move(resultFileInfo));
			count++;
		}
	}

	// 如果搜索结果不足，在 skills 目录中继续搜索
	if (count < request.maxResults)
	{
		Utils::EnumFilesInSkillFolder(request.dbFolderPath.c_str(),
			[&](const char* filePath, CLlmSkills::Skill::Type type) {
				// 如果已经达到最大结果数，停止遍历
				if (count >= request.maxResults)
					return Utils::EnumFilesInSkillFolderFilter::Stop;
				
				bool matched = false;
				
				// 因为正则表达式去掉了 icase，所以统一预先转成全小写路径进行匹配
				std::string lowerPath = filePath;
				StringLower(lowerPath);

				if (useWildcard)
				{
					// 通配符：预编译的正则表达式匹配
					matched = std::regex_match(lowerPath, compiledRegex);
				}
				else
				{
					// 无通配符：子串包含匹配
					matched = lowerPath.find(lowerKeyword) != std::string::npos;
				}
				
				if (matched)
				{
					SearchFileResult::FileInfo resultFileInfo;
					resultFileInfo.filePath = filePath;
					result.results.fileInfos.push_back(std::move(resultFileInfo));
					count++;
				}
				
				return Utils::EnumFilesInSkillFolderFilter::Accept;
			});
	}
}


void CSolutionDBServer::_SetEmbeddingModel(const SolutionDBMsg_SetEmbeddingModel& request, SolutionDBMsg_EmbeddingModelSet& response)
{
	response.dbFolderPath = request.dbFolderPath;

	CSolutionDB* db = g_solutionDBs.Obtain(request.dbFolderPath.c_str());
	if (!db)
	{
		response.success = false;
		return;
	}

	EmbedModelParam modelParam;
	modelParam._modelName = request.modelName;
	modelParam._endpoint = request.endpoint;
	modelParam._apiKey = request.apiKey;
	modelParam._timeoutSeconds = request.timeoutSeconds;

	db->_embeddingDB.SetModelParam(modelParam);
	response.success = true;
}


// void CSolutionDBServer::_QueryRefs(const SolutionDBMsg_QueryRefs& request, SolutionDBMsg_Refs& result)
// {
// 	CSolutionDB* db = g_solutionDBs.Obtain(request.dbFolderPath.c_str());
// 	if (!db)
// 		return;
// 
// 	std::vector<FileRange> collected;
// 	db->_symbolDB.QueryCollectedRef(
// 		request.lowerCasedFilePath.c_str(),
// 		request.lineRange.start,
// 		request.lineRange.end,
// 		collected
// 	);
// 
// 	// 按文件路径合并
// 	std::unordered_map<std::string, std::vector<LineRange>> grouped;
// 
// 	for (const auto& fr : collected)
// 	{
// 		if (fr.filePath == StringIndex_Null)
// 			continue;
// 
// 		std::string filePathStr;
// 		db->_symbolDB.GetStr(fr.filePath, filePathStr);
// 
// 		if (filePathStr.empty())
// 			continue;
// 
// 		grouped[filePathStr].push_back(fr.lineRange);
// 	}
// 
// 	result.refs.clear();
// 	result.refs.reserve(grouped.size());
// 
// 	for (auto& p : grouped)
// 	{
// 		auto& ranges = p.second;
// 
// 		// 先排序
// 		std::sort(ranges.begin(), ranges.end(),
// 			[](const LineRange& a, const LineRange& b) {
// 				return a.start < b.start || (a.start == b.start && a.end < b.end);
// 			});
// 
// 		// 合并重叠或相邻区间
// 		std::vector<LineRange> merged;
// 		for (const auto& r : ranges)
// 		{
// 			if (merged.empty() || r.start > merged.back().end)
// 			{
// 				merged.push_back(r);
// 			}
// 			else
// 			{
// 				merged.back().end = std::max<WORD>(merged.back().end, r.end);
// 			}
// 		}
// 
// 		SolutionDBMsg_Refs::Ref ref;
// 		ref.filePath = p.first;
// 		ref.lineRanges = std::move(merged);
// 		result.refs.push_back(std::move(ref));
// 	}
// }
