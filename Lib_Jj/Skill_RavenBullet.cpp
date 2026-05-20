
#include "stdh.h"


#include "Skill_RavenBullet.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"
#include "Buff_FlyBirth.h"

////////////////////////////////////////////////////////////////////////
//CLevelGesture_RavenThrust
void CLevelGesture_RavenThrust::Create(CLevelObj *loSrc,CLevelObj *loTarget,SkillParam_RavenBullet *param)
{
	SAFE_REPLACE(_loTarget,loTarget);
	_param=param;

	_gtm=loTarget->GetLevel()->GetGtm();

	_dirInitial=loTarget->GetFramePos();
	_dirInitial-=loSrc->GetFramePos();

	_htInitial=loTarget->GetFramePos3D().y;

	_dirInitial.safe_normalize();
}

void CLevelGesture_RavenThrust::Destroy()	
{	
	SAFE_RELEASE(_loTarget);
	Zero();	
	Release();
}

void CLevelGesture_RavenThrust::Update(CUnit3D *unit,float dt)
{
	const float blend=0.4f;
	i_math::vector3df pos=unit->GetPos();
	i_math::vector3df vel=unit->GetVel();
	if (!_bPassBy)
	{
		LevelPos posTarget;

		if (_loTarget)
		{
			if (!_loTarget->IsAlive())
				SAFE_RELEASE(_loTarget);
		}

		if (_loTarget)
		{
			posTarget=_loTarget->GetFramePos();

			LevelPos dir;
			dir.x=posTarget.x-pos.x;
			dir.y=posTarget.y-pos.z;

			dir.safe_normalize();

			float ht=_gtm->GetHeight(posTarget.x,posTarget.y)+_param->htThrust;

			float adjust=-i_math::clamp_f((pos.y-ht)/4.0f,0.0f,1.0f);

			vel.set(dir.x,adjust,dir.y);
			vel.setLength(_param->spdThrust);
		}
		unit->AccumVelPos(vel,blend,dt);

		//检查是否Pass By了
		if (_loTarget)
		{
			i_math::vector3df posNew=unit->GetPos();

			LevelPos dir;
			dir.set(posTarget.x-posNew.x,posTarget.y-posNew.z);
			dir.safe_normalize();

			if (dir.dotProduct(_dirInitial)<0.0f)
			{
				_bPassBy=TRUE;
				SAFE_RELEASE(_loTarget);
			}
		}
	}
	else
	{
		vel.normalize();
		float adjust=-i_math::clamp_f((pos.y-_htInitial)/4.0f,0.0f,1.0f);
		vel.y+=adjust;
		vel.setLength(unit->GetSpeed());

		unit->AccumVelPos(vel,blend,dt);
	}
}




//////////////////////////////////////////////////////////////////////////
//Skill_RavenBullet

BIND_SKILLPARAM(Skill_RavenBullet,SkillParam_RavenBullet);


void Skill_RavenBullet::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	BOOL bOk=FALSE;

	CLevelObj *lo=_owner->GetLevel()->GetIDs()->LoFromID(_target.ObjID());
	if (lo)
	{
		SkillParam_RavenBullet*param=_rec->GetParam<SkillParam_RavenBullet>();
		if (param)
		{
			CUnit3D *unit=_owner->GetUnit3D();
			if (unit)
			{
				_ges=Class_New2(CLevelGesture_RavenThrust);
				_ges->Create(_owner,lo,param);
				_ges->AddRef();
				unit->SetGesture(_ges);
				bOk=TRUE;
			}
		}
	}

	if (!bOk)
		_SetState(SkillState_Fail);
	else
	{
		_SetState(SkillState_Casting);
		_tCasting=0;
	}
}

void Skill_RavenBullet::_UpdateCast(AnimTick dt)
{
	_tCasting+=dt;

	if (_state==SkillState_Casting)
	{
		CLevelObj *loTarget=NULL;
		if (TRUE)
		{
			CLevel *level=_owner->GetLevel();
			loTarget=level->GetIDs()->LoFromID(_target.ObjID());
		}

		if (!loTarget)
		{
			_SetState(SkillState_Finished);
			return;
		}

		SkillParam_RavenBullet*param=_rec->GetParam<SkillParam_RavenBullet>();

		if (param)
		{
			float dist2=loTarget->GetFramePos().getDistanceSQFrom(_owner->GetFramePos());
			if (dist2<param->rangeFire*param->rangeFire)
			{
// 				_bullet=Class_New2(CThrowBullet);
// 				LevelPos3D posSrc=_owner->GetFramePos3D();
// 				LevelPos3D posTarget=loTarget->GetFramePos3D();
// 				posTarget.y+=loTarget->GetAimHeight();
// 				_bullet->Init(posSrc,posTarget,param->spdBullet,param->radiusBullet,_owner,_rec);
				
				_SetState(SkillState_Casted);

				//发送一个Op,通知发射
				if (TRUE)
				{
					LevelOp_StartFire *op=NewOp<LevelOp_StartFire>(LevelOpLink());
					_owner->AddOp(op);
				}

			}
		}
	}


}

void Skill_RavenBullet::_OnUpdate(AnimTick dt)
{
	_UpdateCast(dt);
	_UpdateBullet(dt);
}


void Skill_RavenBullet::_OnFinish()
{
	SAFE_RELEASE(_ges);
// 	Safe_Class_Delete(_bullet);
}


void Skill_RavenBullet::_UpdateBullet(AnimTick dt)
{
	if (_state!=SkillState_Casted)
		return;

// 	if (_bullet)
// 	{
// 		LevelObjHits hits;
// 		BulletStaticHit hitStatic;
// 		if (!_bullet->Update((float)LEVEL_SKILL_UPDATE_INTERVAL,hits,hitStatic))
// 		{
// 			if (!hits.IsEmpty())
// 			{
// 				CLevelDecider *decider=GetLevel()->GetDecider();
// 				CLoUnit *lo=GetLevel()->GetIDs()->LoFromID<CLoUnit>(hits.ids[0]);
// 				if (lo)
// 				{
// 
// 					i_math::vector3df dir3D=_bullet->GetDir();
// 
// 					DealArg arg;
// 					arg.dir=dir3D;
// 					arg.link.id=GetLevel()->GenOpLinkID();
// 					arg.grd=_grd;
// 
// 					_MakeDeals(lo,arg);
// 				}
// 			}
// 			Safe_Class_Delete(_bullet);
// 		}
// 	}
// 	if (!_bullet)
// 		_SetState(SkillState_Finished);
}
