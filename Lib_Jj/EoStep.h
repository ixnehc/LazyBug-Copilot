#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_Step 51



struct EoParamStep:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamStep);


	BEGIN_GOBJ_PURE(EoParamStep,1);

		GELEM_VAR_INIT(float,distStep,0.5f);
			GELEM_EDITVAR("单步距离",GVT_F,GSem(GSem_Float,"0.0,10.0,0.05"),"单步距离");
		GELEM_VAR_INIT(AnimTick,durStep,ANIMTICK_FROM_SECOND(0.1f));
			GELEM_EDITVAR("单步时间",GVT_U,GSem(GSem_AnimTick,"0.01,100,0.01"),"单步时间");
		GELEM_VAR_INIT(int,nSteps,4);
			GELEM_EDITVAR("步数",GVT_S,GSem_Interger,"步数");
	END_GOBJ();

	float distStep;
	AnimTick durStep;
	int nSteps;

};



class EoStep:public CLoEffectObj
{
public:
	EoStep()
	{
		_nSteps=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoStep,CLASSUID_Step);

	virtual const char *GetShowName()	{		return "步长";	}

protected:
	virtual void _OnPostCreate() override;
	virtual void _OnUpdate() override;

	void _DoUpdate();

	DWORD _nSteps;


};
