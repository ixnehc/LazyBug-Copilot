#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"


class CUpgradeSacredArrow_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeSacredArrow_Init,LevelAbilityType_SacredArrow);

	BEGIN_GOBJ_PURE(CUpgradeSacredArrow_Init,1);


	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:


	friend class CLevelAbility_SacredArrow;

};



struct CBSacredArrowThrust;
struct LevelRecordSkill;
class CLevelAbility_SacredArrow:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_SacredArrow,CUpgradeSacredArrow_Init,LevelAbilityType_SacredArrow);

	CLevelAbility_SacredArrow()
	{
		GConstructor();
		_bToggledOn=FALSE;
	}
	~CLevelAbility_SacredArrow()
	{
		GDestructor();
	}

	BEGIN_GOBJ_UID(CLevelAbility_SacredArrow,1);

		GELEM_ABILITY_BASE();

	END_GOBJ();

	virtual BOOL SupportToggle() override 	{		return TRUE;	}
	virtual AnimTick GetToggleOnCD()	 override 	{		return 0;	}
	virtual BOOL CheckInToggleOnCD()	 override 	{		return FALSE;	}
	virtual BOOL CheckToggledOn() override	{		return _bToggledOn;	}
	virtual BOOL Toggle(BOOL bOn) override;


public:

	virtual void _SaveSync(CDataPacket &dp) override;
	virtual void _LoadSync(CDataPacket &dp,CRecords *records) override;

	virtual void _OnBuildRT() override;
	virtual void _OnClearRT() override;

	virtual void _OnUpdate(LevelTick dt) override;

	virtual void _OnBuildArtifactState(LevelItemState &state)	 override;

public://Take it as protected

	virtual void _InitTechs()	 override{	}

	BOOL _bToggledOn;

};


