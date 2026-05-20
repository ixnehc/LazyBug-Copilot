#pragma once

#include "LevelDefines.h"


#include "LevelAbility.h"
#include "gds/GObj.h"
#include "gds/GObjUID.h"
#include "stringparser/stringparser.h"

#include "LevelRecordSkill.h"

#include "Random/Random.h"


class CUpgradeToeStone_Init:public CLevelAbilityInitial
{
public:
	DEFINE_ABILITY_UPGRADE_CLASS(CUpgradeToeStone_Init,LevelAbilityType_ToeStone);

	BEGIN_GOBJ_PURE(CUpgradeToeStone_Init,1);

		GELEM_VAR_INIT(RecordID,idEo,RecordID_Invalid);
			GELEM_EDITVAR("创建EO",GVT_S,GSem(GSem_RecordID,"eos"),"创建EO");
		GELEM_VAR_INIT(int,nMaxCharge,5);
			GELEM_EDITVAR("最大充能数",GVT_S,GSem_Interger,"最大充能数");


	END_GOBJ();


	virtual BOOL Init(CLevelAbility *ability_) override;

protected:

	RecordID idEo;
	int nMaxCharge;
	

	friend class CLevelAbility_ToeStone;

};



struct CBToeStoneThrust;
struct LevelRecordSkill;
class CLevelAbility_ToeStone:public CLevelAbility
{
	DEFINE_ABILITY_CLASS_WITH_INIITIAL_UPGRADE(CLevelAbility_ToeStone,CUpgradeToeStone_Init,LevelAbilityType_ToeStone);

	CLevelAbility_ToeStone()
	{
		GConstructor();
		_bToggledOn=FALSE;
		_bToggledOnAction=FALSE;
		_bInToggleOnCD=FALSE;
		_tLastThrust=ANIMTICK_INFINITE;
		_nMaxCharge=0;
	}
	~CLevelAbility_ToeStone()
	{
		GDestructor();
	}

	BEGIN_GOBJ_UID(CLevelAbility_ToeStone,1);

		GELEM_ABILITY_BASE();
		GELEM_VAR_INIT(int,_nCharge,0);GELEM_UID(1)

	END_GOBJ();

	virtual BOOL SupportToggle() override 	{		return TRUE;	}
	virtual AnimTick GetToggleOnCD()	 override 	{		return _durToggleOnCD;	}
	virtual BOOL CheckInToggleOnCD()	 override 	{		return _bInToggleOnCD;	}
	virtual BOOL CheckToggledOn() override	{		return _bToggledOn;	}
	virtual BOOL Toggle(BOOL bOn) override;

	virtual int GetChargeCount()	{		return _nCharge;	}
	virtual int GetMaxChargeCount()	{		return _nMaxCharge;	}


	void StartThrusts(CBToeStoneThrust &msg);

	BOOL FetchActivatedAction()
	{
		if (_bToggledOnAction)
		{
			_bToggledOnAction=FALSE;
			return TRUE;
		}
		return FALSE;
	}

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
	BOOL _bToggledOnAction;//只在client端有效,发生了一次_bToggledOn从FALSE变为TRUE的变化
	BOOL _bInToggleOnCD;

	AnimTick _tLastThrust;
	AnimTick _durToggleOnCD;

	int _nMaxCharge;
	int _nCharge;

};


