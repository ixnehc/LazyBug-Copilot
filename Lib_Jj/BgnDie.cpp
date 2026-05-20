/********************************************************************
	created:	2013/8/21 
	author:		cxi
	
	purpose:	死亡
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordGlobal.h"

#include "BgnDie.h"

#include "Buff_Dead.h"

#include "LevelOSB.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Die
BIND_BGN_CLASS(CBgn_Die,CBgp_Die);


void CBgn_Die::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Die*pad=_GetPad<CBgp_Die>();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevelDecider *decider=ctx->level->GetDecider();

	RecordID idDeathBuff=pad->_idDeathBuff;
	if (idDeathBuff==RecordID_Invalid)
		idDeathBuff=_GetLevel()->GetRecords()->GetGlobal()->idDefBuff_Dead;

	extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
	if (!LevelUtil_CheckDead(ctx->lo))
	{
		BuffArg_Dead arg;
		LevelBuffID idBuff=decider->MakeBuff(LevelOSB(ctx->lo),ctx->lo,idDeathBuff,ANIMTICK_INFINITE,&arg,LevelOpLink());
	}

	_OutputOk(outputs,1,"结束");
}
