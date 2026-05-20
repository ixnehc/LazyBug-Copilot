#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeHPPotion_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeHPPotion_Init,LevelAbilityType_HPPotion);

	BEGIN_GOBJ_PURE(CUpgradeHPPotion_Init,1);

		GELEM_VAR_INIT(DWORD,_percentAdd,40);
			GELEM_EDITVAR("增加最大HP百分比",GVT_U,GSem_Interger,"增加最大HP百分比");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:
	DWORD _percentAdd;

	friend class CLevelAbility_HPPotion;

};


struct LevelRecordSkill;
class CLevelAbility_HPPotion:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_HPPotion,CUpgradeHPPotion_Init,LevelAbilityType_HPPotion);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_HPPotion,1);

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

