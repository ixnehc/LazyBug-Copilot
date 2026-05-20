/********************************************************************
	created:	2016/05/29 
	author:		cxi
	
	purpose:	 TrapµÄEO
*********************************************************************/

#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoTrap.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoTrap,EoParamTrap);

void EoTrap::_OnDetroy()
{
}


void EoTrap::_OnUpdate()
{
	EoParamTrap *param=GetParam<EoParamTrap>();
	if (!param)
		return;

	if (!_bBurst)
	{
		if (param->durLife>0)
		{
			if (_GetAge()>param->durLife)
			{
				DeferDestroy();
				return;
			}
		}
		if (param->bEnvBound)
		{
			BOOL bEnv=FALSE;
			if (_level)
			{
				CLevelObj *loEnv=_level->GetEoEnv();
				if (loEnv&&loEnv->IsAlive())
					bEnv=TRUE;
			}
			if(!bEnv)
			{
				DeferDestroy();
				return;
			}
		}
	}

	if (!_bBurst)
	{
		if (!_bTriggered)
		{
			AnimTick tAge=_GetAge();

			_iLevelUp=param->GetLevelUp(tAge);

			if (tAge>=param->durSafe)
			{
				CLevelObj *lo=_DetectFirstInRange(GetFramePos(),param->radiusDetect);

				if (lo)
				{
					_bTriggered=TRUE;
					_tTriggered=_level->GetT_();

					LevelOp_StartFire *op=NewOp<LevelOp_StartFire>(LevelOpLink());
					AddOp(op);
				}
			}
		}

		if (_bTriggered)
		{
			if (_level->GetT_()>_tTriggered+param->delay)
			{
				_bBurst=TRUE;

				if (_iLevelUp<0)
					_MakeRangeDeal(param->radius);
				else
				{
					DealArg arg;

					DWORD c;
					CLevelObj **los=_DetectRange(GetFramePos(),param->radius,c);

					for (int i=0;i<c;i++)
					{
						CLevelObj *loTarget=los[i];
						LevelPos dir=loTarget->GetFramePos()-_GetInitialPos();

						//override some values
						arg.dir.setXZ(dir.safe_normalize());
						arg.link.id=GetLevel()->GenOpLinkID();
						arg.grd=0;

						MakeDeals(param->levelups[_iLevelUp].deals,LevelOSB(this),loTarget,arg,NULL);
					}
				}
			}
		}
	}

	if (_bBurst)
	{
		if (_level->GetT_()>_tTriggered+param->delay+ANIMTICK_FROM_SECOND(4.0f))
			DeferDestroy();
	}

}

