#pragma once

#include "LevelDefines.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpTroop_Clean:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgpTroop_Clean);

	enum Type
	{
		None,
		CleanAll,
		FlushDeadFrame,
		Detach,
	};


	virtual const char *GetTypeName()	{		return "清除Troop";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"OK");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_troop!=StringID_Invalid)
		{
			if (_tp==CleanAll)
				FormatString(s,"清除%s中的单位",assist->GetStr(_troop));
			if (_tp==FlushDeadFrame)
				FormatString(s,"清除%s中的已死单位的编制",assist->GetStr(_troop));
			if (_tp==Detach)
				FormatString(s,"清除%s中的单位及编制,但不杀死这些单位",assist->GetStr(_troop));
		}
	}

    BEGIN_GOBJ_PURE(CBgpTroop_Clean,1);

		GELEM_VAR_INIT(Type,_tp,CleanAll);
			GELEM_EDITVAR("清除类型",GVT_S,GSem(GSem_Interger,"n/a,清除所有单位及其编制,清除所有已死单位的编制,Detach模式"),"清除的类型");
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","清除哪个Troop");

	END_GOBJ();    

public: //当作protected

	Type _tp;
	StringID _troop;

};

class CBgnTroop_Clean:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_Clean);
	CBgnTroop_Clean()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
