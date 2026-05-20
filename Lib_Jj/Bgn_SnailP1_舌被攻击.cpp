/********************************************************************
	created:	2020/8/21 
	author:		cxi
	
	*******************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"


#include "Bgn_SnailP1_舌被攻击.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"

#include "LoSnailP1.h"

#include "LevelEventSrc.h"

#include "Random/Random.h"

#include "Log/LogDump.h"

#include "Buff_TongueFly.h"



////////////////////////////////////////////////////////////////////////
//CBgn_SnailP1_舌被攻击
BIND_BGN_CLASS(CBgn_SnailP1_舌被攻击,CBgp_SnailP1_舌被攻击);
 

void CBgn_SnailP1_舌被攻击::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_SnailP1_舌被攻击*pad=_GetPad<CBgp_SnailP1_舌被攻击>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	extern Buff_TongueFly *FindTongueFlyBuff(CLevel *level);
	Buff_TongueFly *buff=FindTongueFlyBuff(level);
	if (buff)
	{
		LevelObjID idKnot=buff->GetKnotID();
		if (idKnot!=LevelObjID_Invalid)
		{
			CLevelObj *lo=LevelUtil_GetAliveLo(level,idKnot);
			if (lo)
			{
				CLevelEventSrc *src=lo->GetEventSrc();
				if (src)
				{
					AnimTick tAfter=ctx->level->GetT_();
					tAfter=ANIMTICK_SAFE_MINUS(tAfter,pad->_dur);
					LevelEventTypeMask mask=(1<<(DWORD)LET_Hit)|(1<<(DWORD)LET_Damage);
					if (src->ExistWithMask(mask,tAfter,(LevelObjID *)NULL))
					{
						_OutputOk(outputs,1,"是");
						return;
					}
				}
			}
		}
	}

	_OutputFail(outputs,2,"否");
	return;
}
