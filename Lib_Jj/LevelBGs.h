#pragma once

#include "behaviorgraph/BehaviorGraphs.h"
#include "behaviorgraph/BehaviorGraphsUtil.h"

#include <set>

class CLevelBGs:public CBehaviorGraphs
{
public:
	IMPLEMENT_REFCOUNT_C
	DEFINE_CLASS(CLevelBGs);

	CLevelBGs()
	{
		Zero();
	}

	~CLevelBGs()
	{
		Clear();
	}

	void Zero()
	{
	}
	void Init(CBehaviorGraphUtil &util);
	virtual void Clear();

protected:
	BOOL _LoadBGPads(const char *path,CBehaviorGraphPads &pads);

	BOOL _ResolveIncludes(CBehaviorGraph *bg,std::set<CBehaviorGraph *>&stack,std::set<CBehaviorGraph *>&resolved);

	std::vector<BYTE>_bufTemp;


};