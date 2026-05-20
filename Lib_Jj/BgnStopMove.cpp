/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnStopMove.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"
#include "LevelTroops.h"

#include "LevelUtil.h"

#include "Log/LogDump.h"

////////////////////////////////////////////////////////////////////////
//CBgn_StopMove
BIND_BGN_CLASS(CBgn_StopMove,CBgp_StopMove);

extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
extern BOOL LevelUtil_StopMove(CLevelObj *lo);

void CBgn_StopMove::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelObj *lo=_GetLo();

	if (!LevelUtil_GetCastingSkill(lo))
	{
		LevelUtil_StopMove(lo);
		_OutputOk(outputs,1,"结束");
	}
}

void CBgn_StopMove::Update(BGNOutputs &outputs)
{
	CLevelObj *lo=_GetLo();
	if (!LevelUtil_GetCastingSkill(lo))
	{
		LevelUtil_StopMove(lo);
		_OutputOk(outputs,1,"结束");
	}

}

