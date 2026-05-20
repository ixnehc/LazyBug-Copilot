#pragma once

#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "Protocal.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeUtumTide_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeUtumTide_Init,LevelAbilityType_UtumTide);

	BEGIN_GOBJ_PURE(CUpgradeUtumTide_Init,1);
		GELEM_VAR_INIT(RecordID,idAttackEo,RecordID_Invalid); GELEM_UID(1)
			GELEM_EDITVAR("攻击Eo",GVT_S,GSem(GSem_RecordID,"eos"),"EO");
		GELEM_VAR_INIT(RecordID,idRepairBridgeEo,RecordID_Invalid); GELEM_UID(2)
			GELEM_EDITVAR("修复断桥Eo",GVT_S,GSem(GSem_RecordID,"eos"),"EO");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability);

	RecordID idAttackEo;
	RecordID idRepairBridgeEo;

};




struct LevelRecordSkill;
class CLevelAbility_UtumTide:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_UtumTide,CUpgradeUtumTide_Init,LevelAbilityType_UtumTide);

	CLevelAbility_UtumTide()
	{
		GConstructor();
		_mode=Mode_None;
		_tModeStart=0;
		_bToggledOn=FALSE;
		_nExausted=0;
		_nAvailable=0;
		_nSummonedAttack=0;
		_nSummonedRepair=0;
	}
	enum WorkingMode
	{
		Mode_None,
		Mode_Default,
		Mode_RepairBridge,
	};


	BEGIN_GOBJ_UID2(CLevelAbility_UtumTide,435,1);

		GELEM_ABILITY_BASE();

		GELEM_VAR_INIT(DWORD,_nExausted,0);GELEM_UID(1)

	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	int GetAvailableCount()	{		return _nAvailable;	}

	virtual BOOL SupportToggle() override 	{		return TRUE;	}
	virtual AnimTick GetToggleOnCD()	 override 	{		return 0;	}
	virtual BOOL CheckInToggleOnCD()	 override 	{		return FALSE;	}
	virtual BOOL CheckToggledOn() override	{		return _bToggledOn;	}
	virtual BOOL Toggle(BOOL bOn) override;

	void StartRepairBridge(LevelObjID idBridge,DWORD nRequiredLabor);
	void EndRepairBridge(LevelObjID idBridge,BOOL bAbort);

	void NotifyUtumReturn(CBUtumReturn&msg);

	void _OnUpdate(LevelTick dt) override;
	void _OnStartDay() override;

public://Take it as protected

	void _UpdateExausted();
	void _UpdateAvailable();

	void _UpdateAttack();
	void _UpdateRepairBridge();

	int _nExausted;
	int _nAvailable;
	std::unordered_map<LevelObjID,BOOL> _flyings;

	void _SetMode(WorkingMode mode);
	WorkingMode _mode;
	AnimTick _tModeStart;

	//Mode_Default
	BOOL _bToggledOn;
	AnimTick _tToggledOn;
	DWORD _nSummonedAttack;

	//Mode_RepairBridge
	LevelObjID _idBridge;
	DWORD _nRequiredLabor;
	DWORD _nSummonedRepair;

};

