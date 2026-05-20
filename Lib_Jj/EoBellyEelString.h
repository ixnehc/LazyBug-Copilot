#pragma once

#include "EoBulletBase.h"
#include "EoBullet.h"

#include "gds/GObjUID.h"

#include "behaviorgraph/BehaviorParam.h"

#include "spline/CubicSpline.h"

#include "LevelDeal.h"

#include "protocal.h"


#define CLASSUID_BellyEelString 76


struct EoParamBellyEelString:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamBellyEelString);

	enum Mode
	{
		Mode_None,
		Mode_Throw,
		Mode_Swing,

		Mode_ForceDword=0xffffffff,
	};



	BEGIN_GOBJ_PURE_UID2(EoParamBellyEelString,501,1);

		GELEM_OBJVECTOR(DealEntry,_dealsTouch);
			GELEM_EDITOBJ("接触结算列表","接触结算列表");

		GELEM_VARVECTOR_INIT(RecordID,_candidatesAttachAgent,RecordID_Invalid);GELEM_UID(2);
			GELEM_EDITVAR("附着点Agent列表",GVT_U,GSem(GSem_RecordID,"agents"),"附着点Agent列表");

	END_GOBJ();


	std::vector<RecordID> _candidatesAttachAgent;

	std::vector<DealEntry> _dealsTouch;
};

struct CBEelString;

class EoBellyEelString:public CLoEffectObj
{
public:
	EoBellyEelString()
	{
		_stage=Stage_None;
		_tStageStart = 0;
		_idSrc=LevelObjID_Invalid;
		_idTarget=LevelObjID_Invalid;
		_idOwner=LevelObjID_Invalid;
		_bSyncDirty=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoBellyEelString,CLASSUID_BellyEelString);

	enum Stage
	{
		Stage_None,
		Stage_Connecting,
		Stage_Connected,
		Stage_Broken,
	};

	virtual const char *GetShowName()	{		return "BellyEelString";	}

	Stage GetStage()	{		return _stage;	}

	void NotifyNetMsg(CBEelString &msg)	{		_msg=msg;	}

protected:

	void _OnPostCreate() override;
	void _OnDetroy() override;
	void _OnUpdate() override;

	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;
	void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer) override;
	void _OnPostWriteSync() override;

	void _SetStage(Stage stage);

	LevelObjID _idSrc;
	LevelObjID _idOwner;
	LevelObjID _idTarget;
	Stage _stage;
	AnimTick _tStageStart;

	CBEelString _msg;

	DWORD _bSyncDirty:1;

};
