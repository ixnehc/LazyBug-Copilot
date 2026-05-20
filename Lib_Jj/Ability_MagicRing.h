#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeMagicRing_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeMagicRing_Init,LevelAbilityType_MagicRing);

	BEGIN_GOBJ_PURE(CUpgradeMagicRing_Init,1);

		GELEM_VAR_INIT(DWORD,_nCrystalHeartCapacity,3);
			GELEM_EDITVAR("魔晶之心单位数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"魔晶之心单位数");
		GELEM_VAR_INIT(float,_recoverPerSec,0.25f);
			GELEM_EDITVAR("每秒回复单位数",GVT_F,GSem(GSem_Float,"0.01,10.0,0.01"),"每秒回复几个魔晶单位");
	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:
	int _nCrystalHeartCapacity;
	float _recoverPerSec;

	friend class CLevelAbility_MagicRing;

};

#define MAX_MAGICRING_MP (41)


struct LevelRecordSkill;
class CLevelAbility_MagicRing:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_MagicRing,CUpgradeMagicRing_Init,LevelAbilityType_MagicRing);


	BEGIN_GOBJ_PURE_UID(CLevelAbility_MagicRing,1);

		GELEM_ABILITY_BASE();
		GELEM_VAR_INIT(short,_nMP,0);GELEM_UID(3);
		GELEM_VAR_INIT(short,_grade,1);GELEM_UID(4);

	END_GOBJ();


	DWORD GetMP()	{		return _nMP;	}
	DWORD MakeCost(DWORD cost);//返回实际的消耗
	void ResetMP()	{		_nMP=MAX_MAGICRING_MP;	}
	void IncMP(int amount)
	{
		_nMP+=amount;
		if (_nMP>MAX_MAGICRING_MP)
			_nMP=MAX_MAGICRING_MP;
	}
	void IncGrade()
	{
		_grade++;
	}


	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;

	virtual void _OnStartDay() override;

	virtual void _OnEvent(LevelEvent &e) override;

	virtual void _OnBuildArtifactState(LevelItemState &state)	 override;

public://Take it as protected

	short _nMP;
	LevelGrade _grade;


};

