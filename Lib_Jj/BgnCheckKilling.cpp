/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 检测Killing事件
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "BgnCheckKilling.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckKilling

BIND_BGN_CLASS(CBgn_CheckKilling,CBgp_CheckKilling);

void CBgn_CheckKilling::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckKilling*pad=_GetPad<CBgp_CheckKilling>();

	if (!_Update(outputs))
	{
		if (!pad->bWait)
			_OutputFail(outputs,2,"未检测到");
	}
}

BOOL CBgn_CheckKilling::_Update(BGNOutputs &outputs)
{
	CBgp_CheckKilling*pad=_GetPad<CBgp_CheckKilling>();

	LevelBehaviorContext *ctx=_GetCtx();
	CLevel *level=ctx->level;
	CLevelObj *lo=_GetLo();
	if (!lo)
		return FALSE;
	LevelPlayerID idPlayer=lo->GetPlayerID();

	extern LevelPlayerMask LevelUtil_GetPlayersByRelation(CLevel *level,LevelPlayerID idPlayer,LevelRelation relation);
	LevelPlayerMask mask=0,maskUnit=0,maskPlayer=0;
	if (pad->flagsDetect&LevelDetectTarget_Enemy)
		mask|=LevelUtil_GetPlayersByRelation(level,idPlayer,LevelRelation_Enemy);
	if (pad->flagsDetect&LevelDetectTarget_Ally)
		mask|=LevelUtil_GetPlayersByRelation(level,idPlayer,LevelRelation_Ally);
	if (pad->flagsDetect&LevelDetectTarget_Neutral)
		mask|=LevelUtil_GetPlayersByRelation(level,idPlayer,LevelRelation_Neutral);
	if (pad->flagsDetect&LevelDetectTarget_Native)
		mask|=LevelUtil_GetPlayersByRelation(level,idPlayer,LevelRelation_Native);
	if (pad->flagsDetect&LevelDetectTarget_Player)
		maskPlayer=mask;
	if (pad->flagsDetect&LevelDetectTarget_Unit)
		maskUnit=mask;

	LevelEventQueue *qu=level->GetEventMap()->GetEventQueue(lo->GetFramePos());
	if (qu)
	{
		if ((qu->ePlayerKilling&maskPlayer)||(qu->eUnitKilling&maskUnit))
		{
			_OutputOk(outputs,1,"检测到");
			return TRUE;
		}
	}
	return FALSE;
}


void CBgn_CheckKilling::Update(BGNOutputs &outputs)
{
	_Update(outputs);
}
