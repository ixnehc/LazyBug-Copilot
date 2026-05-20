#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckDead:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckDead);

	virtual const char *GetTypeName()	{		return "检测死亡";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"是");
			STUB_OUT(2,"否");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Buff;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist);

    BEGIN_GOBJ_PURE_UID(CBgp_CheckDead,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_OBJID(_nmLo,"游戏对象变量","检测哪个游戏对象")
	END_GOBJ();    

public: //当作protected
	StringID _nmLo;
};


class CBgn_CheckDead:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckDead);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
