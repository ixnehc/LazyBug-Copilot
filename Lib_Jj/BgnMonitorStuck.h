#pragma once

#include "LevelDefines.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "ringbuf/RingBuf.h"


class CBgp_MonitorStuck:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_MonitorStuck);

	virtual const char *GetTypeName()	{		return "监控是否被卡住";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"监控到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

    BEGIN_GOBJ_PURE_UID(CBgp_MonitorStuck,1);
		GELEM_BGP_BASE();

	END_GOBJ();    

public: //当作protected


};

class CBgn_MonitorStuck:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_MonitorStuck);
	CBgn_MonitorStuck()
	{
		_nStuck=0;
		_t=0;
		_speed=0.0f;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	AnimTick _t;
	LevelPos _pos;
	int _nStuck;
	CRingBuf<LevelPos,10> _history;
	float _speed;

};
