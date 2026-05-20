/********************************************************************
	created:	2018/04/12 
	author:		cxi
	
	purpose:	 设置Pace
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecordUnit.h"


#include "BgnSetUnitPace.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LoUnit.h"



////////////////////////////////////////////////////////////////////////
//CBgn_SetUnitPace
BIND_BGN_CLASS(CBgn_SetUnitPace,CBgp_SetUnitPace);

void CBgn_SetUnitPace::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelObj *lo=_GetLo();
	CBgp_SetUnitPace*pad=_GetPad<CBgp_SetUnitPace>();

	CLevelObjMove *move=lo->GetMove();
	if (move)
	{
		if (pad->bCustomPace)
			move->SetUnitPace(&pad->pace);
		else
		{
			if (lo->GetType()==LevelObjType_Unit)
			{
				LevelRecordUnit *recUnit=((CLoUnit*)lo)->GetRec();
				if (recUnit)
					move->SetUnitPace(&recUnit->pace);
			}
		}
	}


	_OutputOk(outputs,1,"结束");
}
