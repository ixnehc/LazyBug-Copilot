/********************************************************************
	created:	2022/02/15 
	author:		cxi
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "LevelBehavior.h"

#include "BgnCheckExausted.h"

#include "LevelAttrs.h"


BIND_BGN_CLASS(CBgn_CheckExausted,CBgp_CheckExausted);

void CBgn_CheckExausted::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckExausted*pad=_GetPad<CBgp_CheckExausted>();

	CLevelObj *lo=NULL;
	if (pad->tp==CBgp_CheckExausted::TalkPlayer)
		lo=_GetTalkLo();
	if (pad->tp==CBgp_CheckExausted::Me)
		lo=_GetLo();
	if (lo)
	{
		if (lo->IsAlive())
		{
			LevelAttr_Base *attr=lo->GetAttr_Base();
			if (attr)
			{
				extern float LevelUtil_GetExaustedSP(CLevelObj *lo);
				float spExausted=LevelUtil_GetExaustedSP(lo);
				if (attr->sp.GetMax_Float()<=spExausted+0.001f)
				{
					_OutputOk(outputs,1,"是");
					return;
				}
			}
		}
	}

	_OutputFail(outputs,2,"否");
}


