
#include "stdh.h"


#include "Skill_TeleportAttack.h"

#include "LevelRecordSkill.h"
#include "LevelRecordUnit.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_TeleportAttack

BIND_SKILLPARAM(Skill_TeleportAttack,SkillParam_TeleportAttack);


void Skill_TeleportAttack::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);

	_tCasting=0;
	_Update(0);
}

BOOL Skill_TeleportAttack::_CalcTeleportPos(LevelPos &pos,LevelFace &face)
{
	extern BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos);
	if (!LevelUtil_CalcTargetPos(_owner->GetLevel(),_target,pos))
		return FALSE;

	LevelPos posMe=_owner->GetFramePos();

	float distKeep=1.0f;
	i_math::vector2df dir=posMe-pos;
	float length=dir.getLength();
	dir.normalize();

	if (length<distKeep)
	{
		return FALSE;
	}

	pos+=dir*distKeep;
	dir=-dir;
	face=LevelFaceFromDir(dir);

	return TRUE;
}


void Skill_TeleportAttack::_Update(AnimTick dt)
{
	if (_state==SkillState_Casting)
	{
		if (_bBroken)
		{
			_SetState(SkillState_Finished);
			return;
		}

		//累加Cast的时间
		extern void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting);
		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		SkillParam_TeleportAttack *param=_rec->GetParam<SkillParam_TeleportAttack>();

		if (!_bTeleported)
		{
			if (_tCasting+LEVEL_FRAME_TICK>param->delayTeleport)
			{
				float faceTeleport=0.0f;
				LevelPos posTeleport;
				if (_CalcTeleportPos(posTeleport,faceTeleport))
					_DoTeleport(posTeleport,faceTeleport);

				_bTeleported=TRUE;
			}
		}

		if (!_bDamaged)
		{
			if (_tCasting+LEVEL_FRAME_TICK>param->delayTeleport+_rec->HitDelay)
			{
				_MakeFanDeals(60.0f,1.5f);

// 				CLevelObj*lo=GetLevel()->GetIDs()->LoFromID(_target.ObjID());
// 				CLevelDecider *decider=GetLevel()->GetDecider();
// 				if (lo)
// 				{
// 					extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
// 					if (LevelUtil_CheckSkillTarget(_rec,_owner,lo))
// 					{
// 						if (decider->CheckInRange(_owner,lo,param->rangeDmg))
// 						{
// 							DealArg arg;
// 							arg.dir.setXZ(lo->GetFramePos()-_owner->GetFramePos());
// 							arg.link.id=GetLevel()->GenOpLinkID();
// 							arg.grd=_grd;
// 
// 							_MakeDeals(lo,arg);
// 						}
// 					}
// 				}

				_bDamaged=TRUE;
			}

		}

		if (_bDamaged&&(_tCasting>_rec->CastTime))
		{
			_SetState(SkillState_Finished);
		}

	}
}


void Skill_TeleportAttack::_OnUpdate(AnimTick dt)
{
	_Update(dt);

}
