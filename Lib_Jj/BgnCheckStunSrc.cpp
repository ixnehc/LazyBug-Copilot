/********************************************************************
	created:	2016/05/15 
	author:		cxi
	
	purpose:	 检测EventSrc
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelObj.h"

#include "LevelEventSrc.h"


#include "BgnCheckStunSrc.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckStunSrc

BIND_BGN_CLASS(CBgn_CheckStunSrc,CBgp_CheckStunSrc);

void CBgn_CheckStunSrc::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckStunSrc*pad=_GetPad<CBgp_CheckStunSrc>();

	if (!_Update(outputs))
	{
		if (!pad->bWait)
			_OutputFail(outputs,2,"未检测到");
	}
}

BOOL CBgn_CheckStunSrc::_Update(BGNOutputs &outputs)
{
	CBgp_CheckStunSrc*pad=_GetPad<CBgp_CheckStunSrc>();

	LevelBehaviorContext *ctx=_GetCtx();
	CLevel *level=ctx->level;
	CLevelObj *lo=NULL;

	lo=_GetLo();

	if (!lo)
		return FALSE;

	CLevelEventSrc *src=lo->GetEventSrc();
	if (!src)
		return FALSE;

	AnimTick t=lo->GetT();
	t=ANIMTICK_SAFE_MINUS(t,pad->durGap);

	if (src->ExistStun(pad->count,pad->idSkillBreak,&pad->idsSkillStageBreak,t))
	{
		_OutputOk(outputs,1,"检测到");
		return TRUE;
	}

	return FALSE;
}


void CBgn_CheckStunSrc::Update(BGNOutputs &outputs)
{
	_Update(outputs);
}
