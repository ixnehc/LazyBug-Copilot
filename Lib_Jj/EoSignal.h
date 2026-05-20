#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Signal 36

struct EoParamSignal:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamSignal);

	BEGIN_GOBJ_PURE(EoParamSignal,1);

		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "信号名称", GVT_U, GSem(GSem_StringID,"信号名称"), "施放的信号名称" );
		GELEM_VAR_INIT(float,radius,5.0f);
			GELEM_EDITVAR("作用范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"作用范围");

	END_GOBJ();

	StringID nm;
	float radius;
};



class EoSignal:public CLoEffectObj
{
public:
	EoSignal()
	{
	}
	DEFINE_LEVELOBJ_CLASS(EoSignal,CLASSUID_Signal);

	virtual const char *GetShowName()	{		return "散播信号";	}

protected:
	virtual BOOL IsServerOnly()	{		return TRUE;	}

	virtual void _OnPostCreate();

};
