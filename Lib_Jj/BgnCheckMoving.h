#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckMoving:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckMoving);

	virtual const char *GetTypeName()	{		return "检查Moving";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (tpTarget==0)
			s="检查自己是否正在移动";
		if (tpTarget==1)
			s="检查TalkPlayer是否正在移动";
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckMoving,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(DWORD,tpTarget,0);
			GELEM_EDITVAR("检查目标",GVT_U,GSem(GSem_Interger,"自己,TalkPlayer"),"检查目标");
	END_GOBJ();    

public: //当作protected
	int tpTarget;
};


class CBgn_CheckMoving:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckMoving);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
