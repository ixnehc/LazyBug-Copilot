#pragma once

#include <vector>
#include <deque>
#include <unordered_map>
#include <unordered_set>

#include "Random/Random.h"
#include "stackcontainer/stackvector.h"

class CEelRoadNetwork 
{
private:
	// 节点结构体
	struct Node 
	{
		LevelObjID id;

		Node(LevelObjID id) : id(id) {}
	};

	// 道路结构体
	struct Road 
	{
		LevelObjID from;
		LevelObjID to;
		BOOL bBroken; // 表示道路是否被破坏

		Road(LevelObjID from, LevelObjID to) : from(from), to(to), bBroken(TRUE) {} // 默认新加入的道路是破坏状态
	};

	// 检查节点是否存在
	BOOL NodeExists(LevelObjID id) const 
	{
		for (std::vector<Node>::const_iterator it = _nodes.begin(); it != _nodes.end(); ++it) 
		{
			if (it->id == id) 
			{
				return TRUE; // 节点已存在
			}
		}
		return FALSE; // 节点不存在
	}

	std::vector<Node> _nodes; // 存储所有节点
	std::vector<Road> _roads; // 存储所有道路

public:

	BOOL IsEmpty()
	{
		return _nodes.size()<=0&&_roads.size()<=0;
	}

	// 添加节点
	BOOL AddNode(LevelObjID id) 
	{
		// 使用完整的vector迭代器遍历所有节点
		for (std::vector<Node>::const_iterator it = _nodes.begin(); it != _nodes.end(); ++it) 
		{
			if (it->id == id) 
			{
				return FALSE; // 节点已存在，返回FALSE
			}
		}
		_nodes.push_back(Node(id));
		return TRUE; // 添加成功，返回TRUE
	}

	// 添加道路
	BOOL AddRoad(LevelObjID from, LevelObjID to) 
	{
		// 检查节点是否存在
		BOOL fromExists = FALSE, toExists = FALSE;
		for (std::vector<Node>::const_iterator it = _nodes.begin(); it != _nodes.end(); ++it) 
		{
			if (it->id == from) 
			{
				fromExists = TRUE;
			}
			if (it->id == to) 
			{
				toExists = TRUE;
			}
			if (fromExists && toExists) 
			{
				break;
			}
		}
		if (!fromExists || !toExists) 
		{
			return FALSE; // 节点不存在，返回FALSE
		}

		// 检查道路是否已存在
		for (std::vector<Road>::const_iterator it = _roads.begin(); it != _roads.end(); ++it) 
		{
			if ((it->from == from && it->to == to) || (it->from == to && it->to == from)) 
			{
				return FALSE; // 道路已存在，返回FALSE
			}
		}

		// 添加道路
		_roads.push_back(Road(from, to));
		return TRUE; // 添加成功，返回TRUE
	}

	// 重置网络
	void Reset() 
	{
		_nodes.clear();
		_roads.clear();
	}

	LevelObjID FindReparingTarget(LevelObjID currentNode)
	{
		StackVector<LevelObjID, 32> neighbours;
		// 检查当前节点是否已经在一条被打断的道路上
		for (std::vector<Road>::const_iterator it = _roads.begin(); it != _roads.end(); ++it)
		{
			const Road& road = *it; // 获取当前道路的引用

			if (road.bBroken && (road.from == currentNode || road.to == currentNode))
			{
				if (road.from == currentNode)
					neighbours.push_back(road.to);
				if (road.to == currentNode)
					neighbours.push_back(road.from);
			}
		}
		if (neighbours.size() <= 0)
			return LevelObjID_Invalid;
		return neighbours[CSysRandom::RandRangeInt<int>(0, (int)neighbours.size())];
	}

	LevelObjID FindNextStepForRepairingRoad(LevelObjID currentNode) const 
	{
		// 检查当前节点是否存在
		BOOL nodeExists = FALSE;
		for (std::vector<Node>::const_iterator it = _nodes.begin(); it != _nodes.end(); ++it) 
		{
			if (it->id == currentNode) 
			{
				nodeExists = TRUE;
				break;
			}
		}
		if (!nodeExists) 
		{
			return LevelObjID_Invalid; // 当前节点不存在
		}

		// 检查当前节点是否已经在一条被打断的道路上
		for (std::vector<Road>::const_iterator it = _roads.begin(); it != _roads.end(); ++it) 
		{
			const Road& road = *it; // 获取当前道路的引用

			if (road.bBroken && (road.from == currentNode || road.to == currentNode)) 
			{
				// 如果当前节点已经在一条被打断的道路上，直接返回当前节点本身
				return currentNode;
			}
		}

		// 使用广度优先搜索 (BFS) 找到最近的被打断的道路
		std::deque<LevelObjID> queue;
		std::unordered_set<LevelObjID> visited;
		std::unordered_map<LevelObjID, LevelObjID> parent;

		queue.push_back(currentNode);
		visited.insert(currentNode);
		parent[currentNode] = LevelObjID_Invalid; // 当前节点没有父节点

		while (!queue.empty()) 
		{
			LevelObjID node = queue.front();
			queue.pop_front();

			// 遍历所有道路，找到与当前节点相连的道路
			for (std::vector<Road>::const_iterator it = _roads.begin(); it != _roads.end(); ++it) 
			{
				const Road& road = *it; // 获取当前道路的引用

				if (road.from == node || road.to == node) 
				{
					LevelObjID neighbor = (road.from == node) ? road.to : road.from;

					// 如果道路被打断，回溯到修路工人的当前位置
					if (road.bBroken) 
					{
						// 回溯路径，找到修路工人下一步应该去往的节点
						LevelObjID nextNode = node;
						while (parent.find(nextNode) != parent.end() && parent[nextNode] != currentNode) 
						{
							nextNode = parent[nextNode];
						}
						return nextNode; // 返回修路工人下一步应该去往的节点
					}

					// 如果邻居节点未被访问，加入队列
					if (visited.find(neighbor) == visited.end()) 
					{
						visited.insert(neighbor);
						parent[neighbor] = node; // 记录父节点
						queue.push_back(neighbor);
					}
				}
			}
		}

		// 如果没有找到被打断的道路，返回LevelObjID_Invalid表示没有可修复的道路
		return LevelObjID_Invalid;
	}

	// 检查当前节点是否处于一条被打断的道路的一端
	BOOL CanRepair(LevelObjID currentNode, LevelObjID& otherNode) const 
	{
		// 检查当前节点是否存在
		if (!NodeExists(currentNode)) 
		{
			return FALSE; // 当前节点不存在
		}

		// 存储所有符合条件的道路
		std::vector<LevelObjID> possibleNodes;

		// 遍历所有道路，检查是否有被打断的道路连接到当前节点
		for (std::vector<Road>::const_iterator it = _roads.begin(); it != _roads.end(); ++it) 
		{
			const Road& road = *it; // 获取当前道路的引用

			if (road.bBroken && (road.from == currentNode || road.to == currentNode)) 
			{
				// 将这条道路的另一端节点加入到可能的节点列表中
				possibleNodes.push_back((road.from == currentNode) ? road.to : road.from);
			}
		}

		// 如果没有符合条件的道路，返回FALSE
		if (possibleNodes.empty()) 
		{
			return FALSE; // 当前节点不处于任何被打断的道路的一端
		}

		int randomIndex = CSysRandom::RandRangeInt<int>(0,possibleNodes.size());

		otherNode = possibleNodes[randomIndex]; // 选择随机节点

		return TRUE; // 返回TRUE，表示找到了一个可以修复的道路
	}

	// 修补当前节点到传入节点之间的道路
	BOOL Repair(LevelObjID currentNode, LevelObjID targetNode) 
	{
		// 使用完整的vector迭代器遍历所有道路
		for (std::vector<Road>::iterator it = _roads.begin(); it != _roads.end(); ++it) 
		{
			Road& road = *it; // 获取当前道路的引用

			// 检查是否有连接当前节点和目标节点的道路
			if (road.bBroken && 
				((road.from == currentNode && road.to == targetNode) || 
				(road.from == targetNode && road.to == currentNode))) 
			{
				road.bBroken = FALSE; // 修补道路
				return TRUE; // 修补成功，返回TRUE
			}
		}
		return FALSE; // 道路不存在或未被打断，返回FALSE
	}
	BOOL Break(LevelObjID currentNode, LevelObjID targetNode)
	{
		// 使用完整的vector迭代器遍历所有道路
		for (std::vector<Road>::iterator it = _roads.begin(); it != _roads.end(); ++it)
		{
			Road& road = *it; // 获取当前道路的引用

			// 检查是否有连接当前节点和目标节点的道路
			if (!road.bBroken &&
				((road.from == currentNode && road.to == targetNode) ||
					(road.from == targetNode && road.to == currentNode)))
			{
				road.bBroken = TRUE; 
				return TRUE; 
			}
		}
		return FALSE; 
	}

};