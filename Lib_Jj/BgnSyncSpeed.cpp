/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "LevelBGs.h"

#include "LevelBehavior.h"
#include "BgnSyncSpeed.h"
#include "LevelAttrs.h"

#include "LevelObjMove.h"
#include "LevelObj.h"

#include "LevelSkillDriver.h"

////////////////////////////////////////////////////////////////////////
//CBgn_SyncSpeed
BIND_BGN_CLASS(CBgn_SyncSpeed,CBgp_SyncSpeed);

void CBgn_SyncSpeed::Start(DWORD iStb,BGNOutputs &outputs)
{
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		CLevelObjMove *move=lo->GetMove();
		if (move)
		{
			float speed=_GetLockPlayerSpeed();
			SpeedMod *mod=move->ObtainSpeedMod();
			if (mod)
			{
				_attrnode=mod->speed.Add(speed,100);
				_attrnodeFlying=mod->speedFlying.Add(speed,100);
			}
		}
	}
}

void CBgn_SyncSpeed::Update(BGNOutputs &outputs)
{
	float speed=_GetLockPlayerSpeed();
	if (_attrnode)
		((AttrNodeFloat*)_attrnode)->v=speed;
	if (_attrnodeFlying)
		((AttrNodeFloat*)_attrnodeFlying)->v=speed;
}

void CBgn_SyncSpeed::Destroy()
{
	SAFE_DESTROY(_attrnode);
	SAFE_DESTROY(_attrnodeFlying);
}


void CBgn_SyncSpeed::Break(BGNOutputs &outputs)
{
	Destroy();
}
