
#pragma once

#include <unordered_map>

#include "routing.h"

#define COST_WEIGHT	0x0fffffff
struct rtNode
{
	enum
	{
		Open,
		Closed,
		Arrive,
	};
	rtGateRef	idGate;
	int			cost;
	int			costTotal;
	int			flag;
	int			pidx;
	int			idx;
	int			il;
};

class rtSearchPool
{
public:
	void reset();
	void push(const rtRegRef&reg,rtNode &node);
	rtNode * top();
	rtNode * bottom();
	void bubbleUp(rtNode * node);
	void trickleDown(rtNode * node);
	rtNode * find(const rtRegRef &id);
	rtNode * get(int idx);
	int count() {return int(_nodeBuffer.size());}
	bool check();
	typedef std::unordered_map<rtRegRef,int>		MapGate;
	typedef MapGate::iterator				itMapGate;
private:
	std::vector<int>		_nodeList;
	std::vector<rtNode>		_nodeBuffer;
	MapGate					_mapGateRef;
};

