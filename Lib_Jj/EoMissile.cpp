
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoMissile.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "LevelSensor.h"

#include "LoMagicCircuit.h"

#include "Random/Random.h"

//////////////////////////////////////////////////////////////////////////
//CMissile
void CMissile::Init(EoMissile *owner,LevelPos3D &src,i_math::vector3df&dir,EoParamMissile *param)
{
	_owner=owner;

	CBulletBase::Init(owner,param,src,param->radius,param->fall,param->distIgnoreStatic,param->bMH);
	_param=param;
	_euler=dir;
	_euler.toEuler();
	_t=0.0f;
	_speed=_speedMax=CSysRandom::RandVary(param->speed,param->speedVary);

	_idLockTarget=LevelObjID_Invalid;

	if (param->modeLockTarget==EoParamMissile::LockTargetMode_Default)
	{
		CLevelSkill *skill=owner->GetRootSkill();
		if (skill)
		{
			CLevelObj *loTarget=LevelUtil_GetTargetObj(_owner->GetLevel(),skill->GetTarget());
			if (loTarget)
				_idLockTarget=loTarget->GetID();
		}

		if (_idLockTarget==LevelObjID_Invalid)
		{
			LevelObjID idSrcOwner=owner->GetRootOwnerID();
			if (idSrcOwner)
			{
				CLevelObj *loSrcOwner=LevelUtil_GetAliveLo(_owner->GetLevel(),idSrcOwner);
				if (loSrcOwner)
				{
					CLevelSensor *sensor=loSrcOwner->GetSensor();
					if (sensor)
					{
						CLevelObj *loThreat=sensor->GetThreat();
						if (loThreat)
							_idLockTarget=loThreat->GetID();
					}
				}
			}
		}
		_bTracing=TRUE;
	}
	if (param->modeLockTarget==EoParamMissile::LockTargetMode_Host)
	{
		_idLockTarget=owner->GetHost();
		_bTracing=TRUE;
	}

	if (param->modeLockTarget==EoParamMissile::LockTargetMode_Moth)
	{
		_bTracing=FALSE;
	}

	if (param->modeLockTarget==EoParamMissile::LockTargetMode_Special_MagicCircuitCrystalTarget)
	{
		CLoMagicCircuit *loMagicCircuit=(CLoMagicCircuit *)owner->GetLevel()->GetUniqueObj(LevelUniqueObj_MagicCircuit);
		if (loMagicCircuit)
			_idLockTarget=loMagicCircuit->GetCrystalTarget();
		_bTracing=TRUE;
	}

}

LevelObjID CMissile::_DetectHit_ShieldAmulet(i_math::line3df &line)
{
	if (_owner)
	{
		LevelObjID idIgnore=LevelObjID_Invalid;

		if (_param->bIgnoreHost)
		{
			if (_t<1.0f)
				idIgnore=((EoMissile*)_owner)->GetHost();
		}

		LevelEoDetectHitArg argHit;
		argHit.radius=_param->radius;
		argHit.fall=_param->fall;
		argHit.idIgnore=idIgnore;

		CLevelObj *hit=_owner->DetectHit_ShieldAmulet(line,argHit);
		if (hit)
			return hit->GetID();
	}
	return LevelObjID_Invalid;
}


LevelObjID CMissile::_DetectHit(i_math::line3df &line)
{
	if (_owner)
	{
		LevelObjID idIgnore=LevelObjID_Invalid;

		if (_param->bIgnoreHost)
		{
			if (_t<1.0f)
				idIgnore=((EoMissile*)_owner)->GetHost();
		}

		LevelEoDetectHitArg argHit;
		argHit.radius=_param->radius;
		argHit.fall=_param->fall;
		argHit.idIgnore=idIgnore;
		if (_param->modeLockTarget==EoParamMissile::LockTargetMode_Special_MagicCircuitCrystalTarget)
		{
			argHit.idSpecify=_idLockTarget;
			argHit.bAgent=TRUE;
		}

		CLevelObj *hit=_owner->DetectHit(line,argHit);
		if (hit)
			return hit->GetID();
	}
	return LevelObjID_Invalid;
}


void CMissile::_UpdateStep(AnimSecond dt,LevelPos3D &dir,float &dDist)
{
	if (_bTracing)
	{
		if (_speed<_speedMax)
		{
			_speed+=((float)dt)*_param->accel;
			if (_speed>_speedMax)
				_speed=_speedMax;
		}
	}
	else
	{
		if (_speed>_param->speedWonder)
		{
			_speed-=((float)dt)*_param->accel;
			if (_speed<_param->speedWonder)
				_speed=_param->speedWonder;
		}
	}

	if (_speed<_param->speedWonder+1.0f)
	{
		_speedMin+=((float)dt)*180.0f;
		if (_speedMin>60.0f)
			_speedMin=60.0f;
	}


	if (!_bTracing)
	{
		if (_param->modeLockTarget==EoParamMissile::LockTargetMode_Moth)
		{
			if (_owner)
			{
				CLevelObj *loTarget=_owner->DetectFirstInRange(_pos.getXZ(),_param->radiusDetect);
				if (loTarget)
				{
					_idLockTarget=loTarget->GetID();
					_bTracing=TRUE;
				}
			}
		}
	}

	if (_bTracing)
	{
		if(_owner)
		{
			CLevelObj *loTarget=LevelUtil_GetAliveLo(_owner->GetLevel(),_idLockTarget);
			if (loTarget)
			{
				LevelPos3D posTarget=loTarget->GetFramePos3D();
				posTarget.y+=loTarget->GetAimHeight();
				if (_param->modeLockTarget==EoParamMissile::LockTargetMode_Special_MagicCircuitCrystalTarget)
					posTarget.y+=1.0f;
				LevelPos3D posMe=GetPos();

				if (posMe.getDistanceFromSQ(posTarget)>0.0004f)
				{
					LevelPos3D dirTarget=posTarget-posMe;
					dirTarget.normalize();

					LevelPos3D eulerTarget;
					eulerTarget=dirTarget;
					eulerTarget.toEuler();

					float dRad=(float)dt*_param->speedRot*i_math::GRAD_PI2;
					i_math::rotate_limited(_euler.x,eulerTarget.x,dRad);
					i_math::rotate_limited(_euler.y,eulerTarget.y,dRad);
					i_math::rotate_limited(_euler.z,eulerTarget.z,dRad);
				}
			}
		}
	}
	else
	{
		if (_speedMin>0.0f)		
		{
			float dRad=(float)dt*_speedMin*i_math::GRAD_PI2;

			float radRandom=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
			i_math::rotate_limited(_euler.x,radRandom,dRad);
		}
	}
	dir=_euler;
	dir.eulerToDir();
	dDist=((float)dt)*_speed;

	_t+=(float)dt;
}

BOOL CMissile::_NeedStop()
{
	if (_t>=_param->dur)
		return TRUE;
	return FALSE;
}




//////////////////////////////////////////////////////////////////////////
//EoMissile
BIND_EOPARAM(EoMissile,EoParamMissile);

CBulletBase *EoMissile::_CreateBullet()
{
	CMissile *bullet=Class_New2(CMissile);
	EoParamMissile*param=GetParam<EoParamMissile>();
	bullet->Init(this,_GetInitialPos3D(),_GetInitialDir3D(),param);
	bullet->SetHiResoStaticCheck(TRUE);

	return bullet;
}
void EoMissile::_DestroyBullet(CBulletBase *bullet0)
{
	CMissile *bullet=(CMissile *)bullet0;
	Safe_Class_Delete(bullet);
}

void EoMissile::_WritePos(CBitPacket *bp)
{
	if (_core->IsStop())
	{
		bp->Bit_Write_0();
		bp->Data_WriteSimpleR(_core->GetDir());
	}
	else
	{
		bp->Bit_Write_1();
		bp->Data_WriteSimpleR(_core->GetPos());
	}

}


void EoMissile::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	__super::_OnWriteFirstSync(bp,bContent,idPlayer);
}


void EoMissile::_OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	__super::_OnWriteSyncH(bp,bContent,idPlayer);

	_WritePos(bp);
	bContent=TRUE;
}

