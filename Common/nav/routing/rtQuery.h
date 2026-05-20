
#pragma once

#include "rtWorld.h"

#include "rtNode.h"

enum rtStatus
{
	RT_SUCCESS,
	RT_TRYOVER,
	RT_UNREACHABLE,
	RT_FAILURE,
};

class rtQuery
{
public:
	
	void init(rtWorld * world);

	rtStatus findRegPath(const rtGateRef &srcGate,const rtGateRef & dstGate,
					 rtGateRef *pathGates,
					 int &numGates,int maxGates,int &d2dstGate) const;

	rtWorld * getWorld(){return _world;}
	
private:
	rtWorld * _world;
	mutable rtSearchPool	_searchPool;
	std::vector<rtGateRef>	_gatesBuffer[255];
};






