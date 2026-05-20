#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "LevelAIContext.h"

struct AttrNodeFloat;
class CBgpTroop_Combat:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpTroop_Combat);

	virtual const char *GetTypeName()	{		return "Troop战斗";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"在Troop(%s)周围%s米范围内侦测:\r\n%s\r\n",
			GetBVRDesc_StringID(BVR_ARG(_troop),assist),
			GetBVRDesc_Float(BVR_ARG(_range),assist),
			LevelDetectTargetFlags_GetName(BVR_ARG(_flags)));

		StringID idCmd=_idCmd;
		if (idCmd==StringID_Invalid)
			idCmd=LevelAIContext::GetStdCmd_Combat();
		AppendFmtString(s,"向Troop内的(%s)发送(%s)命令",LevelTroopRankFlag_GetDesc(BVR_ARG(_flagsRank)),assist->GetStr(idCmd));
	}

    BEGIN_GOBJ_PURE_UID(CBgpTroop_Combat,1);
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","组建哪个Troop");
			GELEM_BVR();

		GELEM_VAR_INIT(LevelTroopRankFlags,_flagsRank,LevelTroopRankFlag_All);
			GELEM_EDITVAR("职级",GVT_U,GSem(GSem_Flags,LevelTroopRankFlags_GSemContrains),"对哪些职级发命令");
			GELEM_BVR();

		GELEM_VAR_INIT(StringID,_idCmd,StringID_Invalid);
			GELEM_EDITVAR("命令",GVT_U,GSem(GSem_StringID,"AI命令"),"命令");
			GELEM_BVR();

		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,_flags,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位");
			GELEM_BVR();
		GELEM_VARVECTOR_INIT(LevelObjRequire,_requires,LevelObjRequire_Attackable);
			GELEM_EDITVAR("特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"有哪些特定的需求");
			GELEM_BVR();
		GELEM_VAR_INIT(float,_range,15.0f);
			GELEM_EDITVAR("侦测范围",GVT_F,GSem(GSem_Float,"0.0,100.0,0.1"),"侦测范围");
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(StringID,_troop);
	DEFINE_BVR(LevelTroopRankFlags,_flagsRank);

	DEFINE_BVR(std::vector<LevelDetectTargetFlag>,_flags);
	DEFINE_BVR(std::vector<LevelObjRequire>,_requires);
	DEFINE_BVR(float,_range);

	StringID _idCmd;

};


class CBgnTroop_Combat:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_Combat);

	CBgnTroop_Combat()
	{
		_idCmd=StringID_Invalid;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Destroy();

protected:
	void _UpdateTcc(CLevelTroop *troop);

	void _OccupyTroopControl();
	void _DiscardTroopControl();

	StringID _idCmd;

};

