/********************************************************************
	created:	2018/02/22 
	author:		cxi
	
	purpose:	 检查Grade的范围
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LevelAttrs.h"

#include "BgnCheckGrade.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckGrade,CBgp_CheckGrade);

void CBgn_CheckGrade::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckGrade*pad=_GetPad<CBgp_CheckGrade>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	LevelAttr_Base *attr=lo->GetAttr_Base();
	if (attr)
	{
		if ((attr->grade>=pad->gradeMin)&&(attr->grade<=pad->gradeMax))
		{
			_OutputOk(outputs,1,"是");
			return;
		}
	}
	_OutputFail(outputs,2,"否");
}

