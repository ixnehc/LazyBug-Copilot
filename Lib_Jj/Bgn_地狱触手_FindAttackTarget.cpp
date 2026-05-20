/********************************************************************
	created:	2020/8/21 
	author:		cxi
	
	*******************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"


#include "Bgn_地狱触手_FindAttackTarget.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelUnitArg.h"
#include "LoUnit.h"
#include "LevelSensor.h"

#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgn_地狱触手_FindAttackTarget
BIND_BGN_CLASS(CBgn_地狱触手_FindAttackTarget,CBgp_地狱触手_FindAttackTarget);

extern BOOL func_地狱触手_侦测干扰(CLevelObj *loSrc,LevelPos pos,float radius);


void CBgn_地狱触手_FindAttackTarget::Start(DWORD iStb,BGNOutputs &outputs)
{

	CBgp_地狱触手_FindAttackTarget*pad=_GetPad<CBgp_地狱触手_FindAttackTarget>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	BOOL bFound=FALSE;
	LevelPos posFound;

	if (lo)
	{
		if (lo->GetType()==LevelObjType_Unit)
		{
			CLoUnit *loUnit=(CLoUnit *)lo;
			UnitArg_地狱触手 *arg=loUnit->GetArg<UnitArg_地狱触手>();
			if (arg)
			{
				if (lo->GetSensor())
				{
					CLevelObj *loThreat=lo->GetSensor()->GetThreat();
					if (loThreat)
					{
						LevelPos pos=loThreat->GetFramePos();
						float distMin=100000.0f;
						int idxLocMin=-1;
						for (int i=0;i<arg->locsAttack.size();i++)
						{
							float dist=arg->locsAttack[i].getDistanceTo(pos);
							if (dist<=0.0f)
							{
								posFound=pos;
								bFound=TRUE;
								break;
							}
							if (pad->bFindClosest)
							{
								if (dist<distMin)
								{
									distMin=dist;
									idxLocMin=i;
								}
							}
						}

						if (pad->bFindClosest)
						{
							if (idxLocMin>=0)
							{
								posFound=arg->locsAttack[idxLocMin].clip(pos);
								bFound=TRUE;
							}
						}
					}
				}
			}
		}
	}

	if (bFound)
	{
		if (func_地狱触手_侦测干扰(lo,posFound,pad->_radius侦测干扰+lo->GetRadius_()))
			bFound=FALSE;
	}

	if (bFound)
	{
		_SetPos(pad->varPos,posFound);
		_OutputOk(outputs,1,"成功");
		return;
	}

	_OutputFail(outputs,2,"失败");
	return;
}

