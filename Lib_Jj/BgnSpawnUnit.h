#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelTroops.h"

#include "records/recordsdefine.h"

#include "behaviorgraph/BehaviorCustomConst.h"
#include "behaviorgraph/BehaviorParam.h"

struct BP_SpawnUnit
{
	BEGIN_GOBJ_PURE(BP_SpawnUnit,1);

		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("单位",GVT_U,GSem(GSem_RecordID,"units"),"创建哪个单位");

		GELEM_VAR_INIT(LevelPlayerID,idPlayer,LevelPlayerID_Wild);
			GELEM_EDITVAR("PlayerID",GVT_B,GSem(GSem_Interger,"Wild:15,NeutalWild:14,PlayerWild:13"),"隶属于哪个Player");

		GELEM_VAR_INIT(LevelGrade,grd,LevelSkillGrade_Invalid);
			GELEM_EDITVAR("等级",GVT_B,GSem(GSem_Interger,LevelGradeBase_SemConstraint),"单位的等级");

		GELEM_VAR_INIT(RecordID,idBirthBuff,RecordID_Invalid);
			GELEM_EDITVAR("出生Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"出生Buff");

		GELEM_VAR_INIT( StringID,senarioAI,StringID_Invalid);
			GELEM_EDITVAR( "AI方案", GVT_U, GSem(GSem_StringID,"AIScenario"), "AI方案" );

	END_GOBJ();

	RecordID idUnit;
	LevelPlayerID idPlayer;
	LevelSkillGrade grd;
	RecordID idBirthBuff;

	StringID senarioAI;
};

class CBgp_SpawnUnit:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_SpawnUnit);

	virtual const char *GetTypeName()	{		return "创建Troop单位(单个)";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_troop!=StringID_Invalid)
			FormatString(s,"创建%s的单位",assist->GetStr(_troop));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_SpawnUnit,1);
		GELEM_BGP_BASE();

		GELEM_BEHAVIORMEM_POS(_nmPos,"位置变量","从哪个变量中取得位置")
		GELEM_VARVECTOR(i_math::matrix43f,_matsLS)
			GELEM_EDITVAR("位点(局部空间)",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"位点(局部空间)");
			GELEM_BVR();
		GELEM_OBJ(BP_SpawnUnit,_param);
			GELEM_EDITOBJ("创建参数","创建参数");
			GELEM_BVR();
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","加入到哪个Troop");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(BP_SpawnUnit,_param);
	StringID _nmPos;
	DEFINE_BVR(std::vector<i_math::matrix43f>,_matsLS);
	StringID _troop;

};


class CBgn_SpawnUnit:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SpawnUnit);

	CBgn_SpawnUnit()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

	void _Spawn(CLevel *level,CLevelTroop *troop,BP_SpawnUnit &info,LevelPos &pos,LevelFace face,StringID senarioAI);

};

