#pragma once

#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/Behavior.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "behaviorgraph/BehaviorValue.h"

#include "behaviorgraph/BgpFunc.h"

#include "LevelBehavior.h"


class CBgn_Func:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Func);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
};



class CBgn_Call:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Call);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void RewindOk(WORD keyRewind,BGNOutputs &outputs);//因为执行成功导致的Rewind
	virtual void RewindFail(WORD keyRewind,BGNOutputs &outputs);//因为执行失败导致的Rewind
	virtual void Break(BGNOutputs &outputs);

	BehaviorCall _call;
	std::deque<BhvVal> _values;


};

