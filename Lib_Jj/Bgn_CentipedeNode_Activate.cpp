/********************************************************************
	created:	2019/12/22 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "Bgn_CentipedeNode_Activate.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "LoCentipede.h"


#include "Skill_GeneralAdvS.h"

#include "Random/Random.h"



////////////////////////////////////////////////////////////////////////
//CBgn_CentipedeNode_Activate
BIND_BGN_CLASS(CBgn_CentipedeNode_Activate,CBgp_CentipedeNode_Activate);

void CBgn_CentipedeNode_Activate::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CentipedeNode_Activate*pad=_GetPad<CBgp_CentipedeNode_Activate>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	CLevelObj *loAgent=_GetLoFromVar(pad->varCentipedeAgent);
	if (loAgent->GetClass()->IsSameWith(Class_Ptr2(CLoCentipede)))
		((CLoCentipede*)loAgent)->Activate(lo);

	_OutputOk(outputs,1,"结束");
}

