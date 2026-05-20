/********************************************************************
	created:	2022/2/9 
	
	purpose:	GA功能:得到基本属性
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"


#include "LevelOSB.h"

#include "BgnGA_GetBaseAttr.h"

#include "LevelObj.h"

#include "LevelAttrs.h"
#include "LevelBGs.h"

////////////////////////////////////////////////////////////////////////
//CBgnGA_GetBaseAttr
BIND_BGN_CLASS(CBgnGA_GetBaseAttr,CBgpGA_GetBaseAttr);


void CBgnGA_GetBaseAttr::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_GetBaseAttr*pad=_GetPad<CBgpGA_GetBaseAttr>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	CLevelObj *lo=_GetTalkLo();
	if(lo)
	{
		short c=0;
		switch(pad->tp)
		{
			case 0://MaxHP
			{
				LevelAttr_Base *attr=lo->GetAttr_Base();
				c=attr->hp.GetMax_Int();
				break;
			}
		}
		_SetNumber(pad->var,(short)c);
	}

	_OutputOk(outputs,1,"结束");
	return;
}
