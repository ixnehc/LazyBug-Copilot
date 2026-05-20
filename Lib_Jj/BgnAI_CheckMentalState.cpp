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

#include "BgnAI_CheckMentalState.h"

//////////////////////////////////////////////////////////////////////////
//CBgpAI_CheckMentalState
void CBgpAI_CheckMentalState::FillDesc(std::string &s,FillDescAssist *assist)
{
	FormatString(s,"检测自己是否处于%s状态",GetMentalStateName(_stateMental));
}


////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgnAI_CheckMentalState,CBgpAI_CheckMentalState);

void CBgnAI_CheckMentalState::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpAI_CheckMentalState*pad=_GetPad<CBgpAI_CheckMentalState>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	LevelAIContext *ctx=lo->GetAIContext();
	if (ctx)
	{
		if (ctx->stateMental==pad->_stateMental)
		{
			_OutputOk(outputs,1,"是");
			return;
		}
	}

	_OutputFail(outputs,2,"否");
}

