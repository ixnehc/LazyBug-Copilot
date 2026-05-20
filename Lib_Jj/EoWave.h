#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Wave 22

struct EoParamWave:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamWave);

	BEGIN_GOBJ_PURE(EoParamWave,1);

		GELEM_VAR_INIT(float,radius,2.0f);
			GELEM_EDITVAR("作用范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"作用范围");

		GELEM_VAR_INIT(float,speed,5.0f);
			GELEM_EDITVAR("扩散速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"扩散速度");

		GELEM_VAR_INIT(float,fov,360.0f);
			GELEM_EDITVAR("FOV",GVT_F,GSem(GSem_Float,"0.01,360,0.1"),"角度范围(0,360]");

	END_GOBJ();

	float radius;
	float speed;
	float fov;
};



class EoWave:public CLoEffectObj
{
public:
	EoWave()
	{
		_radiusCur=0.0f;
	}
	DEFINE_LEVELOBJ_CLASS(EoWave,CLASSUID_Wave);

	virtual const char *GetShowName()	{		return "波";	}

protected:

	void _OnUpdate();

	float _radiusCur;
	std::unordered_set<LevelObjID> _damaged;

};
