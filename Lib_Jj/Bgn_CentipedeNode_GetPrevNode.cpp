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

#include "Bgn_CentipedeNode_GetPrevNode.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "LoCentipede.h"


#include "Skill_GeneralAdvS.h"

#include "Random/Random.h"



////////////////////////////////////////////////////////////////////////
//CBgn_CentipedeNode_GetPrevNode
BIND_BGN_CLASS(CBgn_CentipedeNode_GetPrevNode,CBgp_CentipedeNode_GetPrevNode);

void CBgn_CentipedeNode_GetPrevNode::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CentipedeNode_GetPrevNode*pad=_GetPad<CBgp_CentipedeNode_GetPrevNode>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	CLevelObj *loAgent=_GetLoFromVar(pad->varCentipedeAgent);
	if (loAgent->GetClass()->IsSameWith(Class_Ptr2(CLoCentipede)))
	{
		DWORD index;
		if (((CLoCentipede*)loAgent)->GetNodeIndex(lo->GetID(),index))
		{
			if (index>0)
			{
				LevelObjID idNext=((CLoCentipede*)loAgent)->GetNodeFromIndex(index-1);
				if (idNext!=LevelObjID_Invalid)
				{
					if (pad->varResult!=StringID_Invalid)
					{
						_SetID(pad->varResult,BehaviorMemType_ObjID,idNext);
						_OutputOk(outputs,1,"成功");
						return;
					}
				}
			}
		}
	}

	_OutputFail(outputs,2,"失败");
}

