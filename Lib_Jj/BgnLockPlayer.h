#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelDetectTargetFlags.h"

class CBgp_LockPlayer:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_LockPlayer);

	virtual const char *GetTypeName()	{		return "锁定玩家";	}
	virtual DWORD GetStubCount()
	{
		return 4;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_C_OUT(1,"玩家条件");
			STUB_OUT(2,"锁定");
			STUB_OUT(3,"未锁定");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		std::vector<LevelDetectTargetFlag>flags=flagsDetect;
		for (int i=0;i<flags.size();i++)
			flags[i]=(LevelDetectTargetFlag)(flags[i]|(LevelDetectTarget_Player|LevelDetectTarget_Ground));

		FormatString(s,"在%.2f米范围内锁定:\n%s",range,LevelDetectTargetFlags_GetName(flags,StringID_BhvValInvalidRef));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_LockPlayer,1);
		GELEM_BGP_BASE();

		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTarget_Enemy);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,"敌方:1,本方:2,友方:4,中立:8"),"侦测什么类型的单位");

		GELEM_VAR_INIT(float,range,5.0f);
			GELEM_EDITVAR("侦测范围 ",GVT_F,GSem(GSem_Float,"0,100,0.1"),"侦测范围");
	END_GOBJ();    

public: //当作protected

	std::vector<LevelDetectTargetFlag> flagsDetect;

	float range;

};


class CBgn_LockPlayer:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_LockPlayer);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
