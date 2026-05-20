#pragma once

#include "LevelSpell.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"

struct LevelRecordSkill;
class CLevelSpell_FireBall:public CLevelSpell
{
	DEFINE_ABILITY_CLASS(CLevelSpell_FireBall,LevelAbilityType_Spell_FireBall);

	CLevelSpell_FireBall()
	{
		GConstructor();
	}

	~CLevelSpell_FireBall()
	{
		GDestructor();
	}

	BEGIN_GOBJ_UID(CLevelSpell_FireBall,1);

		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);GELEM_UID(1)
		GELEM_VAR_INIT(int,_grd,0);GELEM_UID(2)

	END_GOBJ();

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *recordsSkill) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;
	virtual void _OnUpdate(LevelTick dt) override;


public://Take it as protected
	virtual void _InitTechs()	 override{	}


	RecordID _idSkill;
	int _grd;
	SkillRuntime _cache;

};


class CUpgradeFireBall_Init:public CLevelSpellInitial
{
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeFireBall_Init,LevelAbilityType_Spell_FireBall);

	BEGIN_GOBJ_PURE(CUpgradeFireBall_Init,1);
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_S,GSem(GSem_RecordID,"skills"),"技能");
	END_GOBJ();

	virtual BOOL Init(CLevelAbility *ability);
	virtual RecordID GetSkillID()		{			return idSkill;		}

	RecordID idSkill;

};


class CUpgradeFireBall_LevelUp:public CLevelAbilityUpgrade_LevelUp
{
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeFireBall_LevelUp,LevelAbilityType_Fire);

	BEGIN_GOBJ_PURE(CUpgradeFireBall_LevelUp,1);
		GELEM_VAR_INIT(int,_grdRequired,0);
			GELEM_EDITVAR("等级需求",GVT_S,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"等级需求");
		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);
			GELEM_EDITVAR("升级后的技能",GVT_S,GSem(GSem_RecordID,"skills"),"新的技能");
	END_GOBJ();

	virtual RecordID GetSkillID()		{			return _idSkill;		}

	virtual BOOL CanUpgrade(CLevelAbility *ability)
	{
		CLevelSpell_FireBall *spell=(CLevelSpell_FireBall *)ability;
		if (spell->_grd==_grdRequired)
			return TRUE;
		return FALSE;
	}
	virtual void MakeSeed(CLevelAbility *ability,LevelAwardSeed &seed)
	{
	}

	virtual void Upgrade(CLevelAbility *ability0,LevelAwardSeed &seed)
	{
		CLevelSpell_FireBall *ability=(CLevelSpell_FireBall *)ability0;
		ability->_idSkill=_idSkill;
		ability->_grd++;
	}
	virtual const char *GetDesc(LevelAwardSeed &seed,CLevelRecords *records)
	{
		static std::string s;
		FormatString(s,"火伤害+%d",seed.v1);
		return s.c_str();
	}

protected:
	int _grdRequired;
	RecordID _idSkill;

};
