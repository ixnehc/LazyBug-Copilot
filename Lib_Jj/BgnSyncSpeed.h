#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

class CBgp_SyncSpeed:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_SyncSpeed);

	virtual const char *GetTypeName()	{		return "同步速度";	}
	virtual DWORD GetStubCount()
	{
		return 1;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"与锁定的玩家同步速度");
	}

    BEGIN_GOBJ_PURE_UID(CBgp_SyncSpeed,1);
		GELEM_BGP_BASE();
    END_GOBJ();    

public: //当作protected

};


struct AttrNodeBase;
class CBgn_SyncSpeed:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SyncSpeed);
	CBgn_SyncSpeed()
	{
		_attrnode=NULL;
		_attrnodeFlying=NULL;
	}


	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:

	AttrNodeBase *_attrnode;
	AttrNodeBase *_attrnodeFlying;

};
