/********************************************************************
	created:	2017/2/15 
	author:		cxi
	
	purpose:	GA功能:修改HP
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelOSB.h"

#include "BgnGA_ModHP.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_ModHP
BIND_BGN_CLASS(CBgnGA_ModHP,CBgpGA_ModHP);

//返回是否扣血扣的只剩一丝血
BOOL CBgnGA_ModHP::_DoMod(int nMod)
{
	CBgpGA_ModHP*pad=_GetPad<CBgpGA_ModHP>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();
	BOOL bAlmostKilling=FALSE;

	if (level)
	{
		CLevelDecider *decider=level->GetDecider();
		if (decider)
		{
			CLevelObj *lo=NULL;
			if (pad->tpTarget==0)
				lo=_GetTalkLo();
			if (pad->tpTarget==1)
				lo=_GetLo();
			if(lo)
			{
				int nToMod=nMod;

				if (!pad->bMaxHP)
				{
					LevelStrike strike;
					int nMod;
					if (nToMod<0)
					{
						nToMod=-nToMod;
						extern float LevelUtil_GetCurHP(CLevelObj *lo);
						float hpCur=LevelUtil_GetCurHP(lo);
						if ((float)nToMod>(hpCur-1.0f))
						{
							nToMod=FloatToNearestInt(hpCur)-1;
							bAlmostKilling=TRUE;
						}
						nToMod=-nToMod;
					}
					decider->CommitHPMod(nToMod,LevelOSB(_GetLo()),lo,strike,LevelOpLink(),nMod);
				}
				else
					decider->MakeCure_MaxHP(LevelOSB(_GetLo()),lo,nToMod,LevelOpLink());
			}
		}
	}
	return bAlmostKilling;
}



void CBgnGA_ModHP::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_ModHP*pad=_GetPad<CBgpGA_ModHP>();

	if (pad->dur<=0)
	{
		_DoMod(pad->nMod);
		_OutputOk(outputs,1,"结束");
	}

	_tStartMod=_GetT();
}

void CBgnGA_ModHP::Update(BGNOutputs &outputs)
{
	CBgpGA_ModHP*pad=_GetPad<CBgpGA_ModHP>();
	AnimTick t=_GetT();

	AnimTick tAge=ANIMTICK_SAFE_MINUS(t,_tStartMod);

	float r=((float)tAge)/((float)pad->dur);
	if (r>1.0f)
		r=1.0f;
	BOOL bAdd=TRUE;
	if(pad->nMod<0.0f)
		bAdd=FALSE;
	int nToMod=FloatToNearestInt((fabsf((float)pad->nMod))*r);

	if (nToMod>_nModed)
	{
		if (bAdd)
			_DoMod(nToMod-_nModed);
		else
		{
			if (_DoMod(-(nToMod-_nModed)))
			{
				_OutputOk(outputs,2,"中断");
				return;
			}
		}

		_nModed=nToMod;
	}

	if (r>=1.0f)
		_OutputOk(outputs,1,"结束");
}
