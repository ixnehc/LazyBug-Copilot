#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelDetectTargetFlags.h"




class CBgp_CheckKilling:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckKilling);

	virtual const char *GetTypeName()	{		return "等待Killing";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"检测到");
			STUB_OUT(2,"未检测到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"等待%s的Killing事件",LevelDetectTargetFlags_GetName((LevelDetectTargetFlag)(flagsDetect|LevelDetectTarget_Ground|LevelDetectTarget_Resided|LevelDetectTarget_Flying)));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckKilling,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("检测对象",GVT_U,GSem(GSem_Flags,"敌方:1,本方:2,友方:4,中立:8,单位:16,玩家:32"),"检测什么类型的单位");
		GELEM_VAR_INIT(BOOL,bWait,TRUE);
			GELEM_EDITVAR("持续检测",GVT_S,GSem_Boolean,"持续检测直至检测到");
	END_GOBJ();    

public: //当作protected

	LevelDetectTargetFlag flagsDetect;
	BOOL bWait;
};


class CBgn_CheckKilling:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckKilling);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	BOOL _Update(BGNOutputs &outputs);

};
