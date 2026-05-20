#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

class CUpgradeStormEye_Init:public CLevelAbilityInitial_Shield
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeStormEye_Init,LevelAbilityType_StormEye);

	BEGIN_GOBJ_PURE(CUpgradeStormEye_Init,1);

		GELEM_OBJ(AbilityActionSettings,settings);
			GELEM_EDITOBJ("Action参数","Action参数");

		GELEM_OBJ(BlockingEx,blocking);
			GELEM_EDITOBJ("格挡数值","格挡数值");

		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid); GELEM_UID(1)
			GELEM_EDITVAR("格挡Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"格挡Buff");

		GELEM_VAR_INIT(AnimTick,durAccum,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("蓄力时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"蓄力时间");

		GELEM_VAR_INIT(AnimTick,durCrytical,ANIMTICK_FROM_SECOND(0.2f));
			GELEM_EDITVAR("Crytical时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.01"),"Crytical");

		GELEM_OBJVECTOR(DealEntry,dealsBlocking);
			GELEM_EDITOBJ("格挡结算列表","格挡结算列表");

		GELEM_OBJVECTOR(DealEntry,dealsCryticalBlocking);
			GELEM_EDITOBJ("格挡结算列表(Crytical)","Crytical格挡结算列表");

		GELEM_DYNOBJPTR_DEAL(CLevelDeal,dealCryticalShockwave,Deal_CreateEo, "震荡波的Deal", "震荡波的Deal" );
			GELEMS_LEVELDEAL_CANDIDATES();

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

public:
	AbilityActionSettings settings;
	BlockingEx blocking;
	RecordID idBuff;
	AnimTick durAccum;
	AnimTick durCrytical;

	std::vector<DealEntry> dealsBlocking;
	std::vector<DealEntry> dealsCryticalBlocking;
	CLevelDeal *dealCryticalShockwave;


};



struct LevelRecordSkill;
class CLevelAbility_StormEye:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_StormEye,CUpgradeStormEye_Init,LevelAbilityType_StormEye);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_StormEye,1);

		GELEM_ABILITY_BASE();

		GELEM_VAR_INIT(BOOL,_bActive,1);GELEM_UID(1)
		GELEM_VAR_INIT(LevelTick,_tAccum,0);GELEM_UID(2)

	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override
	{
		_SaveSync_SkillsRT(dp);
		dp.Data_NextByte()=_bActive;
	}
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override
	{
		_LoadSync_SkillsRT(dp,records);
		_bActive=dp.Data_NextByte();
	}

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;

	virtual void _OnBuildArtifactState(LevelItemState &state)	 override;

	virtual void _OnEvent(LevelEvent &e) override;

	virtual BOOL CanGuard() override	{		return _bActive;	}

public://Take it as protected

	virtual void _InitTechs()	 override{	}

	BOOL _bActive;
	AnimTick _tAccum;


};

