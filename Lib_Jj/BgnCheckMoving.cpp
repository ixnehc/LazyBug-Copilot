/********************************************************************
	created:	2019/08/19 
	author:		cxi
	
	purpose:	 检测Moving
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "BgnCheckMoving.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckMoving

BIND_BGN_CLASS(CBgn_CheckMoving,CBgp_CheckMoving);

void CBgn_CheckMoving::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckMoving*pad=_GetPad<CBgp_CheckMoving>();


	CLevelObj *lo=_GetLo();
	if (pad->tpTarget==1)
		lo=_GetTalkLo();
	extern BOOL LevelUtil_IsMovingOrRotating(CLevelObj *lo);
	if (LevelUtil_IsMovingOrRotating(lo))
		_OutputOk(outputs,1,"是");
	else
		_OutputFail(outputs,2,"否");
}

