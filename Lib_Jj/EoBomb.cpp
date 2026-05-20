
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoBomb.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoBomb,EoParamBomb);

void EoBomb::_OnUpdate()
{
	EoParamBomb *param=GetParam<EoParamBomb>();
	if (!param)
		return;

	if (!_bBurst)
	{
		if (_level->GetT_()>=_tCreate+param->delay)
		{
			_bBurst=TRUE;

			BOOL bDone=FALSE;
			if ((param->bHostBomb)&&(param->radius<=0.0f)&&(!param->bIgnoreHost))
			{
				extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
				CLevelObj *loTarget=LevelUtil_GetAliveLo(_level,_idHost);
				if (loTarget)
				{
					DealArg arg;
					_MakeDeals(loTarget,arg);
					bDone=TRUE;
				}
			}
			if (!bDone)
				_MakeRangeDeal(param->radius,param->bIgnoreHost);
		}
	}
	else
	{
		if (_level->GetT_()>_tCreate+param->delay+ANIMTICK_FROM_SECOND(4.0f))
			DeferDestroy();
	}

}


void EoBomb::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	EoParamBomb *param=GetParam<EoParamBomb>();
	if (param)
	{
		if (param->bHostBomb)
		{
			bp->Data_WriteSimple(_idHost);
			bContent=TRUE;
		}
	}

}
