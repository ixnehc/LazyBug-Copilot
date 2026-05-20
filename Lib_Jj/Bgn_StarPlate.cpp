/********************************************************************
	created:	2023/11/26 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "Bgn_StarPlate.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoStarPlate.h"


////////////////////////////////////////////////////////////////////////
//CBgn_StarPlateOp
BIND_BGN_CLASS(CBgn_StarPlateOp,CBgp_StarPlateOp);

void CBgn_StarPlateOp::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_StarPlateOp*pad=_GetPad<CBgp_StarPlateOp>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	CLoStarPlate *loStarPlate=(CLoStarPlate *)level->GetUniqueObj(LevelUniqueObj_StarPlate);
	if (loStarPlate)
	{
		if (pad->mode==1)
		{
			if (loStarPlate->CanActivateSite(lo->GetID()))
			{
				_OutputOk(outputs,1,"成功");
				return;
			} 
		}
		if (pad->mode==0)
		{
			loStarPlate->ActivateSite(lo->GetID());
			if (loStarPlate->CheckSiteActivated(lo->GetID()))
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode==2)
		{
			if (loStarPlate->CheckSiteActivated(lo->GetID()))
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode==3)
		{
			//Obsolete
		}
		if (pad->mode==4)
		{
			//Obsolete
		}
		if (pad->mode==5)
		{
			if (loStarPlate->CheckNeedSpawnEnemy())
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode==6)
		{
			loStarPlate->NotifyEnemySpawned();
			_OutputOk(outputs,1,"成功");
			return;
		}
		if (pad->mode==7)
		{
			if (loStarPlate->CheckAnySiteActivated())
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
		if (pad->mode==8)
		{
			if (loStarPlate->CheckFullActivated())
			{
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
	}
	_OutputFail(outputs,2,"失败");
}

