#pragma once
#include <vector>
#include <string>
#include <functional>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <condition_variable>

#include "ChatInputACItem.h"


// 前向声明
class CSolutionDB;
class CLspClient;

class CChatInputACListBuilder
{
public:
	struct Context
	{
		Context()
		{
		}
		std::string dbFolderPath;
	};

	CChatInputACListBuilder();
	~CChatInputACListBuilder();

	//发送请求,如果上一个请求还未处理完,则Abort上一个请求
	//根据请求,在以下范围内寻找匹配的Item:
	// CVcxprojDatabase中所有文件(_files._lowerCasedFiles), 
	// CLspClient里的 根据query能够得到的workspace symbol
	//将这些item按照它们的名称与query的相似度排序,取前10个item作为结果
	//要启动单独的线程来作处理,Query()函数本身立即返回
	void Query(const std::string& query, Context ctx);

	//Fetch结果,如果Query还在进行则返回false
	bool Fetch(std::string &query, std::vector<ChatInputACItem>& sortedItems);

private:

	// 查询状态
	std::atomic<bool> _isQuerying;
	std::string _currentQuery;
	std::string _lastCompletedQuery;
	std::atomic<int> _queryId;
	
	// 线程管理
	std::unique_ptr<std::thread> _workerThread;
	std::mutex _resultMutex;
	
	// 结果数据
	std::vector<ChatInputACItem> _results;
	
	// 工作线程函数
	void _WorkerThreadFunc(const std::string& dbFolderPath,const std::string& query, int queryId, Context ctx);
	
	// 终止当前查询
	void _AbortCurrentQuery();
	
};
