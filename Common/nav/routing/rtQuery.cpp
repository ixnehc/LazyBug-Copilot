
#include "stdh.h"

#include "rtQuery.h"

#include <float.h>

#include "routing.h"

#define MAX_GATES_SERCH	40

void rtQuery::init(rtWorld * world)
{
	_world = world;
}

rtStatus rtQuery::findRegPath(   const rtGateRef &srcGate,const rtGateRef & dstGate,
							 rtGateRef *pathGates,
							 int &numGates,int maxGates,int &d2dstGate) const
{	
	numGates = 0;
	_searchPool.reset();

	//初始化状态
	{
		rtNode node;
		node.cost = 0;
		node.flag = rtNode::Open;
		node.idGate = srcGate;
		node.costTotal = _world->getDistGateToGate(srcGate,dstGate);
		node.pidx = -1;
		_searchPool.push(_world->getRegByGate(srcGate),node);
	}

	rtGateRef nextGate = INVALID_GATE;
	float nextGate_cost = 0;
	float nextGate_cost2End = 0;
	rtRegRef regDst = _world->getRegByGate(dstGate);
	
	rtStatus status = RT_SUCCESS;

	const int maxtry = 1000;
	int tryCount = 0;
	while(tryCount < maxtry)
	{
		rtNode *nbest = _searchPool.top(); //当前最优节点
		
		if(nbest->flag!=rtNode::Open)
			break;
		
		if(_world->getRegByGate(nbest->idGate)==regDst)
		{
			nbest->flag = rtNode::Arrive;
			break;
		}

		//封闭节点
		{
			nbest->flag = rtNode::Closed;
			rtNode *nn = _searchPool.get(nbest->idx);
			_searchPool.trickleDown(nbest);
		}

		rtGateRef neigates[MAX_GATES_SERCH];
		int mincosts[MAX_GATES_SERCH];
		int gateCount = 0;

		for(int dir = 0;dir<4;++dir)
		{	
			if(!_world->getBestConGates(nbest->idGate,
										neigates,mincosts,gateCount,
										dir,MAX_GATES_SERCH))
				continue;
			
			for(int i = 0;i<gateCount;++i)
			{
				rtRegRef reg = _world->getRegByGate(neigates[i]);
				rtNode *node = _searchPool.find(reg);
				
				if(node&&node->flag==rtNode::Closed)
					continue;

				int cost = nbest->cost + mincosts[i];
				int cost2End = _world->getDistGateToGate(neigates[i],dstGate);
				int costTotal = cost + cost2End;

				if(node)
				{
					if(costTotal<node->costTotal)
					{
						node->cost = cost;
						node->costTotal = costTotal;
						node->idGate = neigates[i];
						_searchPool.bubbleUp(node);
					}
				}
				else
				{
					rtNode newNode;
					newNode.cost = cost;
					newNode.costTotal = costTotal;
					newNode.flag = rtNode::Open;
					newNode.idGate = neigates[i];
					newNode.pidx = nbest->idx;
					_searchPool.push(reg,newNode);
					nbest = _searchPool.get(newNode.pidx);//更新指针 内存大小改变可能至指针无效
				}
			}
		}
		
		++tryCount;
	}
	
	rtNode * nbest = _searchPool.top();
	if(!nbest)
		return RT_FAILURE;

	if(nbest->flag==rtNode::Arrive)
	{
		status = RT_SUCCESS;
		d2dstGate = 0;
	}
	else
	{
		if(tryCount>maxtry)
			status = RT_TRYOVER;
		else
			status = RT_UNREACHABLE;

		d2dstGate = _world->getDistGateToGate(nbest->idGate,dstGate);
	}
	
	std::vector<int> paths;
	rtNode * node = nbest;
	while(node)
	{
		paths.push_back(node->idx);
		if(node->pidx>=0)
			node = _searchPool.get(node->pidx);
		else 
			node = NULL;
	}
	int idx = paths.back();
	node = _searchPool.get(idx);
	for(int i = paths.size()-1;i>=0;--i)
	{
		int idx = paths[i];
		node = _searchPool.get(idx);
		
		if(node->idGate)

		if(numGates>=maxGates)
			break;

		pathGates[numGates] = node->idGate;
		++numGates;
	}

	return status;
}



