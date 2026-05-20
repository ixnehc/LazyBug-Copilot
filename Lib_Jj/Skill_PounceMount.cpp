/********************************************************************
	created:	2012/10/22 
	author:		cxi
	
	purpose:	猛扑Mount的技能
*********************************************************************/
#include "stdh.h"


#include "Skill_PounceMount.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"

#include "Buff_Stun.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_Sweep
BIND_SKILLPARAM(Skill_PounceMount,SkillParam_PounceMount);

CLevelObj *Skill_PounceMount::_GetLoTarget()
{
	if (_target.tp==LevelSkillTarget::Target_DefObj)
	{
		LevelObjID id=_target.ObjID();
		return _owner->GetLevel()->GetIDs()->LoFromID(id);
	}
	return NULL;
}

BOOL Skill_PounceMount::_WriteSyncData(CBitPacket *bp)
{
	bp->Data_NextByte()=(BYTE)_stage;
	return TRUE;
}



void Skill_PounceMount::_OnStart()
{
	_SetState(SkillState_Casting);
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	CLevelObj *loTarget=_GetLoTarget();

	BOOL bOk=FALSE;

	if (loTarget)
	{
		if (loTarget->GetMove())
		{
			_posLock=loTarget->GetFramePos();
			_posOrg=_owner->GetFramePos();
			_stage=Wait;
			bOk=TRUE;
		}
	}

	if (!bOk)
		_SetState(SkillState_Finished);

}

extern void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting);
extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);

void Skill_PounceMount::_OnUpdate(AnimTick dt)
{
	if (_state!=SkillState_Casting)
		return;

	if (_stage==None)
		return;

	SkillParam_PounceMount *param=_rec->GetParam<SkillParam_PounceMount>();
	if (_stage==Wait)
	{
		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		CLevelObj *loTarget=_GetLoTarget();
		BOOL bCancel=!LevelUtil_CheckSkillTarget(_rec,_owner,loTarget);
		if(!bCancel)
		{
			LevelPos posTarget=loTarget->GetFramePos();
			float dist2=posTarget.getDistanceSQFrom(_posOrg);
			if ((dist2>param->rangeMax*param->rangeMax)||
				(dist2<param->rangeMin*param->rangeMin))
			{
				bCancel=TRUE;
			}
		}

		if (!bCancel)
		{
			if (_tCasting>=param->durWait)
			{
// 				CLevelObjMove *move=_owner->GetMove();
// 				move->SwitchMount(_target.ObjID(),0.0f,LevelTeleportID_Invalid);

				_stage=Bite;
				_tCasting-=param->durWait;

				//加一个Stun Buff
				if (param->idStun!=RecordID_Invalid)
				{
					BuffArg_Stun arg;
					LevelStrike strike;
					LevelPos dir=loTarget->GetFramePos();
					dir-=_posOrg;
					strike.SetDir(dir);
					arg.strike=strike;
					AnimTick durStun=param->durBite+param->durBack/2;
					_owner->GetLevel()->GetDecider()->MakeBuff(LevelOSB(this),loTarget,param->idStun,durStun,&arg,LevelOpLink());
				}

				_AddSyncDataOp();//同步一下
			}
		}
		else
		{
			_stage=WaitCanceled;
			_AddSyncDataOp();//同步一下
			_SetState(SkillState_Finished);
		}
	}

	if (_stage==Bite)
	{
		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		if (!_bBiteDealed)
		{
			if (_tCasting>=param->tDmgDelay)
			{
				CLevelObj *loTarget=_GetLoTarget();
				if (LevelUtil_CheckSkillTarget(_rec,_owner,loTarget))
				{
					DealArg arg;
					arg.link.id=GetLevel()->GenOpLinkID();
					arg.grd=_grd;

					_MakeDeals(loTarget,arg);
				}

				_bBiteDealed=TRUE;
			}
		}

		if (_tCasting>=param->durBite)
		{
// 			LevelTeleportID idTeleport=_owner->GenTeleportID();

// 			CLevelObjMove *move=_owner->GetMove();
// 			move->SwitchGround(_posOrg,LevelTeleportID_Invalid);

			_stage=Back;
			_tCasting-=param->durBite;
			_AddSyncDataOp();//同步一下
		}
	}

	if (_stage==Back)
	{
		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		if (_tCasting>=param->durBack)
		{
			_stage=Finish;
			_AddSyncDataOp();//同步一下
			_SetState(SkillState_Finished);
		}

	}


}

void Skill_PounceMount::_OnBreak()	
{		
	assert(!IsImmune());
	extern BOOL LevelUtil_CheckDamageImmune(CLevelObj *lo);
	LevelUtil_CheckDamageImmune(_owner);
	_SetState(SkillState_Finished);	
}
