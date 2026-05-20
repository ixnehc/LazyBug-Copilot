#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelAIContext.h"



class CBgpAI_SetMentalState:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgpAI_SetMentalState);

	virtual const char *GetTypeName()	{		return "设置MentalState";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_AI;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist);

    BEGIN_GOBJ_PURE_UID(CBgpAI_SetMentalState,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelAIMentalState,_stateMental,LevelAIMentalState_Relax);
			GELEM_EDITVAR("MentalState",GVT_U,GSem(GSem_Interger,LevelAIMentalState_SemConstraint),"MentalState");
	END_GOBJ();    

public: //当作protected
	LevelAIMentalState _stateMental;
};


class CBgnAI_SetMentalState:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnAI_SetMentalState);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
