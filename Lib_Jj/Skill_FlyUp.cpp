/********************************************************************
	created:	2012/10/22 
	author:		cxi
	
	purpose:	进入某个Agent的技能
*********************************************************************/
#include "stdh.h"


#include "Skill_FlyUp.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"


#include "LevelDecider.h"

#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CSkillGesture_FlyUp


void CSkillGesture_FlyUp::Create(SkillParam_FlyUp *cfg,LevelPos3D &posInitial,LevelFace faceInitial)
{
	float angle=CSysRandom::RandRange(0.0f,2.0f*(float)i_math::Pi);
	i_math::vector2df dir=LevelFaceToDir(faceInitial);

	_step.x=cfg->shift*dir.x;
	_step.z=cfg->shift*dir.y;
	_step.y=cfg->lift;

	_posInitial=posInitial;

	_cfg=cfg;

}


void CSkillGesture_FlyUp::Update(CUnit3D *unit,float dt)
{
	if (_t<_cfg->tDelay)
	{
		_t+=dt;
		return;
	}
	_t+=dt;
	float t=_t-_cfg->tDelay;
	i_math::vector3df pos=unit->_pos;
	pos+=_step*(dt/_cfg->dur);
	pos.y=_posInitial.y+_cfg->vsLift.GetFloat(ANIMTICK_FROM_SECOND(t/_cfg->dur))*_cfg->lift;

	unit->_ClampGround(pos);

	unit->_vel=(pos-unit->_pos)/dt;
	unit->_pos=pos;
	if (_t>=_cfg->tDelay+_cfg->dur)
		_bFinished=TRUE;
}

BOOL CSkillGesture_FlyUp::IsFinished()
{
	return _bFinished;
}


//////////////////////////////////////////////////////////////////////////
//CSkill_Sweep
BIND_SKILLPARAM(Skill_FlyUp,SkillParam_FlyUp);

void Skill_FlyUp::_OnStart()
{
	_SetState(SkillState_Casting);
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	LevelPos3D pos=_owner->GetFramePos3D();
	LevelFace face=_owner->GetFrameFace();

	CLevelObjMove *move=_owner->GetMove();
	if (move)
	{
		move->SwitchFlying(pos,face,LevelTeleportID_Invalid);
	}

	SkillParam_FlyUp*param=_rec->GetParam<SkillParam_FlyUp>();
	if (param)
	{
		CUnit3D *unit=_owner->GetUnit3D();
		if (unit)
		{
			CSkillGesture_FlyUp *ges=Class_New2(CSkillGesture_FlyUp);
			ges->Create(param,pos,face);
			ges->AddRef();
			unit->SetGesture(ges);
			SAFE_RELEASE(ges);
		}
	}

}


void Skill_FlyUp::_OnUpdate(AnimTick dt)
{
	if (_state==SkillState_Casting)
	{
		SkillParam_FlyUp*param=_rec->GetParam<SkillParam_FlyUp>();
		if (param)
		{
			if (_GetAge()>ANIMTICK_FROM_SECOND(param->dur/2.0f))
				_Finish();
		}
		else
			_Finish();
	}

}


void Skill_FlyUp::_Finish()
{
	_SetState(SkillState_Finished);
}
