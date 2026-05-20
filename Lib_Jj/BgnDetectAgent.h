#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"







class CBgp_DetectSpecifiedAgent:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_DetectSpecifiedAgent);

	virtual const char *GetTypeName()	{		return "侦测指定Agent";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"侦测到");
			STUB_OUT(2,"未侦测到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		extern const char *GetBgnRegName(StringID nm);

		s="n/a";
		if (idAgent!=RecordID_Invalid)
		{
			if (idBuff==RecordID_Invalid)
			{
				if (rangeMin<=0.0f)
					FormatString(s,"在%.2f米范围内侦测:%s(%s)",range,assist->GetAgentName(idAgent),
						LevelDetectTargetFlags_GetRelationName(flagsDetect));
				else
					FormatString(s,"在%.2f~%.2f米范围内侦测:%s(%s)",rangeMin,range,assist->GetAgentName(idAgent),
						LevelDetectTargetFlags_GetRelationName(flagsDetect));
			}
			else
			{
				if (rangeMin<=0.0f)
					FormatString(s,"在%.2f米范围内侦测:有[%s]的%s(%s)",range,assist->GetBuffName(idBuff),assist->GetAgentName(idAgent),
					LevelDetectTargetFlags_GetRelationName(flagsDetect));
				else
					FormatString(s,"在%.2f~%.2f米范围内侦测:有[%s]的%s(%s)",rangeMin,range,assist->GetBuffName(idBuff),assist->GetAgentName(idAgent),
					LevelDetectTargetFlags_GetRelationName(flagsDetect));
			}
			if (nmVar!=StringID_Invalid)
			{
				AppendFmtString(s,"\n结果保存在变量[%s]中",assist->GetStr(nmVar));
			}
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_DetectSpecifiedAgent,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTarget_Enemy);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,"敌方:1,本方:2,友方:4,中立:8"),"侦测什么类型的单位");
		GELEM_OBJ(LevelDetectWeights,weights);
			GELEM_EDITOBJ("侦测权重","侦测权重");
			GELEM_BVR();
		GELEM_VAR_INIT(RecordID,idAgent,RecordID_Invalid);
			GELEM_EDITVAR("单位类型",GVT_U,GSem(GSem_RecordID,"agents"),"指定侦测某个类型的单位");
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("指定Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"指定侦测具有某个Buff的单位");
		GELEM_VAR_INIT(float,range,5.0f);
			GELEM_EDITVAR("侦测最大距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"侦测多远距离以内的单位");
		GELEM_VAR_INIT(float,rangeMin,0.0f);
			GELEM_EDITVAR("侦测最小距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"侦测多远距离以外的单位");
		GELEM_BEHAVIORMEM_OBJID(nmVar,"保存变量","侦测到的对象保存在那个变量中")

	END_GOBJ();    

public: //当作protected

	LevelDetectTargetFlag flagsDetect;
	DEFINE_BVR(LevelDetectWeights,weights);
	float range;
	float rangeMin;
	RecordID idAgent;
	RecordID idBuff;
	StringID nmVar;

};


class CBgn_DetectSpecifiedAgent:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_DetectSpecifiedAgent);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:
	BOOL _EnumCallBack(CLevelObj *lo,float dist2);

};


