/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelBehavior.h"
#include "BgnLockPlayer.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"

extern void SetEnumFilter(CLevelObjMap *om,CLevelObj *lo,LevelDetectTargetFlag flags);



////////////////////////////////////////////////////////////////////////
//CBgn_LockPlayer
BIND_BGN_CLASS(CBgn_LockPlayer,CBgp_LockPlayer);
void CBgn_LockPlayer::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		LevelBehaviorContext *ctx=_GetCtx();

		CBgp_LockPlayer*pad=_GetPad<CBgp_LockPlayer>();

		CLevelObjMap *om=lo->GetLevel()->GetObjMap();
		om->SetEnumRange(lo,0.0f,pad->range);

		for (int i=0;i<pad->flagsDetect.size();i++)
			SetEnumFilter(om,lo,(LevelDetectTargetFlag)(pad->flagsDetect[i]|(LevelDetectTarget_Player|LevelDetectTarget_Ground)));

		DWORD c;
		CLevelObj **los=om->Enum(NULL,c);

		for (int i=0;i<c;i++)
		{
			CLevelObj *loDetect=los[i];
			LevelPlayerID idPlayerDetect=loDetect->GetPlayerID();
			Swap(idPlayerDetect,ctx->idPlayerLock);

			_VerifyStbName(1,"玩家条件");
			if (_GetCOut(1))
			{
				_OutputOk(outputs,2,"锁定");
				return;
			}
			Swap(idPlayerDetect,ctx->idPlayerLock);//换回来
		}
	}
	_OutputOk(outputs,3,"未锁定");
}
