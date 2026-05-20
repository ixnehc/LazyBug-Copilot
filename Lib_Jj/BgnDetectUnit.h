#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_DetectUnit:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_DetectUnit);

	CBgp_DetectUnit()
	{
		GConstructor();
		requires.push_back(LevelObjRequire_Attackable);
	}
	~CBgp_DetectUnit()
	{
		GDestructor();
	}

	virtual const char *GetTypeName()	{		return "侦测单位";	}
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

		if (rangeMin<=0.0f)
			FormatString(s,"在%.2f米范围内侦测%s:\n%s",GetBVRDesc_Float(BVR_ARG(range),assist),bClosest?"最近的":"",
				LevelDetectTargetFlags_GetName(BVR_ARG(flagsDetect)));
		else
			FormatString(s,"在%.2f~%.2f米范围内侦测%s:\n%s",rangeMin,GetBVRDesc_Float(BVR_ARG(range),assist),bClosest?"最近的":"",
				LevelDetectTargetFlags_GetName(BVR_ARG(flagsDetect)));

		if (nmVar!=StringID_Invalid)
		{
			AppendFmtString(s,"\n结果保存在变量[%s]中",assist->GetStr(nmVar));
		}

	}

    BEGIN_GOBJ(CBgp_DetectUnit,1);
		GELEM_BGP_BASE();
		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位");
			GELEM_BVR();
		GELEM_VARVECTOR_INIT(LevelObjRequire,requires,LevelObjRequire_Attackable);
			GELEM_EDITVAR("特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"有哪些特定的需求");
		GELEM_OBJ(LevelDetectWeights,weights);
			GELEM_EDITOBJ("侦测权重","侦测权重");
			GELEM_BVR();
		GELEM_VAR_INIT(DetectSightType,tpSight,DetectSightType_Me);
			GELEM_EDITVAR("侦测源",GVT_S,GSem(GSem_Interger,
				"自己"		"|指定侦测源,"
				"Owner"		"|指定侦测源,"
				"Troop"		"|指定侦测源,"
				"指定对象"		""
				),"以谁的视野进行侦测");
		GELEM_BEHAVIORMEM_OBJID(nmCustomSrc,"指定侦测源","指定侦测源")
		GELEM_VAR_INIT(float,range,5.0f);
			GELEM_EDITVAR("侦测最大距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"侦测多远距离以内的单位");
			GELEM_BVR()
		GELEM_VAR_INIT(float,rangeMin,0.0f);
			GELEM_EDITVAR("侦测最小距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"侦测多远距离以外的单位");
		GELEM_VAR_INIT(BOOL,bClosest,0);
			GELEM_EDITVAR("侦测最适合的单位",GVT_S,GSem_Boolean,"是否侦测最适合的满足条件的单位");
		GELEM_BEHAVIORMEM_OBJID(nmVar,"保存变量","侦测到的对象保存在那个变量中")

	END_GOBJ();    

public: //当作protected

	DEFINE_BVR(std::vector<LevelDetectTargetFlag>,flagsDetect);
	std::vector<LevelObjRequire> requires;
	DEFINE_BVR(LevelDetectWeights,weights);
	DetectSightType tpSight;
	DEFINE_BVR(float,range);
	float rangeMin;
	StringID nmCustomSrc;
	StringID nmVar;
	DWORD flagsBuff;
	BOOL bClosest;
};


class CBgn_DetectUnit:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_DetectUnit);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



class CBgp_DetectSpecifiedUnit:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_DetectSpecifiedUnit);

	virtual const char *GetTypeName()	{		return "侦测指定单位";	}
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

		if (rangeMin<=0.0f)
			FormatString(s,"在%.2f米范围内侦测满足条件的%s单位",range,LevelDetectTargetFlags_GetRelationName(flagsDetect));
		else
			FormatString(s,"在%.2f~%.2f米范围内侦测满足条件的%s单位",rangeMin,range,LevelDetectTargetFlags_GetRelationName(flagsDetect));
		if (idUnit!=RecordID_Invalid)
			AppendFmtString(s,"\n单位类型为%s",assist->GetUnitName(idUnit));
		if (idBuff!=RecordID_Invalid)
			AppendFmtString(s,"\n拥有Buff:%s",assist->GetBuffName(idBuff));
		if (idItem!=RecordID_Invalid)
			AppendFmtString(s,"\n拥有Item:%s",assist->GetItemName(idItem));

		if (nmVar!=StringID_Invalid)
		{
			AppendFmtString(s,"\n结果保存在变量[%s]中",assist->GetStr(nmVar));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_DetectSpecifiedUnit,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTarget_Enemy);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,"敌方:1,本方:2,友方:4,中立:8"),"侦测什么类型的单位");
		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位类型",GVT_U,GSem(GSem_RecordID,"units"),"指定侦测某个类型的单位");
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("指定Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"指定侦测具有某个Buff的单位");
		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);
			GELEM_EDITVAR("拥有指定Item",GVT_U,GSem(GSem_RecordID,"items"),"指定侦测拥有某个Item的单位");
		GELEM_VAR_INIT(float,range,5.0f);
			GELEM_EDITVAR("侦测最大距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"侦测多远距离以内的单位");
		GELEM_VAR_INIT(float,rangeMin,0.0f);
			GELEM_EDITVAR("侦测最小距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"侦测多远距离以外的单位");
		GELEM_BEHAVIORMEM_OBJID(nmVar,"保存变量","侦测到的对象保存在那个变量中")

	END_GOBJ();    

public: //当作protected

	LevelDetectTargetFlag flagsDetect;
	float range;
	float rangeMin;
	RecordID idUnit;
	RecordID idBuff;
	RecordID idItem;
	StringID nmVar;
};


class CBgn_DetectSpecifiedUnit:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_DetectSpecifiedUnit);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:
	BOOL _EnumCallBack(CLevelObj *lo,float dist2);

};


