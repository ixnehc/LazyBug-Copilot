#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_AirBomb 48

struct EoParamAirBomb:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamAirBomb);

	BEGIN_GOBJ_PURE(EoParamAirBomb,1);

		GELEM_VAR_INIT(float,radius,2.0f);
			GELEM_EDITVAR("作用范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"作用范围");


	END_GOBJ();

	float radius;
};



class EoAirBomb:public CLoEffectObj
{
public:
	EoAirBomb()
	{
		_bBurst=FALSE;
	}
	DEFINE_LEVELOBJ_CLASS(EoAirBomb,CLASSUID_AirBomb);

	virtual const char *GetShowName()	{		return "空中炸弹";	}

protected:

	void _OnUpdate();

	BOOL _bBurst;//初始的Damage有没有结算

};
