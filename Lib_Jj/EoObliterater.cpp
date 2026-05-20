
#include "stdh.h"

#include "Level.h"

#include "LoUnit.h"

#include "EoObliterater.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoObliterater,EoParamObliterater);

void EoObliterater::_OnPostCreate()
{

}

void EoObliterater::_OnUpdate()
{
	if (!_bBurst)
	{
		EoParamObliterater *param=GetParam<EoParamObliterater>();
		if (!param)
			return;

		DealArg arg;
		arg.argObliterate=&_arg;

		DWORD c;
		CLevelObj **los=_DetectRange(GetFramePos(),param->radius,c);

		for (int i=0;i<c;i++)
		{
			CLevelObj *loTarget=los[i];
			LevelPos dir=loTarget->GetFramePos()-_GetInitialPos();
			dir.safe_normalize();

			//override some values
			arg.dir.setXZ(dir);
			arg.link.id=GetLevel()->GenOpLinkID();
			arg.grd=0;

			if (_arg.tp!=LevelObliterate_None)
			{
				LevelStrike strike;
				strike.idSrc=GetID();
				strike.SetDir(dir);
				strike.SetStr(param->strKill);

				CLevel *level=GetLevel();
				if (level)
					level->GetDecider()->MakeDamage(LevelOSB(this),loTarget,strike,&_arg.dmgs,arg.link,DmgBlockType_NotBlockable,FALSE);
			}

			_MakeDeals(LevelOSB(this),loTarget,arg);
		}

		_bBurst=TRUE;
	}
	if (_level->GetT_()>_tCreate+ANIMTICK_FROM_SECOND(4.0f))
		DeferDestroy();
}



void EoObliterater::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Data_WriteSimple(_idHost);
	bContent=TRUE;
}

void EoObliterater::SetObliterateArg(LevelObliterateArg &arg) 
{
	_arg=arg;
}

