/********************************************************************
	created:	2020/06/14 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnThreat_CalcDist.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"

#include "LevelUtil.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_CalcDist
BIND_BGN_CLASS(CBgnThreat_CalcDist,CBgpThreat_CalcDist);


void CBgnThreat_CalcDist::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpThreat_CalcDist*pad=_GetPad<CBgpThreat_CalcDist>();

	LevelPos posSrc,posTarget;
	if (_GetLevelSkillTarget_Pos(pad->src,posSrc))
	{
		if (_GetLevelSkillTarget_Pos(pad->target,posTarget))
		{
			float dist=posSrc.getDistanceFrom(posTarget);
			_SetFloat(pad->varDist,dist);
			_OutputOk(outputs,1,"成功");
			return;
		}
	}
	_OutputFail(outputs,2,"失败");
}

