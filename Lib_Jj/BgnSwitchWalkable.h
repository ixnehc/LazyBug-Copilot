#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_SwitchWalkable:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_SwitchWalkable);

	virtual const char *GetTypeName()	{		return "切换可走区域";	}
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
		if (bOn)
			s="切换周围的导航网格为可走";
		else
			s="切换周围的导航网格为不可走";
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_SwitchWalkable,415,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(BOOL,bOn,FALSE);
			GELEM_EDITVAR( "切换模式", GVT_S,GSem( GSem_Interger, "切换为可走:1,切换为不可走:0" ), "切换模式" );
    END_GOBJ();    

public: //当作protected

	BOOL bOn;

};

class CBgn_SwitchWalkable:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SwitchWalkable);

	CBgn_SwitchWalkable()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);


protected:



};
