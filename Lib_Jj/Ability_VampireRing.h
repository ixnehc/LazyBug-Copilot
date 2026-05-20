#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeVampireRing_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeVampireRing_Init,LevelAbilityType_VampireRing);

	BEGIN_GOBJ_PURE(CUpgradeVampireRing_Init,1);

		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,detects,LevelDetectTargetFlag_Default); 
			GELEM_EDITVAR("对象类型",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"作用于什么类型的单位");

		GELEM_VARVECTOR_INIT(LevelObjRequire,requires,LevelObjRequire_Attackable); 
			GELEM_EDITVAR("对象的特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"对象的特定需求");

		GELEM_VAR_INIT(float,radius,4.0f);
			GELEM_EDITVAR("吸血半径",GVT_F,GSem(GSem_Float,"0.1,20.0,0.05"),"吸血半径");
		GELEM_VAR_INIT(float,tolerance,2.0f);
			GELEM_EDITVAR("吸血半径冗余值",GVT_F,GSem(GSem_Float,"0.1,20.0,0.05"),"开始吸血后,离开额外的多远距离后停止吸血");
		GELEM_VAR_INIT(float,ratioHP,0.5f);
			GELEM_EDITVAR("最大HP比率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.01"),"HP比率在什么值以下,开始吸血");

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,deal,Deal_CreateEo, "结算", "选择不同的结算" );
			GELEM_DYNOBJPTR_CLASS_DEAL( "创建Eo", Deal_CreateEo);

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:
	std::vector<LevelDetectTargetFlag> detects;
	std::vector<LevelObjRequire> requires;


	float radius;
	float tolerance;
	float ratioHP;
	CLevelDeal *deal; 


	friend class CLevelAbility_VampireRing;

};


struct LevelRecordSkill;
class CLevelAbility_VampireRing:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_VampireRing,CUpgradeVampireRing_Init,LevelAbilityType_VampireRing);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_VampireRing,1);

		GELEM_ABILITY_BASE();

	END_GOBJ();

	virtual void _OnBuildRT()	{	}
	virtual void _OnClearRT()	{	}
	virtual void _OnUpdate(LevelTick dt);
	virtual void _SaveSync(CDataPacket &dp);
	virtual void _LoadSync(CDataPacket &dp,CRecords *records);


public://Take it as protected

	BOOL _CanSuck(CLevelObj *lo);

	std::unordered_map<LevelObjID,LevelObjID> _links;


};

