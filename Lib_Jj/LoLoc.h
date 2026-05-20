#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "LevelDefines.h"

#include "LevelObjSrc.h"

struct LosLoc:public CLevelObjSrc
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosLoc,12);

	BEGIN_GOBJ_PURE(LosLoc,1);


	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return FALSE;	}

};

struct LopLoc:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopLoc,12);

	BEGIN_GOBJ_PURE(LopLoc,1);

		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "标签名称", GVT_U, GSem(GSem_StringID,"地图位置标签"), "地图位置标签的名称" );

	END_GOBJ();

	virtual StringID GetUniqueName()	{		return nm;	}

	StringID nm;
};
