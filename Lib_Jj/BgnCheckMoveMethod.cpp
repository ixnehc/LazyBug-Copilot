/********************************************************************
	created:	2016/11/07 
	author:		cxi
	
	purpose:	 检查移动方式
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "Level.h"

#include "BgnCheckMoveMethod.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckMoveMethod,CBgp_CheckMoveMethod);

void CBgn_CheckMoveMethod::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckMoveMethod*pad=_GetPad<CBgp_CheckMoveMethod>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	LevelMoveMethodMask mask=(LevelMoveMethodMask)(pad->flags/LevelDetectTarget_Ground);
	if (lo->GetMoveMethodMask()&mask)
	{
		_OutputOk(outputs,1,"是");
		return;
	}

	_OutputFail(outputs,2,"否");
}

