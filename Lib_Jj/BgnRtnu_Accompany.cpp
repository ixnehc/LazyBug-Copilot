/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelRecords.h"
#include "LevelUtil.h"
#include "LevelPlayer.h"

#include "LevelRtnuCircum.h"

#include "BgnRtnu_Accompany.h"

#include "LevelObjMove.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Accompany
BIND_BGN_CLASS(CBgnRtnu_Accompany,CBgpRtnu_Accompany);

void CBgnRtnu_Accompany::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpRtnu_Accompany*pad=_GetPad<CBgpRtnu_Accompany>();

	CLevelPlayer *player=_GetLockPlayer();
	if (player)
	{
		CLevelObj *lo=_GetLo();
		if (lo)
		{
			CLevelObjMove *move=lo->GetMove();
			if (move)
				move->SwitchRtnuMode(TRUE,LevelRtnuRank_Knight);
		}
	}

}

void CBgnRtnu_Accompany::Update(BGNOutputs &outputs)
{
}

void CBgnRtnu_Accompany::Destroy()
{
	CLevelPlayer *player=_GetLockPlayer();
	if (player)
	{
		CLevelObj *lo=_GetLo();
		if (lo)
		{
			CLevelObjMove *move=lo->GetMove();
			if (move)
				move->SwitchRtnuMode(FALSE,LevelRtnuRank_None);
		}
	}

}


void CBgnRtnu_Accompany::Break(BGNOutputs &outputs)
{
	Destroy();
}

