/********************************************************************
	created:	2022/2/2 
	author:		cxi
	
	*******************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelBehavior.h"
#include "LevelObj.h"
#include "Level.h"
#include "BgnSwitchWalkable.h"

#include "LevelUtil.h"

#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgn_SwitchWalkable
BIND_BGN_CLASS(CBgn_SwitchWalkable,CBgp_SwitchWalkable);

void CBgn_SwitchWalkable::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SwitchWalkable*pad=_GetPad<CBgp_SwitchWalkable>();

	CLevelObj *lo=_GetLo();
	LevelBehaviorContext *ctx=_GetCtx();

	ctx->level->GetUnitMgr()->SwitchWalkable(pad->bOn,lo->GetFramePos());

	_OutputOk(outputs,1,"结束");

}

