#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_CheckAward:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_CheckAward);

	enum Op
	{
		IsEmpty=0,
		ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "检测奖励";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(award!=StringID_Invalid)
		{
			if (op==IsEmpty)
				FormatString(s,"检测 %s 中的奖励是否为空",assist->GetStr(award));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpGA_CheckAward,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,award,StringID_Invalid);
			GELEM_EDITVAR( "奖励保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "哪个变量里的奖励");
		GELEM_VAR_INIT(Op,op,IsEmpty);
			GELEM_EDITVAR("操作",GVT_U,GSem(GSem_Interger,"是否为空"),"操作");
    END_GOBJ();    

public: //当作protected

	StringID award;
	Op op;
};


class CBgnGA_CheckAward:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_CheckAward);

	CBgnGA_CheckAward()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

