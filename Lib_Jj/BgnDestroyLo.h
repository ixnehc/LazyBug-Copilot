#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_DestroyLo:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_DestroyLo);

	virtual const char *GetTypeName()	{		return "销毁对象";	}
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
		s="销毁本对象";
		if (_nmLo!=StringID_Invalid)
			FormatString(s,"销毁指定对象[%s]",assist->GetStr(_nmLo));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_DestroyLo,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_OBJID(_nmLo,"指定对象变量","指定对象变量")
	END_GOBJ();    

public: //当作protected

	StringID _nmLo;

};


class CBgn_DestroyLo:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_DestroyLo);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
