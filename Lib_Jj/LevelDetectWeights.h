#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "LevelDefines.h"

enum LevelDetectWeightsFlag
{
	LevelDetectWeights_None,
	LevelDetectWeights_Dist=1,
	LevelDetectWeights_Player=2,
	LevelDetectWeights_Agent=4,
	LevelDetectWeights_Aggressive=8,
	LevelDetectWeights_RecentTarget=16,
	LevelDetectWeights_FailedTarget=32,
	LevelDetectWeights_VeryClose=64,
	LevelDetectWeights_AttackingMe=128,
	LevelDetectWeights_AttackedByMe=256,
	LevelDetectWeights_ThreatingCount=512,
	LevelDetectWeights_AlertedCount=1024,
	LevelDetectWeights_CombatedCount=2048,

	LevelDetectWeights_FoceDword=0xffffffff
};

inline const char *LevelDetectWeightsFlag_GetSemStr()
{
	return "距离|权重值(距离)&距离的参考值:1,"
				"是否Player|权重值(是否Player):2,"
				"是否Agent|权重值(是否Agent):4,"
				"是否Aggressive|权重值(是否Aggressive):8,"
				"是否最近的Target|权重值(是否最近的Target):16,"
				"是否失败的Target|权重值(是否失败的Target):32,"
				"是否非常近|权重值(是否非常近)&非常近距离的参考值:64,"
				"是否正在攻击我|权重值(是否正在攻击我):128,"
				"是否正被我攻击|权重值(是否正被我攻击):256,"
				"Threating个数|权重值系数(每个Threating):512,"
				"Alerted个数|权重值系数(每个Alerted):1024,"
				"Combated个数|权重值系数(每个Combated):2048"
				;
}

struct LevelDetectWeightsBase
{
	LevelDetectWeightsBase()
	{
		Zero();
	}
	void Zero()
	{
		memset(this,0,sizeof(*this));
	}

	LevelDetectWeightsFlag flags;
	float wtDist;
	float distRef;
	float wtPlayer;
	float wtAgent;
	float wtAggressive;
	float wtRecentTarget;
	float wtFailedTarget;
	float wtVeryClose;
	float distVeryClose;
	float wtAttackingMe;
	float wtAttackedByMe;
	float scalePerThreating;
	float scalePerAlerted;
	float scalePerCombated;

	void AddFlag(LevelDetectWeightsFlag flag)
	{
		((DWORD&)flags)|=(DWORD)flag;
	}

	void CopyFrom(LevelDetectWeightsBase &other)
	{
		flags=other.flags;
		wtDist=other.wtDist;
		distRef=other.distRef;
		wtPlayer=other.wtPlayer;
		wtAgent=other.wtAgent;
		wtAggressive=other.wtAggressive;
		wtRecentTarget=other.wtRecentTarget;
		wtFailedTarget=other.wtFailedTarget;
		wtVeryClose=other.wtVeryClose;
		distVeryClose=other.distVeryClose;
		wtAttackingMe=other.wtAttackingMe;
		wtAttackedByMe=other.wtAttackedByMe;
		scalePerThreating=other.scalePerThreating;
		scalePerAlerted=other.scalePerAlerted;
		scalePerCombated=other.scalePerCombated;
	}

	void OverrideFrom(LevelDetectWeightsBase &other)
	{
		((DWORD&)flags)|=(DWORD)other.flags;
		if (other.flags&LevelDetectWeights_Dist)
		{
			wtDist=other.wtDist;
			distRef=other.distRef;
		}
		if (other.flags&LevelDetectWeights_Player)
			wtPlayer=other.wtPlayer;
		if (other.flags&LevelDetectWeights_Agent)
			wtAgent=other.wtAgent;
		if (other.flags&LevelDetectWeights_Aggressive)
			wtAggressive=other.wtAggressive;
		if (other.flags&LevelDetectWeights_RecentTarget)
			wtRecentTarget=other.wtRecentTarget;
		if (other.flags&LevelDetectWeights_FailedTarget)
			wtFailedTarget=other.wtFailedTarget;
		if (other.flags&LevelDetectWeights_VeryClose)
		{
			wtVeryClose=other.wtVeryClose;
			distVeryClose=other.distVeryClose;
		}
		if (other.flags&LevelDetectWeights_AttackingMe)
			wtAttackingMe=other.wtAttackingMe;
		if (other.flags&LevelDetectWeights_AttackedByMe)
			wtAttackedByMe=other.wtAttackedByMe;
		if (other.flags&LevelDetectWeights_ThreatingCount)
			scalePerThreating=other.scalePerThreating;
		if (other.flags&LevelDetectWeights_AlertedCount)
			scalePerAlerted=other.scalePerAlerted;
		if (other.flags&LevelDetectWeights_CombatedCount)
			scalePerCombated=other.scalePerCombated;
	}
};

struct LevelDetectWeights:public LevelDetectWeightsBase
{

	BEGIN_GOBJ_PURE(LevelDetectWeights,1)

		GELEM_VAR_INIT(DWORD,flags,LevelDetectWeights_Dist)
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,LevelDetectWeightsFlag_GetSemStr()),"哪些权重有效");

		GELEM_VAR_INIT(float,wtDist,100.0f)
			GELEM_EDITVAR("权重值(距离)",GVT_F,GSem(GSem_Float,"0.0,10000,0.01"),"距离权重");
		GELEM_VAR_INIT(float,distRef,20.0f)
			GELEM_EDITVAR("距离的参考值",GVT_F,GSem(GSem_Float,"0.0,1000,0.01"),"距离的参考值");
		GELEM_VAR_INIT(float,wtPlayer,100.0f)
			GELEM_EDITVAR("权重值(是否Player)",GVT_F,GSem(GSem_Float,"0.0,10000,0.01"),"是否Player权重");
		GELEM_VAR_INIT(float,wtAgent,100.0f)
			GELEM_EDITVAR("权重值(是否Agent)",GVT_F,GSem(GSem_Float,"0.0,10000,0.01"),"是否Agent权重");
		GELEM_VAR_INIT(float,wtAggressive,100.0f)
			GELEM_EDITVAR("权重值(是否Aggressive)",GVT_F,GSem(GSem_Float,"0.0,10000,0.01"),"是否Aggressive权重");
		GELEM_VAR_INIT(float,wtRecentTarget,100.0f)
			GELEM_EDITVAR("权重值(是否最近的Target)",GVT_F,GSem(GSem_Float,"0.0,10000,0.01"),"是否最近的Target权重");
		GELEM_VAR_INIT(float,wtFailedTarget,100.0f)
			GELEM_EDITVAR("权重值(是否失败的Target)",GVT_F,GSem(GSem_Float,"0.0,10000,0.01"),"是否失败的Target权重");
		GELEM_VAR_INIT(float,wtVeryClose,100.0f)
			GELEM_EDITVAR("权重值(是否非常近)",GVT_F,GSem(GSem_Float,"0.0,10000,0.01"),"是否非常近权重");
		GELEM_VAR_INIT(float,distVeryClose,2.0f)
			GELEM_EDITVAR("非常近距离的参考值",GVT_F,GSem(GSem_Float,"0.0,1000,0.01"),"多近算非常近");
		GELEM_VAR_INIT(float,wtAttackingMe,100.0f)
			GELEM_EDITVAR("权重值(是否正在攻击我)",GVT_F,GSem(GSem_Float,"0.0,10000,0.01"),"是否正在攻击我权重");
		GELEM_VAR_INIT(float,wtAttackedByMe,100.0f)
			GELEM_EDITVAR("权重值(是否正被我攻击)",GVT_F,GSem(GSem_Float,"0.0,10000,0.01"),"是否正在被我攻击的权重");
		GELEM_VAR_INIT(float,scalePerThreating,0.9f)
			GELEM_EDITVAR("权重值系数(每个Threating)",GVT_F,GSem(GSem_Float,"0,10,0.01"),"每个Threating乘上的权重值系数");
		GELEM_VAR_INIT(float,scalePerAlerted,0.7f)
			GELEM_EDITVAR("权重值系数(每个Alerted)",GVT_F,GSem(GSem_Float,"0,10,0.01"),"每个Alerted乘上的权重值系数");
		GELEM_VAR_INIT(float,scalePerCombated,0.7f)
			GELEM_EDITVAR("权重值系数(每个Combated)",GVT_F,GSem(GSem_Float,"0,10,0.01"),"每个Combated乘上的权重值系数");

	END_GOBJ();

};

