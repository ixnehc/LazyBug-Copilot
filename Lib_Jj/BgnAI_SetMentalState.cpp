/********************************************************************
	created:	2019/07/27
	author:		cxi
	
	purpose:	 检查自己的mental state
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelRecordBuff.h"

#include "BgnAI_SetMentalState.h"

//////////////////////////////////////////////////////////////////////////
//CBgpAI_SetMentalState
void CBgpAI_SetMentalState::FillDesc(std::string &s,FillDescAssist *assist)
{
	FormatString(s,"设置自己为%s状态",GetMentalStateName(_stateMental));
}


////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgnAI_SetMentalState,CBgpAI_SetMentalState);

void CBgnAI_SetMentalState::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpAI_SetMentalState*pad=_GetPad<CBgpAI_SetMentalState>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	LevelAIContext *ctx=lo->ObtainAIContext();
	if (ctx)
		ctx->stateMental=pad->_stateMental;

	_OutputOk(outputs,1,"结束");
}

