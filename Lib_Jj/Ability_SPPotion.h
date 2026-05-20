#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeSPPotion_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeSPPotion_Init,LevelAbilityType_SPPotion);

	BEGIN_GOBJ_PURE(CUpgradeSPPotion_Init,1);

		GELEM_VAR_INIT(DWORD,_percentAdd,40);
			GELEM_EDITVAR("增加最大HP百分比",GVT_U,GSem_Interger,"增加最大HP百分比");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:
	DWORD _percentAdd;

	friend class CLevelAbility_SPPotion;

};


struct LevelRecordSkill;
class CLevelAbility_SPPotion:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_SPPotion,CUpgradeSPPotion_Init,LevelAbilityType_SPPotion);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_SPPotion,1);

		GELEM_ABILITY_BASE();
		GELEM_VAR_INIT(BYTE,_bConsumed,0);GELEM_UID(1)

	END_GOBJ();

	virtual BOOL IsConsumable() override{return TRUE;}
	virtual BOOL IsConsumed()	{		return _bConsumed;	}
	virtual void Consume();

	virtual void _OnBuildRT()	{	}
	virtual void _OnClearRT()	{	}
	virtual void _OnUpdate(LevelTick dt){}
	virtual void _SaveSync(CDataPacket &dp);
	virtual void _LoadSync(CDataPacket &dp,CRecords *records);


public://Take it as protected



	BYTE _bConsumed;

};

