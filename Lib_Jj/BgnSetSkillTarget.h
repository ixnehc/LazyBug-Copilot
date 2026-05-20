#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "LevelObjMap.h"
#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"



class CBgpSetSkillTarget:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSetSkillTarget);


	virtual const char *GetTypeName()	{		return "设置技能目标";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	enum Type
	{
		SpecifiedPos,
		Threat,
		SpecifiedObjId,
		ThreatPos,
		SpecifiedObjIDAtSpecifiedPos,
		ThreatAtSpecifiedPos,

		ForceDword=0xffffffff,
	};

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (tp==SpecifiedPos)
		{
			if (varPos!=StringID_Invalid)
				FormatString(s,"指定技能目标为[%s]中的位置",StrLib_GetStr(varPos));
		}
		if (tp==SpecifiedObjId)
		{
			if (varObjId!=StringID_Invalid)
				FormatString(s,"指定技能目标为[%s]中的对象",StrLib_GetStr(varObjId));
		}
		if (tp==SpecifiedObjIDAtSpecifiedPos)
		{
			if ((varObjId!=StringID_Invalid)&&(varPos!=StringID_Invalid))
				FormatString(s,"指定技能目标为[%s]中的对象,到[%s]中的位置处施放",StrLib_GetStr(varObjId),StrLib_GetStr(varPos));
		}
		if (tp==ThreatAtSpecifiedPos)
		{
			if (varPos!=StringID_Invalid)
				FormatString(s,"指定技能目标为Threat,到[%s]中的位置处施放",StrLib_GetStr(varPos));
		}
		if (tp==Threat)
			s="指定技能目标为Threat";
		if (tp==ThreatPos)
			s="指定技能目标为Threat的位置";

		if (tp==SpecifiedPos||tp==ThreatPos)
		{
			if (distMin>0.0f)
				AppendFmtString(s,"\n位置离自己的距离保持为最少%.2f米",distMin);
			if (distMax>0.0f)
				AppendFmtString(s,"\n位置离自己的距离保持为最多%.2f米",distMax);
		}

	}

    BEGIN_GOBJ_PURE_UID(CBgpSetSkillTarget,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(Type,tp,SpecifiedPos); GELEM_UID(1);
			GELEM_EDITVAR("目标类型",GVT_S,GSem(GSem_Interger,
				"指定位置:0"		"|对象变量,"
				"指定对象:2"		"|位置变量&位置最短距离&位置最长距离,"
				"在指定位置针对指定对象:4"		","
				"在指定位置针对Threat:5"		"|对象变量,"
				"Threat:1"		"|位置变量&对象变量&位置最短距离&位置最长距离,"
				"Threat位置:3"		"|位置变量&对象变量"
				),"技能目标类型");
		GELEM_BEHAVIORMEM_POS(varPos,"位置变量","使用那个变量里的位置"); GELEM_UID(2);
		GELEM_BEHAVIORMEM_OBJID(varObjId,"对象变量","使用那个变量里的对象"); GELEM_UID(3);
		GELEM_VAR_INIT(float,distMin,0.0f);
			GELEM_EDITVAR("位置最短距离",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"位置离自己的最短距离");
		GELEM_VAR_INIT(float,distMax,0.0f);
			GELEM_EDITVAR("位置最长距离",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"位置离自己的最长距离");
    END_GOBJ();    

public: //当作protected

	Type tp;
	StringID varObjId;
	StringID varPos;
	float distMin;
	float distMax;

};


struct LevelRecordSkill;
class CBgnSetSkillTarget:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSetSkillTarget);

	CBgnSetSkillTarget()
	{
	}



	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

	void _ClampPosDist(LevelPos &pos);
};

