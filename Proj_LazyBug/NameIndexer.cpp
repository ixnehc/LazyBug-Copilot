#include "stdh.h"
#include "NameIndexer.h"
#include <map>
#include <algorithm>
#include <cctype>
#include <fstream>

#include "Utils_File.h"


// --- CNameIndexer 实现 ---

CNameIndexer::CNameIndexer()
	: m_maxDepth(8) // 默认最大深度为50
{
}

CNameIndexer::~CNameIndexer()
{
	Clear();
}

void CNameIndexer::Init(int maxDepth)
{
	// 在初始化新树之前清除任何现有的树
	Clear();
	// 设置最大深度
	m_maxDepth = maxDepth;
	// 在索引0处创建根节点
	m_nodes.emplace_back();
}

void CNameIndexer::Clear()
{
	m_nodes.clear();
}

void CNameIndexer::AddEntry(const char* name, const NameIndexerData& data)
{
	if (m_nodes.empty() || !name)
	{
		return;
	}

	TrieNodeIndex currentIndex = 0; // 从根节点开始（索引0）
	int currentDepth = 0; // 当前深度计数器
	
	for (const char* p = name; *p; ++p)
	{
		// 检查是否超过最大深度限制
		if (currentDepth >= m_maxDepth)
		{
			break; // 停止添加更深的字符
		}
		
		char c = std::tolower(*p); // 不区分大小写
		auto& currentNode = m_nodes[currentIndex];
		if (currentNode.children.find(c) == currentNode.children.end())
		{
			// 在deque末尾创建新节点
			TrieNodeIndex newIndex = static_cast<TrieNodeIndex>(m_nodes.size());
			m_nodes.emplace_back();
			currentNode.children[c] = newIndex;
		}
		currentIndex = currentNode.children[c];
		currentDepth++; // 增加深度计数
	}

	// 将数据添加到最终节点的列表中
	m_nodes[currentIndex].data_list.push_back(data);
}

void CNameIndexer::RemoveEntry(const char* name, const NameIndexerData& data)
{
	if (m_nodes.empty() || !name)
	{
		return;
	}

	TrieNodeIndex currentIndex = 0; // 从根节点开始（索引0）
	int currentDepth = 0; // 当前深度计数器
	
	for (const char* p = name; *p; ++p)
	{
		// 检查是否超过最大深度限制
		if (currentDepth >= m_maxDepth)
		{
			break; // 停止搜索更深的字符
		}
		
		char c = std::tolower(*p); // 不区分大小写
		auto& currentNode = m_nodes[currentIndex];
		auto it = currentNode.children.find(c);
		if (it == currentNode.children.end())
		{
			// 名称在trie中不存在
			return;
		}
		currentIndex = it->second;
		currentDepth++; // 增加深度计数
	}

	// 查找并移除特定的数据条目
	// 我们需要比较NameIndexerData的内容
	auto& dataList = m_nodes[currentIndex].data_list;
	dataList.erase(
		std::remove_if(dataList.begin(), dataList.end(),
			[&](const NameIndexerData& d) {
				return memcmp(d.data, data.data, sizeof(d.data)) == 0;
			}),
		dataList.end());
	
	// 注意：我们不修剪空节点以保持索引稳定性
	// 这对文件序列化很重要
}

void CNameIndexer::Query(const char* query, DWORD maxResult, std::vector<NameIndexerData>& result)
{
	Query(query, maxResult, result, nullptr);
}

void CNameIndexer::Query(const char* query, DWORD maxResult, std::vector<NameIndexerData>& result, NameFilterCallback filter)
{
	result.clear();
	if (m_nodes.empty() || !query || maxResult == 0)
	{
		return;
	}

	TrieNodeIndex currentIndex = 0; // 从根节点开始（索引0）
	int currentDepth = 0; // 当前深度计数器
	
	for (const char* p = query; *p; ++p)
	{
		// 检查是否超过最大深度限制
		if (currentDepth >= m_maxDepth)
		{
			break; // 停止搜索更深的字符
		}
		
		char c = std::tolower(*p); // 不区分大小写
		auto& currentNode = m_nodes[currentIndex];
		auto it = currentNode.children.find(c);
		if (it == currentNode.children.end())
		{
			// 前缀不存在，没有结果
			return;
		}
		currentIndex = it->second;
		currentDepth++; // 增加深度计数
	}

	// 前缀有效。现在，从此节点及其子节点收集所有结果
	DWORD count = 0;
	bool shouldStop = false;
	recursiveQuery(currentIndex, count, maxResult, result, filter, query, shouldStop);
}

void CNameIndexer::recursiveQuery(TrieNodeIndex nodeIndex, DWORD& count, DWORD maxResult, std::vector<NameIndexerData>& result)
{
	bool shouldStop = false;
	recursiveQuery(nodeIndex, count, maxResult, result, nullptr, nullptr, shouldStop);
}

void CNameIndexer::recursiveQuery(TrieNodeIndex nodeIndex, DWORD& count, DWORD maxResult, std::vector<NameIndexerData>& result, const NameFilterCallback& filter, const char* query, bool& shouldStop)
{
	if (shouldStop || nodeIndex >= static_cast<TrieNodeIndex>(m_nodes.size()))
	{
		return;
	}

	auto& node = m_nodes[nodeIndex];

	// 添加当前节点的所有数据
	for (const auto& data : node.data_list)
	{
		if (shouldStop)
			return;

		// 如果有过滤器，先进行过滤
		if (filter)
		{
			FilterResult fr = filter(data, query, static_cast<int>(count));
			if (fr.shouldStop)
			{
				shouldStop = true;
				return;
			}
			if (!fr.pass)
				continue; // 过滤器要求跳过这个数据
		}
		else if (count >= maxResult)
		{
			// 没有过滤器时，超过上限则直接停止
			return;
		}

		result.push_back(data);
		count++;
	}

	// 递归调用所有子节点
	for (auto const& pair : node.children)
	{
		if (shouldStop)
			return;
		recursiveQuery(pair.second, count, maxResult, result, filter, query, shouldStop);
	}
}

bool CNameIndexer::SaveToFile(const char* filename)
{
	if (!filename)
	{
		return false;
	}

	std::ofstream file;
	Utils::OpenOFStream(file, filename);

	if (!file.is_open())
	{
		return false;
	}

	// 写入最大深度
	file.write(reinterpret_cast<const char*>(&m_maxDepth), sizeof(m_maxDepth));

	// 写入节点数量
	size_t nodeCount = m_nodes.size();
	file.write(reinterpret_cast<const char*>(&nodeCount), sizeof(nodeCount));

	// 写入每个节点
	for (const auto& node : m_nodes)
	{
		// 写入子节点map
		size_t childrenCount = node.children.size();
		file.write(reinterpret_cast<const char*>(&childrenCount), sizeof(childrenCount));
		for (const auto& pair : node.children)
		{
			file.write(reinterpret_cast<const char*>(&pair.first), sizeof(pair.first));
			file.write(reinterpret_cast<const char*>(&pair.second), sizeof(pair.second));
		}

		// 写入数据列表
		size_t dataCount = node.data_list.size();
		file.write(reinterpret_cast<const char*>(&dataCount), sizeof(dataCount));
		for (const auto& data : node.data_list)
		{
			file.write(reinterpret_cast<const char*>(&data), sizeof(data));
		}
	}

	return file.good();
}

bool CNameIndexer::LoadFromFile(const char* filename)
{
	if (!filename)
	{
		return false;
	}

	std::ifstream file;
	Utils::OpenIFStream(file, filename);
	if (!file.is_open())
	{
		return false;
	}

	// 清除现有数据
	Clear();

	// 读取最大深度
	file.read(reinterpret_cast<char*>(&m_maxDepth), sizeof(m_maxDepth));
	if (!file.good())
	{
		return false;
	}

	// 读取节点数量
	size_t nodeCount;
	file.read(reinterpret_cast<char*>(&nodeCount), sizeof(nodeCount));
	if (!file.good())
	{
		return false;
	}

	// 预留空间并读取每个节点
	m_nodes.resize(nodeCount);
	for (size_t i = 0; i < nodeCount; ++i)
	{
		auto& node = m_nodes[i];

		// 读取子节点map
		size_t childrenCount;
		file.read(reinterpret_cast<char*>(&childrenCount), sizeof(childrenCount));
		if (!file.good())
		{
			return false;
		}

		for (size_t j = 0; j < childrenCount; ++j)
		{
			char key;
			TrieNodeIndex value;
			file.read(reinterpret_cast<char*>(&key), sizeof(key));
			file.read(reinterpret_cast<char*>(&value), sizeof(value));
			if (!file.good())
			{
				return false;
			}
			node.children[key] = value;
		}

		// 读取数据列表
		size_t dataCount;
		file.read(reinterpret_cast<char*>(&dataCount), sizeof(dataCount));
		if (!file.good())
		{
			return false;
		}

		node.data_list.resize(dataCount);
		for (size_t j = 0; j < dataCount; ++j)
		{
			file.read(reinterpret_cast<char*>(&node.data_list[j]), sizeof(NameIndexerData));
			if (!file.good())
			{
				return false;
			}
		}
	}

	return true;
}

