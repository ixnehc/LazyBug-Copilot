#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpGA_EnableBody:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_EnableBody);

	enum Op
	{
		Disable=0,
		Enable=1,
	};

	virtual const char *GetTypeName()	{		return "EnableBody";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (op==Disable)
			s="使自己的Body无效";
		else
			s="使自己的Body有效";
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_EnableBody,469,1);
		GELEM_VAR_INIT(Op,op,Disable);
			GELEM_EDITVAR("操作",GVT_S,GSem(GSem_Interger,"使无效,使有效"),"操作");
	END_GOBJ();    

public: //当作protected
	Op op;

};


class CBgnGA_EnableBody:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_EnableBody);

	CBgnGA_EnableBody()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

