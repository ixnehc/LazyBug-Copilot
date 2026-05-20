/********************************************************************
	created:	2020/05/30 
	author:		cxi
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "behaviorgraph/BehaviorGraphs.h"


#include "BgnThreat_UpdateCtrl.h"

#include "LevelSensor.h"



////////////////////////////////////////////////////////////////////////
//CBgnThreat_UpdateCtrl
BIND_BGN_CLASS(CBgnThreat_UpdateCtrl,CBgpThreat_UpdateCtrl);

BOOL CBgnThreat_UpdateCtrl::_Finalize(BGNOutputs &outputs)
{
	if (_bFinalized)
		return FALSE;

	LevelBehaviorContext *ctx=_GetCtx();
	CLevelSensor *sensor=ctx->lo->GetSensor();
	if (sensor)
		sensor->PopActive();

	_bFinalized=TRUE;
	return FALSE;
}


void CBgnThreat_UpdateCtrl::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpThreat_UpdateCtrl*pad=_GetPad<CBgpThreat_UpdateCtrl>();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevelSensor *sensor=ctx->lo->GetSensor();
	if (sensor)
	{
		sensor->PushActive(pad->bActive);
	}

	BgnThread thrd=_thrd;
	thrd.idNode=_id;
	thrd.keyRewind=0;
	_VerifyStbName(1,"³ö¿Ú");
	outputs.Add(1,thrd);
}

void CBgnThreat_UpdateCtrl::RewindOk(WORD keyRewind,BGNOutputs &outputs)
{
	_Finalize(outputs);

	_SetResult(A_Ok);
}

void CBgnThreat_UpdateCtrl::RewindFail(WORD keyRewind,BGNOutputs &outputs)
{
	_Finalize(outputs);

	_SetResult(A_Fail);
}

void CBgnThreat_UpdateCtrl::Break(BGNOutputs &outputs)
{
	_Finalize(outputs);
}
