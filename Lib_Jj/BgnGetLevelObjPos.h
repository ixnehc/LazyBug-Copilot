#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_GetLevelObjPos:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_GetLevelObjPos);

	virtual const char *GetTypeName()	{		return "得到游戏对象位置";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (nmVar==StringID_Invalid)
			s="n/a";
		else
		{
			if (nmLo==StringID_Invalid)
				FormatString(s,"得到自己的位置,结果保存在变量[%s]中",assist->GetStr(nmVar));
			else
				FormatString(s,"得到游戏对象[%s]的位置,结果保存在变量[%s]中",assist->GetStr(nmLo),assist->GetStr(nmVar));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_GetLevelObjPos,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_OBJID(nmLo,"游戏对象变量","尝试得到哪个游戏对象的位置")
		GELEM_BEHAVIORMEM_POS(nmVar,"保存变量","位置保存到哪个变量中")
	END_GOBJ();    

public: //当作protected
	StringID nmLo;
	StringID nmVar;
};


class CBgn_GetLevelObjPos:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_GetLevelObjPos);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


