
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoAreaDmg.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

  
BIND_EOPARAM(EoAreaDmg,EoParamAreaDmg);


void EoAreaDmg::_OnUpdate()
{
	EoParamAreaDmg *param=GetParam<EoParamAreaDmg>();
	if (!param)
		return;

	if (!_bInitialDmg)
	{
		_bInitialDmg=TRUE;

		//初始伤害
		CLevelDecider *decider=_level->GetDecider();

		extern CLevelObj **LevelUtil_DetectEnemies(CLevelObj *lo,float range,CLevelObj *toIgnore,LevelMoveMethodMask methods,DWORD &c);

		DWORD c;
		CLevelObj **los=_DetectRange(GetFramePos(),param->radius,c);

		for (int i=0;i<c;i++)
		{
			CLevelObj *loTarget=los[i];

			DealArg arg;
			arg.link.id=GetLevel()->GenOpLinkID();

			_MakeDeals(loTarget,arg);
		}
	}

	if (_level->GetT_()>_tCreate+param->dur+ANIMTICK_FROM_SECOND(1.0f))
	{
		DeferDestroy();
	}
}

