#include "stdh.h"

#include "ChatInputACListBuilder.h"

#include "SolutionDBApi.h"
#include "LspClient.h"
#include "stringparser/stringparser.h"
#include "Utils.h"
#include "Utils_Skill.h"
#include <algorithm>
#include <cctype>
#include <chrono>
#include <thread>


CChatInputACListBuilder::CChatInputACListBuilder()
    : _isQuerying(false), _queryId(0)
{
}

CChatInputACListBuilder::~CChatInputACListBuilder()
{
    ++_queryId; // 发送中断信号
    if (_workerThread)
    {
        if (_workerThread->joinable())
        {
            _workerThread->join(); // 在析构时必须等待线程结束
        }
        _workerThread.reset();
    }
}

void CChatInputACListBuilder::Query(const std::string& query, Context ctx)
{
	extern const char* GetOpenedDBFolderPath_utf8();
	std::string dbFolderPath = GetOpenedDBFolderPath_utf8();
	if (dbFolderPath.empty())
		return;

    // 如果上一个请求还未处理完,则Abort上一个请求
    _AbortCurrentQuery();
    
    // 确保之前的线程对象已经被清理
    if (_workerThread)
    {
        if (_workerThread->joinable())
        {
            _workerThread->detach(); // 如果还是joinable状态，先detach
        }
        _workerThread.reset(); // 重置指针
    }
    
    _currentQuery = query;
    _isQuerying = true;
    int myQueryId = _queryId.load();
    
    // 启动单独的线程来作处理,Query()函数本身立即返回
    _workerThread = std::make_unique<std::thread>(&CChatInputACListBuilder::_WorkerThreadFunc, this, dbFolderPath, query, myQueryId, ctx);
}

bool CChatInputACListBuilder::Fetch(std::string &query, std::vector<ChatInputACItem>& sortedItems)
{
	query.clear();
	sortedItems.clear();

    // Fetch结果,如果Query还在进行则返回false
    if (_isQuerying.load())
    {
        return false;
    }

    std::lock_guard<std::mutex> lock(_resultMutex);

	if (_lastCompletedQuery.empty() && _results.empty())
		return false;

	query = std::move(_lastCompletedQuery);
	sortedItems = std::move(_results);
    
    return true;
}

ChatInputACItem ConvertToACItem(const SolutionDBMsg_NameItems::Item & item)
{
	ChatInputACItem acItem;

	// 生成唯一ID
//	acItem.id = item.type + "_" + std::to_string(std::hash<std::string>{}(item.text + item.fullPath));
	acItem.id = GenWUID();
	acItem.text = item.name;
	acItem.description = item.desc;
	acItem.fullPath = Utils::GetActualFilePath(item.filePath.c_str());
	if (item.tp == SolutionDBMsg_NameItems::Item::File_)
	{
		std::string tagName;
		if (Utils::MakeSkillTagName(item.filePath.c_str(), tagName))
			acItem.text = tagName;
		else
			acItem.text = Utils::GetActualFileName(item.filePath.c_str());
	}
	if (item.tp== SolutionDBMsg_NameItems::Item::File_)
	{
		// 判断是否为图片文件
		if (Utils::IsImageFile(acItem.fullPath.c_str()))
			acItem.type = "image";
		else
			acItem.type = "file";
	}
	if (item.tp == SolutionDBMsg_NameItems::Item::Symbol)
		acItem.type = "symbol";
	acItem.kind = item.symbolKind;
	acItem.loc = item.fileLoc.lineLoc;

	return acItem;
}

void CChatInputACListBuilder::_WorkerThreadFunc(const std::string& dbFolderPath, const std::string& query, int queryId, Context ctx)
{
    auto is_aborted = [this, queryId]{ return queryId != this->_queryId.load(); };

    try
    {
        if (is_aborted()) 
			return;

		SolutionDBMsg_NameItems result;
		SolutionDB_QueryNameItems(dbFolderPath.c_str(), query.c_str(), result);

		if (is_aborted()) 
			return;

        // 转换为ChatInputACItem格式
        std::vector<ChatInputACItem> acResults;
        for (const auto& item : result.items)
        {
			acResults.push_back(ConvertToACItem(item));
        }
        if (is_aborted()) 
			return;

        // 保存结果
        {
            std::lock_guard<std::mutex> lock(_resultMutex);
            if (is_aborted()) 
				return;
            _results = std::move(acResults);
            _lastCompletedQuery = query;
        }
    }
    catch (...)
    {
        // 处理异常，确保状态正确
    }
    
    if (!is_aborted())
    {
        _isQuerying = false;
    }
}

void CChatInputACListBuilder::_AbortCurrentQuery()
{
    if (_isQuerying.load())
    {
        ++_queryId; // 递增查询ID以中断正在运行的线程
        _isQuerying = false; // 从外部看，查询已中止
        // 注意：线程的清理交给Query方法处理，避免重复处理
    }
}


// void CChatInputACListBuilder::_SearchSymbols(const std::string& query0, std::vector<ACSearchItem>& items, const std::function<bool()>& is_aborted)
// {
//     
//     // 先尝试获取已有的workspace symbols
//     LspSymbols symbols;
//     lspClient->GetWorkspaceSymbols(symbols);
// 
// 	std::string query = query0;
// 	if (query.empty())
// 		query = "A";
//     
//     // 检查是否需要请求新的symbols
//     if (symbols.key != query)
//     {
//         // 请求新的workspace symbols（这是异步的）
//         lspClient->RequestWorkspaceSymbols(query);
// 
// 		while (1)
// 		{
// 			if (is_aborted()) 
// 				return;
// 
// 			Sleep(5);
// 
// 			// 重新获取symbols
// 			lspClient->GetWorkspaceSymbols(symbols);
// 			if (symbols.key == query)
// 				break;
// 		}
//     }
//     
//     // 处理symbols
//     int count = 0;
//     const int maxSymbols = 100; // 限制符号搜索数量
//     
//     for (const auto& symbol : symbols.buf)
//     {
//         if (is_aborted()) return;
// 
//         int score = _CalculateScore(symbol.name, query);
//         if (score > 0)
//         {
//             ACSearchItem item;
//             item.text = symbol.name;
//             item.value = symbol.name;
//             
//             // 构建描述信息
//             std::string desc = symbol.containerName;
//             if (!desc.empty() && !symbol.uri.empty())
//                 desc += " - ";
//             if (!symbol.uri.empty())
//             {
//                 // 从URI中提取文件名
//                 std::string uri = symbol.uri;
//                 size_t lastSlash = uri.find_last_of('/');
//                 if (lastSlash != std::string::npos)
//                     desc += uri.substr(lastSlash + 1);
//                 else
//                     desc += uri;
//             }
//             
//             item.description = desc;
//             item.type = "symbol";
//             item.fullPath = UriToFilePath(symbol.uri);
//             
//             // 构建数据JSON
//             item.data = "{\"type\":\"symbol\",\"name\":\"" + symbol.name + 
//                        "\",\"kind\":" + std::to_string((int)symbol.kind) + 
//                        ",\"uri\":\"" + symbol.uri + 
//                        "\",\"line\":" + std::to_string(symbol.range.start.line) + 
//                        ",\"character\":" + std::to_string(symbol.range.start.character) + "}";
//             
//             item.score = score;
//             
//             items.push_back(item);
//             count++;
//             
//             if (count >= maxSymbols)
//                 break;
//         }
//     }
//}

