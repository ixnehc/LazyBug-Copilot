
#include "stdh.h"

#include "rtNode.h"

#include "assert.h"

void rtSearchPool::reset()
{
	_nodeBuffer.clear();
	_nodeList.clear();

	_mapGateRef.clear();
}

void rtSearchPool::push(const rtRegRef &reg,rtNode &node)
{
	int idx = (int)_nodeBuffer.size();
	int il = (int)_nodeList.size();

	node.idx = idx;
	node.il = il;

	_nodeBuffer.push_back(node);
	_mapGateRef[reg] = idx;
	_nodeList.push_back(idx);

	trickleDown(get(idx));
}

rtNode * rtSearchPool::top()
{
	rtNode * n = NULL;

	if(!_nodeList.empty())
	{
		int idx = _nodeList.back();
		n = get(idx);
	}

	return n;
}

rtNode * rtSearchPool::bottom()
{
	rtNode * n = NULL;

	if(!_nodeList.empty())
	{
		int idx = _nodeList[0];
		n = get(idx);
	}

	return n;
}

inline bool isSmall(rtNode * n0,rtNode *n1)
{
	if(n0->flag==rtNode::Closed)
		return false;
	
	return (n1->flag==rtNode::Closed||n0->costTotal<n1->costTotal);
}

inline bool isLarge(rtNode *n0,rtNode *n1)
{
	if(n1->flag==rtNode::Closed)
		return false;

	return (n0->flag==rtNode::Closed||n0->costTotal>n1->costTotal);
}

void rtSearchPool::bubbleUp(rtNode * node)
{
	int i = node->il;
	
	for(;i<_nodeList.size()-1;++i)
	{
	 	int idx = _nodeList[i+1];
		rtNode * n = get(idx);
		assert(n->il==(i+1));

		if(!isLarge(n,node))
			break;

		n->il = i;
		_nodeList[i] = _nodeList[i+1];
	}

	if(i!=node->il)
	{
		node->il = i;
		_nodeList[i] = node->idx;
	}
}

void rtSearchPool::trickleDown(rtNode * node)
{
	int i = node->il;
		
	for(;i > 0;--i)
	{
		int idx = _nodeList[i-1];
		rtNode * n = get(idx);
		assert(n->il==i-1);

		if(!isSmall(n,node))
			break;

		n->il = i;
		_nodeList[i] = _nodeList[i-1];
	}
	
	if(i!=node->il)
	{	
		node->il = i;
		_nodeList[i] = node->idx;
	}
}

rtNode * rtSearchPool::find(const rtRegRef &id)
{
	itMapGate it = _mapGateRef.find(id);
	if(it!=_mapGateRef.end())
	{
		int idx = (*it).second;
		return &_nodeBuffer[idx];
	}
	return NULL;
}

rtNode * rtSearchPool::get(int idx)
{
	assert(idx<_nodeBuffer.size());
	return &_nodeBuffer[idx];
}

bool rtSearchPool::check()
{
	for(int i = 0;i<_nodeList.size()-1;++i)
	{
		int idx0 = _nodeList[i];
		int idx1 = _nodeList[i+1];
		rtNode * n0 = get(idx0);
		rtNode * n1 = get(idx1);
		if(isSmall(n0,n1))
			return false;
	}
	return true;
}
