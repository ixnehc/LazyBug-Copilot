/********************************************************************
	created:	2010/11/02 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelObj.h"

#include "LevelBuff.h"
#include "Buff_Flies.h"


#include "BgnCheckFlies.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckFlies

BIND_BGN_CLASS(CBgn_CheckFlies,CBgp_CheckFlies);

void CBgn_CheckFlies::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckFlies*pad=_GetPad<CBgp_CheckFlies>();

	if (!_Update(outputs))
	{
		if (!pad->bWait)
			_OutputFail(outputs,2,"否");
	}
}

BOOL CBgn_CheckFlies::_Update(BGNOutputs &outputs)
{
	CBgp_CheckFlies*pad=_GetPad<CBgp_CheckFlies>();

	LevelBehaviorContext *ctx=_GetCtx();
	CLevel *level=ctx->level;
	CLevelObj *lo=NULL;

	lo=_GetLo();

	if (!lo)
		return FALSE;

	extern CLevelBuff *LevelUtil_FindBuffByRecordID(CLevelObj *lo,RecordID idBuff);
	CLevelBuff *buff=LevelUtil_FindBuffByRecordID(lo,pad->idBuff);
	if (buff)
	{
		if (buff->GetClass()->IsSameWith(Class_Ptr2(Buff_Flies)))
		{
			Buff_Flies *buffFlies=(Buff_Flies *)buff;
			if (buffFlies->IsEnchanted())
			{
				_OutputOk(outputs,1,"是");
				return TRUE;
			}
		}
	}

	return FALSE;
}


void CBgn_CheckFlies::Update(BGNOutputs &outputs)
{
	_Update(outputs);
}
