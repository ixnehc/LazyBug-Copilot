/********************************************************************
	created:	2023/05/19
	author:		cxi
	
	purpose:	SlimeµÄBuff
*********************************************************************/
#include "stdh.h"

#include "Level.h"
#include "LevelUtil.h"

#include "LoUnit.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"


#include "Buff_Slime.h"



//////////////////////////////////////////////////////////////////////////
//CBuff_Slime
BIND_BUFFPARAM(Buff_Slime,BuffParam_Slime,BuffArg_Slime);

LevelBuffMask Buff_Slime::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Slime)->GetUID());

	return mask;
}

void Buff_Slime::_OnCreate(LevelBuffArg *arg)
{
	BuffParam_Slime *param=(BuffParam_Slime*)_param;

	CLevelObj *owner=_GetOwner();
	_pos=owner->GetFramePos();

	_state=State_Exhausted;
	_tStateStart=_GetLevel()->GetT_();

	LevelAttr_Base *attrBase=owner->GetAttr_Base();
	if (attrBase)
	{
		_hp=attrBase->hp.GetCur_Float();
		_hpMax=attrBase->hp.GetMax_Float();
	}
	else
		_hp=_hpMax=100.0f;

	if (_hp>_hpMax*0.8f)
		_hp=_hpMax*0.8f;

}

void Buff_Slime::_OnUpdate(AnimTick dt)
{
	BuffParam_Slime *param=(BuffParam_Slime*)_param;
	float fDt=ANIMTICK_TO_SECOND(dt);
	CLevelObj *owner=_GetOwner();
	CLevel *level=owner->GetLevel();
	AnimTick tCur=level->GetT_();

	const float radiusTrample=0.2f;
	const float dmg0=_hpMax/param->durTrampleDmg;
	const float recover0=_hpMax/param->durRecover;

	LevelPos pos=_GetOwner()->GetFramePos();

	CLoUnit *loPlayer=NULL;
	if (TRUE)
	{
		CLevelPlayer *player=LevelUtil_GetFirstPlayer(_GetOwner()->GetLevel());
		if (player)
			loPlayer=player->GetLoUnit();
	}

	float dist=pos.getDistanceFrom(loPlayer->GetFramePos());

	BOOL inTrampleRange=FALSE;
	if (dist<radiusTrample)
		inTrampleRange=TRUE;

	State stateOld=_state;

	switch(_state)
	{
		case State_Dead:
		{
			if (!inTrampleRange)
				_durDead+=dt;

			if (TRUE)
			{
				if (!level->GetEoEnv())
					_bDeadPermanently=TRUE;
			}
			if (!_bDeadPermanently)
			{
				if (_durDead>ANIMTICK_FROM_SECOND(param->durDead))	
				{
					_state=State_Exhausted;
					_hp=_hpMax*0.33f;
					LevelUtil_RemoveBuffByRecordID(owner,param->idBuff_NotAttackable);
				}
			}
			break;
		}
		case State_Exhausted:
		case State_Trampled:
		{
			if (inTrampleRange)
			{
				float dmg=dmg0*fDt;
				_hp-=dmg;
				if (_hp<0.0f)
					_hp=0.0f;
			}
			else
			{
				float recover=recover0*fDt;
				_hp+=recover;
				if (_hp>_hpMax)
					_hp=_hpMax;
			}

			if (inTrampleRange)
				_state=State_Trampled; 
			else
				_state=State_Exhausted;

			if (_state==State_Trampled)
			{
				if (_hp<=0.0f)
				{
					if (param->idBuff_NotAttackable!=RecordID_Invalid)
						level->GetDecider()->MakeBuff(owner,param->idBuff_NotAttackable,ANIMTICK_INFINITE,NULL,TRUE);
					_state=State_Dead;
					_durDead=0;
				}
			}
			else
			{
				if (_hp>=_hpMax)
					_state=State_Ready;
			}

			break;
		}
		case State_Ready:
			break;

	}

	BOOL bNeedSync=FALSE;
	if (_state!=stateOld)
	{
		_tStateStart=tCur;
		bNeedSync=TRUE;
		_buffs->MarkFlagsDirty();
	}

	if (_hpSent!=(int)_hp)
		bNeedSync=TRUE;

	if (bNeedSync)
		_AddSyncDataOp();

}

void Buff_Slime::_OnDestroy()
{

}


void Buff_Slime::_WriteData(CBitPacket *dp)
{
	dp->Bits_Write(_state,4);

	_hpSent=(int)_hp;

	dp->Bits_Write(_hpSent,14);
}
