#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelDetectTargetFlags.h"

class CBgp_SetFaction:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_SetFaction);

	virtual const char *GetTypeName()	{		return "切换阵营";	}
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
		if (relation==0)
			s="切换为敌对阵营";
		if (relation==1)
			s="切换为中立阵营";
		if (relation==2)
			s="切换为同盟阵营";
	}

    BEGIN_GOBJ_PURE_UID(CBgp_SetFaction,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,relation,0);
			GELEM_EDITVAR("阵营关系",GVT_S,GSem(GSem_Interger,"敌对,中立,同盟"),"要切换为什么阵营");
	END_GOBJ();    

public: //当作protected

	int relation;

};


class CBgn_SetFaction:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SetFaction);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
