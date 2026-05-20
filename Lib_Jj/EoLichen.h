#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "EoEnv.h"

#define CLASSUID_Lichen 50

struct EoParamLichen:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamLichen);

	BEGIN_GOBJ_PURE(EoParamLichen,1);

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"持续时间");
		GELEM_VAR_INIT(AnimTick,durFI,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("淡入时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"淡入时间");
		GELEM_VAR_INIT(AnimTick,durFO,ANIMTICK_FROM_SECOND(0.5f));
			GELEM_EDITVAR("淡出时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"淡出时间");
		GELEM_VAR_INIT(float,radius,1.0f);
			GELEM_EDITVAR("半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"地丝半径");

	END_GOBJ();

	AnimTick dur;
	float radius;
	AnimTick durFI;
	AnimTick durFO;

};


class EoLichen:public CLoEffectObj
{
public:
	EoLichen()
	{
		_hLichen=EoEnvLichenHandle_Invalid;
	}
	DEFINE_LEVELOBJ_CLASS(EoLichen,CLASSUID_Lichen);

	virtual const char *GetShowName()	{		return "地丝";	}

protected:

	virtual void _OnPostCreate() override;
	virtual void _OnDetroy() override;

	virtual void _OnUpdate() override;


	EoEnvLichenHandle _hLichen;

};
