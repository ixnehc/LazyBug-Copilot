#pragma once

#include <vector>
#include <string>
#include <deque>
#include <map>
#include <functional>

// 节点索引类型定义
typedef int TrieNodeIndex;

struct NameIndexerData
{
	BYTE data[8];
};

class CNameIndexer
{
public:
	CNameIndexer();
	~CNameIndexer();

	void Init(int maxDepth = 8); // 添加最大深度参数，默认50层
	void Clear();

	void AddEntry(const char* name, const NameIndexerData& data);
	void RemoveEntry(const char* name, const NameIndexerData& data);

	void Query(const char* query, DWORD maxResult, std::vector<NameIndexerData>& result);
	
	// 带过滤器的查询方法
	// Filter回调返回值：pass表示该条目是否通过过滤加入结果，shouldStop表示是否要立即终止整个搜索
	struct FilterResult
	{
		bool pass;      // 该条目是否通过过滤（加入结果集）
		bool shouldStop;// 是否要立即终止整个搜索遍历
	};
	// currentResultCount：当前已加入result的条目数量
	typedef std::function<FilterResult(const NameIndexerData& data, const char* query, int currentResultCount)> NameFilterCallback;
	void Query(const char* query, DWORD maxResult, std::vector<NameIndexerData>& result, NameFilterCallback filter);

	// 文件读写功能
	bool SaveToFile(const char* filename);
	bool LoadFromFile(const char* filename);

	// 获取和设置最大深度
	int GetMaxDepth() const { return m_maxDepth; }
	void SetMaxDepth(int maxDepth) { m_maxDepth = maxDepth; }

private:
	struct TrieNode
	{
		// 使用索引而不是指针的map
		// TrieNodeIndex值是m_nodes deque中的索引
		std::map<char, TrieNodeIndex> children;

		// 在此节点结尾的名称关联的数据列表
		std::vector<NameIndexerData> data_list;

		// 不再需要析构函数，因为我们不再管理指针
	};

	// 所有节点存储在deque中，通过位置索引访问
	std::deque<TrieNode> m_nodes;

	// 树的最大深度限制
	int m_maxDepth;

	// 递归查询的辅助函数
	void recursiveQuery(TrieNodeIndex nodeIndex, DWORD& count, DWORD maxResult, std::vector<NameIndexerData>& result);
	// shouldStop：输出参数，filter要求终止搜索时置为true
	void recursiveQuery(TrieNodeIndex nodeIndex, DWORD& count, DWORD maxResult, std::vector<NameIndexerData>& result, const NameFilterCallback& filter, const char* query, bool& shouldStop);
};
