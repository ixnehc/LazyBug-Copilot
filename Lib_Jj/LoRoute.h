#pragma once

#include "class/class.h"
#include "gds/GObj.h"

#include "LevelDefines.h"

#include "LevelObjSrc.h"

struct LosRoute:public CLevelObjSrc
{
public:
	DEFINE_LEVELOBJSRC_CLASS(LosRoute,13);

	BEGIN_GOBJ_PURE(LosRoute,1);


	END_GOBJ();

	virtual BOOL NeedSyncGUID()	{		return FALSE;	}


};

struct LopRoute:public CLevelObjParam
{
public:
	DEFINE_LEVELOBJPARAM_CLASS(LopRoute,13);

	BEGIN_GOBJ_PURE(LopRoute,1);

		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "路线名称", GVT_U, GSem(GSem_StringID,"路线名称"), "路线名称" );

		GELEM_STRING_INIT(path,"");
			GELEM_EDITVAR("路径资源",GVT_String,GSem_XformAnimPath,"路径资源");

		GELEM_VARVECTOR(i_math::vector2df,nodes)


	END_GOBJ();

	virtual StringID GetUniqueName()	{		return nm;	}

	StringID nm;
	std::vector<i_math::vector2df> nodes;//路点
	std::string path;

};
