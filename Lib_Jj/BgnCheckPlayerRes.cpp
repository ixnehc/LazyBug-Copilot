/********************************************************************
	created:	2013/01/25 
	author:		cxi
	
	purpose:	 检查HP的范围
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LevelAttrs.h"

#include "BgnCheckPlayerRes.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckPlayerRes

BIND_BGN_CLASS(CBgn_CheckPlayerRes,CBgp_CheckPlayerRes);

void CBgn_CheckPlayerRes::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckPlayerRes*pad=_GetPad<CBgp_CheckPlayerRes>();

	CBehavior*bhv=_bhv;

	if (pad->tp!=LevelResource_None)
	{
		int vRef=pad->nRef;
		CLevelPlayer *player=_GetTalkPlayer();
		extern int LevelUtil_GetResCount(CLevelPlayer *player,LevelResourceType tp);
		int v=LevelUtil_GetResCount(player,pad->tp);

		BOOL b=FALSE;
		switch(pad->op)
		{
			case CBgp_CheckPlayerRes::EQ:
				b=(v==vRef);break;
			case CBgp_CheckPlayerRes::NE:
				b=(v!=vRef);break;
			case CBgp_CheckPlayerRes::GE:
				b=(v>=vRef);break;
			case CBgp_CheckPlayerRes::GT:
				b=(v>vRef);break;
			case CBgp_CheckPlayerRes::LE:
				b=(v<=vRef);break;
			case CBgp_CheckPlayerRes::LT:
				b=(v<vRef);break;
		}
		if (b)
		{
			_OutputOk(outputs,1,"是");
			return;
		}
		else
		{
			_OutputFail(outputs,2,"否");
			return;
		}
	}

	_SetResult(A_Fail);
}


