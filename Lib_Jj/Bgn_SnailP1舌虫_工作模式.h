#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_SnailP1舌虫_工作模式:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_SnailP1舌虫_工作模式);

	virtual const char *GetTypeName()	{		return "SnailP1舌虫_工作模式";	}
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
		if (_mode==2)
			s="设置为[缩回]";
		if (_mode==3)
			s="设置为[逃回]";
	}

    BEGIN_GOBJ_PURE_UID(CBgp_SnailP1舌虫_工作模式,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(DWORD,_mode,2);
			GELEM_EDITVAR("工作模式",GVT_U,GSem(GSem_Interger,"缩回:2,逃回:3"),"工作模式");

    END_GOBJ();    

public: //当作protected
	int _mode;

};


class CBgn_SnailP1舌虫_工作模式:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SnailP1舌虫_工作模式);

	CBgn_SnailP1舌虫_工作模式()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


