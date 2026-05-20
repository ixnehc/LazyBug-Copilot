#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "spline/CubicSpline.h"




#define CLASSUID_UtumRepairBridge 58


struct EoParamUtumRepairBridge:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamUtumRepairBridge);

	BEGIN_GOBJ_PURE(EoParamUtumRepairBridge,1);

		GELEM_VAR_INIT( StringID,nmBridgeStateVar,StringID_Invalid);	
			GELEM_EDITVAR( "断桥状态变量名称", GVT_U, GSem(GSem_StringID,"行为图内存变量名称"), "行为图内存变量名称" );

	END_GOBJ();

	StringID nmBridgeStateVar;


};

class EoUtumRepairBridge:public CLoEffectObj
{
public:
	EoUtumRepairBridge()
	{
		_idxSite=-1;
		_bRepaired=FALSE;
	}
	DEFINE_LEVELOBJ_CLASS(EoUtumRepairBridge,CLASSUID_UtumRepairBridge);

	virtual const char *GetShowName()	{		return "Utum修桥";	}

	void SetInfo(LevelObjID idBridge,int idxSite)
	{
		_idBridge=idBridge;
		_idxSite=idxSite;
	}

	static std::string &GetRepairedStr()
	{
		static std::string s("修桥_已完成");
		return s;
	}
	static std::string &GetAbordStr()
	{
		static std::string s("修桥_已中断");
		return s;
	}

	BOOL IsRepaired()
	{
		return _bRepaired;
	}


protected:

	void _OnPostCreate()override;
	void OnDestroy()override;


	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	void _OnPostWriteSync() override;

	void _OnUpdate()override;
	virtual BOOL _NeedOps()	{		return FALSE;	}

	BOOL _bRepaired;
	AnimTick _tRepaired;

	LevelObjID _idBridge;
	int _idxSite;




};
