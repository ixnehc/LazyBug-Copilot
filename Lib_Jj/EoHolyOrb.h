#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "spline/CubicSpline.h"




#define CLASSUID_HolyOrb 62



struct EoParamHolyOrb:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamHolyOrb);

	BEGIN_GOBJ_PURE(EoParamHolyOrb,1);

		GELEM_VAR_INIT( StringID,nmSignal1,StringID_Invalid);	
			GELEM_EDITVAR( "开启圣光塔信号", GVT_U, GSem(GSem_StringID,"信号名称"), "开启圣光塔信号" );
		GELEM_VAR_INIT( StringID,nmSignal2,StringID_Invalid);	
			GELEM_EDITVAR( "解封灵气塔信号", GVT_U, GSem(GSem_StringID,"信号名称"), "解封灵气塔信号" );

	END_GOBJ();

	StringID nmSignal1;
	StringID nmSignal2;

};

class EoHolyOrb:public CLoEffectObj
{
public:
	EoHolyOrb()
	{
		_bSyncDirty=FALSE;
		_idPathRes=RecordID_Invalid;
		_durPath=0;
		_bReached=FALSE;
	}
	DEFINE_LEVELOBJ_CLASS(EoHolyOrb,CLASSUID_HolyOrb);

	virtual const char *GetShowName()	{		return "圣光球";	}

	void SetPath(RecordID idPathRes);


protected:

	void _OnPostCreate()override;
	void OnDestroy()override;


	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	void _OnPostWriteSync() override;

	void _OnUpdate()override;
	virtual BOOL _NeedOps()	{		return FALSE;	}

	BOOL _bSyncDirty;

	RecordID _idPathRes;
	AnimTick _durPath;

	BOOL _bReached;

};
