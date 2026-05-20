/********************************************************************
	created:	2016/12/31 
	author:		cxi
	
	purpose:	 检测Threat是否存在
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "BgnThreat_CheckExist.h"

#include "LevelObj.h"
#include "LevelBGs.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_CheckExist
BIND_BGN_CLASS(CBgnThreat_CheckExist,CBgpThreat_CheckExist);

void CBgnThreat_CheckExist::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpThreat_CheckExist*pad=_GetPad<CBgpThreat_CheckExist>();

	CLevelObj *target=_GetThreat();
	if (!target)
	{
		_OutputFail(outputs,2,"否");
		return;
	}
	_OutputOk(outputs,1,"是");
}


