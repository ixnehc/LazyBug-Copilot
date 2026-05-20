#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckMoveMethod:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckMoveMethod);

	virtual const char *GetTypeName()	{		return "检测移动方式(MoveMethod)";	}
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
		s="n/a";
		if (flags!=0)
			FormatString(s,"检测自己是否为(%s)单位",LevelDetectTargetFlags_GetMethodName(flags));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckMoveMethod,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelDetectTargetFlag,flags,LevelDetectTarget_Ground);
			GELEM_EDITVAR("移动方式",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetMoveMethodSemStr()),"移动方式");
	END_GOBJ();    

public: //当作protected
	LevelDetectTargetFlag flags;
};


class CBgn_CheckMoveMethod:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckMoveMethod);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
