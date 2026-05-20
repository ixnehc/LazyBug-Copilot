#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpGA_CheckDay:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_CheckDay);

	virtual const char *GetTypeName()	{		return "设置当日Check标记";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_bCheck)
			s="标记当日已Check";
		else
			s="清除当日Check标记";
	}

    BEGIN_GOBJ_PURE(CBgpGA_CheckDay,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(BOOL,_bCheck,TRUE);
			GELEM_EDITVAR("是否标记为TRUE",GVT_S,GSem_Boolean,"是否Check");

    END_GOBJ();    

public: //当作protected
	BOOL _bCheck;//or UnCheck
};


class CBgnGA_CheckDay:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_CheckDay);

	CBgnGA_CheckDay()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



class CBgpGA_TestCheckDay:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_TestCheckDay);

	virtual const char *GetTypeName()	{		return "检测当日Check状态";	}
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
		s="是否已Check?";
	}

	BEGIN_GOBJ_PURE(CBgpGA_TestCheckDay,1);

		GELEM_BGP_BASE();

	END_GOBJ();    

public: //当作protected
};


class CBgnGA_TestCheckDay:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_TestCheckDay);

	CBgnGA_TestCheckDay()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
