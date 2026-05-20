#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_圣旗_Assign:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_圣旗_Assign);

	virtual const char *GetTypeName()	{		return "Assign圣旗";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if ((nmLo!=StringID_Invalid)&&(nmBuffVar!=StringID_Invalid))
		{
			FormatString(s,"赋予目标单位[%s]圣旗,使用Buff变量[%s]",assist->GetStr(nmLo),assist->GetStr(nmBuffVar));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_圣旗_Assign,442,1);

		GELEM_BEHAVIORMEM_OBJID(nmLo,"赋予对象","赋予对象")

		GELEM_BEHAVIORMEM_BUFFRECORD(nmBuffVar,"名称(Buff表格项)","行为图内存变量名称")

	END_GOBJ();    

public: //当作protected

	StringID nmLo;
	StringID nmBuffVar;

};


class CBgnGA_圣旗_Assign:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_圣旗_Assign);

	CBgnGA_圣旗_Assign()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

