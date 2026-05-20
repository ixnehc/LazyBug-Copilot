#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeHPFlusk_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeHPFlusk_Init,LevelAbilityType_HPFlusk);

	BEGIN_GOBJ_PURE(CUpgradeHPFlusk_Init,1);

	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:

	friend class CLevelAbility_HPFlusk;

};


struct LevelRecordSkill;
class CLevelAbility_HPFlusk:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_HPFlusk,CUpgradeHPFlusk_Init,LevelAbilityType_HPFlusk);


	BEGIN_GOBJ_PURE_UID2(CLevelAbility_HPFlusk,421,1);

		GELEM_ABILITY_BASE();
		GELEM_VAR_INIT(BYTE,_nFilled,0);GELEM_UID(1)
		GELEM_VAR_INIT(BYTE,_nFlusks,0);GELEM_UID(2)

	END_GOBJ();

	DWORD GetFilledCount()	{		return _nFilled;	}
	DWORD GetFluskCount()	{		return _nFlusks;	}

	void DecFilled();

	virtual BOOL IsConsumable() override{return TRUE;}
	virtual BOOL IsConsumed()	{		return _nFilled<=0;	}
	virtual void Consume();



	virtual void _OnBuildRT()	{	}
	virtual void _OnClearRT()	{	}
	virtual void _OnUpdate(LevelTick dt);
	virtual void _SaveSync(CDataPacket &dp);
	virtual void _LoadSync(CDataPacket &dp,CRecords *records);

	void Refill();

public://Take it as protected

	char _nFilled;
	char _nFlusks;

};

