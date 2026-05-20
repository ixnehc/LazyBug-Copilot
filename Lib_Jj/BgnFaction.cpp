/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelBehavior.h"
#include "BgnFaction.h"

#include "LevelObj.h"


////////////////////////////////////////////////////////////////////////
//CBgn_SetFaction
BIND_BGN_CLASS(CBgn_SetFaction,CBgp_SetFaction);
void CBgn_SetFaction::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		CBgp_SetFaction*pad=_GetPad<CBgp_SetFaction>();

		if (pad->relation==0)
			lo->SetPlayerID(LevelPlayerID_Wild);
		if (pad->relation==1)
			lo->SetPlayerID(LevelPlayerID_NeutralWild);
		if (pad->relation==2)
			lo->SetPlayerID(LevelPlayerID_PlayerWild);
	}
	_OutputOk(outputs,1,"结束");
}
