#pragma once

#include "CppSymbolDefines.h"
#include <thread>
#include <mutex>
#include <atomic>

//注意这个类里的所有文件名都为小写
class CSymbolRefsCache
{
public:
	struct Entry
	{
		CppSymbol::RawSymbolRefs refs;
		std::string fileContent;
		std::vector<std::string> lines;
		bool isDirty;
	};

	CSymbolRefsCache();
	~CSymbolRefsCache();

	//初始化,包括生成两个临时文件名
	void Init(const char *dbFolderPath);

	//等待collect线程结束,清除内容
	void Clear();

	//检测某个文件的cache是否失效
	bool CheckDirty(std::string& filePath);

	//标记某个文件的cache失效
	void SetDirty(std::string& filePath);

	//返回当前是否有collect线程正在进行
	bool IsCaching();

	//先清除文件的cache失效标记,然后启动一个线程,调用SolutionDB_CollectRef(..)来搜集该文件的Ref,Cache到对应的entry中
	//注意同一个时刻只会有一个collect线程
	bool CacheRefs(std::string& filePath, std::string& fileContent);

	struct RefSnippets
	{
		void Clear()
		{
			fileRanges.clear();
			files.clear();
		}
		void Add(const CppSymbol::RawSymbolRef::RefTarget& refTarget)
		{
			if (!refTarget.IsValid()) 
				return;
			fileRanges[refTarget.fileIndex].push_back(refTarget.lineRange);
		}

		std::unordered_map<WORD, std::vector<LineRange>> fileRanges; // 各个文件里的行的范围
		std::vector<CppSymbol::RawSymbolRefs::File> files;           // 文件信息 (路径 + 时间)
	};


	bool FindNearbyRefs(std::string& filePath, const std::vector<std::string>& lines,int lineNum, int range, RefSnippets& snippets, std::atomic<bool>& cancelFlag);

private:
	int _FindClosestLineInCache(Entry* entry, const std::vector<std::string>& lines, int lineNum, std::atomic<bool>& cancelFlag);
	bool _FindNearbyRefs(const Entry* entry, int lineNum, int range,RefSnippets& snippets, std::atomic<bool>& cancelFlag);//得到指定行邻近的行的引用的代码行


	std::unordered_map<std::string, Entry> _entries;

	std::string _dbFolderPath;

	std::string _fileContentTempFilePath;
	std::string _resultTempFilePath;

	// 线程管理相关成员变量
	std::thread _cachingThread;
	std::atomic<bool> _isCaching;
	std::mutex _entriesMutex;
};
