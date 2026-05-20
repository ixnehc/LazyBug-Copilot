
#include "stdh.h"

#include "math/circle.h"
#include "Log/LogDump.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoStep.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

//////////////////////////////////////////////////////////////////////////
//EoStep

BIND_EOPARAM(EoStep,EoParamStep);

void EoStep::_DoUpdate()
{
	EoParamStep *param=_rec->GetParam<EoParamStep>();

	AnimTick t=_GetT();
	t=ANIMTICK_SAFE_MINUS(t,_tCreate);
	DWORD nStepsIntended=0;
	nStepsIntended=1+t/param->durStep;

	while(nStepsIntended>_nSteps)
	{
		int iStep=_nSteps;
		_nSteps++;

		float dist=param->distStep*(float)iStep;
		LevelPos3D pos=_GetInitialPos3D()+_GetInitialDir3D()*dist;

		extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
		pos=LevelUtil_GetGroundHeight(_level,pos.x,pos.z,TRUE);

		DealArg arg;
		arg.dir=_GetInitialDir3D();
		arg.link.id=GetLevel()->GenOpLinkID();
		arg.link.t=iStep*param->durStep;
		_MakeDeals(pos,arg);
	}
}

void EoStep::_OnPostCreate()
{
	CLevelSkill *skill=_GetOwnerSkill();

	EoParamStep *param=_rec->GetParam<EoParamStep>();

	if (param)
	{
		_DoUpdate();
	}

}

void EoStep::_OnUpdate()
{
	_DoUpdate();

	EoParamStep *param=_rec->GetParam<EoParamStep>();
	if (_nSteps>=param->nSteps)
		DeferDestroy();
}


