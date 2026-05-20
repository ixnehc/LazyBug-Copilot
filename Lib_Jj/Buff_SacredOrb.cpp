
#include "stdh.h"

#include "Level.h"

#include "LevelRecordBuff.h"

#include "Buff_SacredOrb.h"

#include "datapacket/BitPacket.h"

#include "LoStarPlate.h"

BIND_BUFFPARAM(Buff_SacredOrb,BuffParam_SacredOrb,BuffArg_SacredOrb);

BOOL Buff_SacredOrb::CanDispel()
{
	CLevel *level=_GetLevel();
	if (_tRecentCharged==ANIMTICK_INFINITE)
		return FALSE;
	if (level->GetT_()>_tRecentCharged+ANIMTICK_FROM_SECOND(2.0f))
		return FALSE;
	return TRUE;
}


BOOL Buff_SacredOrb::_IsCharged()
{
	BOOL bCharged=FALSE;
	CLevel *level=_GetLevel();
	CLoStarPlate *loStarPlate=(CLoStarPlate *)level->GetUniqueObj(LevelUniqueObj_StarPlate);
	if (loStarPlate)
		bCharged=loStarPlate->CheckSacredOrbCharged();
	return bCharged;
}


void Buff_SacredOrb::HandleEvent(LevelEvent &e0)
{
	CLevelObj *owner=_GetOwner();

	if (e0.GetType()==LET_ModDamageAttr)
	{
		LeModDamageAttr *e=(LeModDamageAttr*)&e0;
		if (e->osbSrc)
		{
			if (e->osbSrc->GetRootOwner()==owner)
			{
				if (e->dmgs)
				{
					if (!_bTriggered)
					{
						if (_CanTrigger())
						{
							_bTriggered=TRUE;
							_bFired=FALSE;

						}
					}
				}
			}
		}
		return;
	}

	BOOL bNeedFire=FALSE;
	LevelOpLink link;
	CLevelObj *loTarget=NULL;

	if (e0.GetType()==LET_PreKill)
	{
		LePreKill *e=(LePreKill*)&e0;

		if (e->osbSrc)
		{
			if (e->osbSrc->GetRootOwner()==owner)
			{
				if (_bTriggered)
				{
					bNeedFire=TRUE;
					link=e->link;
					loTarget=e->loTarget;
				}
			}
		}
	}

	if (e0.GetType()==LET_PostDamage)
	{
		LePostDamage *e=(LePostDamage*)&e0;
		if (e->osbSrc->GetRootOwner()==owner)
		{
			if (_bTriggered)
			{
				bNeedFire=TRUE;
				link=e->link;
				loTarget=e->loTarget;
			}
		}
	}

	if (bNeedFire)
	{
		if (!_bFired)
		{
			LevelOp_Dummy *op=NewOp<LevelOp_Dummy>(link);
			op->t=GetAge();
			loTarget->AddOp(op);

			CLevel *level=_GetLevel();
			CLoStarPlate *loStarPlate=(CLoStarPlate *)level->GetUniqueObj(LevelUniqueObj_StarPlate);
			if (loStarPlate)
				loStarPlate->NotifySacredOrbFire();

			_tRecentFire=level->GetT_();
		}

		_bFired=TRUE;
	}
}

void Buff_SacredOrb::_OnUpdate(AnimTick dt)
{
	_bTriggered=FALSE;
	_bFired=FALSE;

	if (TRUE)
	{
		BOOL bCharged=_IsCharged();
		if (bCharged)
			_tRecentCharged=_GetLevel()->GetT_();
	}
}

BOOL Buff_SacredOrb::_CanTrigger()
{
	const float durFireCD=2.5f;
	if (_tRecentFire!=ANIMTICK_INFINITE)
	{
		if (_GetLevel()->GetT_()<_tRecentFire+ANIMTICK_FROM_SECOND(durFireCD))
			return FALSE;
	}
	return _IsCharged();
}
