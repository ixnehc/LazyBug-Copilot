#pragma once

#include "LevelDeal.h"

#include "Level.h"
#include "LevelRecords.h"

#include "LevelRecordEO.h"

struct LevelRecordEo;
class Deal_CreateEo:public CLevelDeal
{
public:
	DEFINE_CLASS(Deal_CreateEo);

	Deal_CreateEo()
	{
		GConstructor();
		_recOverride=NULL;
		_recOrg=NULL;
	}
	~Deal_CreateEo();


	BEGIN_GOBJ(Deal_CreateEo,1);

		GELEM_VAR_INIT(BOOL,_bSendEvent,FALSE); GELEM_UID(1)
			GELEM_EDITVAR("发送创建Eo的系统事件",GVT_S,GSem_Boolean,"发送创建Eo的系统事件");
		GELEM_VAR_INIT(RecordID,_idEo,RecordID_Invalid);GELEM_UID(2)
			GELEM_EDITVAR("创建EO",GVT_U,GSem(GSem_RecordID,"eos"),"创建什么EO");
		GELEM_VAR_INIT(BOOL,_bUseAimPos,FALSE); GELEM_UID(3)
			GELEM_EDITVAR("创建于Aim点",GVT_S,GSem_Boolean,"创建于Aim点");

	END_GOBJ();

	void OverrideRecordEO(LevelRecordEo *rec);

	LevelRecordEo*BeginModify(CLevel *level)
	{
		_recOrg=level->GetRecords()->GetEo(_idEo);
		if (_recOrg)
		{
			if (!_recOverride)
				_recOverride=(LevelRecordEo*)_recOrg->Clone();
			return _recOverride;
		}
		return NULL;
	}
	void MakeDelta()
	{
		((CRecord*)_recOverride)->MakeDeltaOnCloned((CRecord*)_recOrg);
		_recOrg=NULL;
	}
	void EndModify()
	{
		_recOrg=NULL;
	}


	BOOL _bSendEvent;
	RecordID _idEo;
	BOOL _bUseAimPos;
	LevelRecordEo *_recOverride;
	LevelRecordEo *_recOrg;


	void Make(LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg,DealResult *result)override;
	void Make(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg,DealResult *result)override;

	CLoEffectObj *CreateEo(LevelOSB &osbSrc,CLevelObj *loTarget,DealArg&arg);
	CLoEffectObj *CreateEo(LevelOSB &osbSrc,LevelPos3D &pos,DealArg&arg,LevelObjID idHost);

};
