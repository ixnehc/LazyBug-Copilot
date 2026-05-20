
#include "stdh.h"


#include "Skill_Plunge.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"

#include "LevelObjMove.h"

////////////////////////////////////////////////////////////////////////
//CSkillGesture_Plunge
void CSkillGesture_Plunge::Create(LevelObjID idTarget,float speed,CLevel *level)
{
	_idTarget=idTarget;
	_level=level;
	_speed=speed;
}



void CSkillGesture_Plunge::Update(CUnit3D *unit,float dt)
{
	if (_nReachCount>1)
	{
		_bFinished=1;
		return;
	}
	extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
	CLevelObj *loTarget=LevelUtil_GetAliveLo(_level,_idTarget);
	if (!loTarget)
	{
		_bFinished=1;
		return;
	}

	LevelPos3D posTarget=loTarget->GetFramePos3D();
	posTarget.y+=loTarget->GetAimHeight();

	LevelPos3D vel=posTarget-unit->_pos;
	if (vel.getLengthSQ()<0.25f)
	{
		_bFinished=1;
		return;
	}

	vel.setLength(_speed);

	i_math::line3df line;
	line.start=unit->_pos;
	float blend=0.5f;
	vel=unit->_vel*(1.0f-blend)+vel*blend;
	unit->_pos+=vel*dt;
	unit->_vel=vel;

	unit->_ClampGround(unit->_pos);

	line.end=unit->_pos;
	LevelPos3D vHit;
	extern BOOL LevelUtil_UnitHitTest(i_math::line3df &line,i_math::vector3df &center,float radius,float fall,float height,i_math::vector3df &vHit);
	if (LevelUtil_UnitHitTest(line,loTarget->GetFramePos3D(),loTarget->GetRadius_(),0.0f,loTarget->GetHeight(),vHit))
	{
		unit->_pos=posTarget;
		_nReachCount++;
	}
	else
		_nReachCount=0;
}



//////////////////////////////////////////////////////////////////////////
//CSkill_Charge
BIND_SKILLPARAM(Skill_Plunge,SkillParam_Plunge);


void Skill_Plunge::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);
	_SetState(SkillState_Casting);


	BOOL bOk=FALSE;
	SkillParam_Plunge*param=_rec->GetParam<SkillParam_Plunge>();
	CUnit3D *unit=_owner->GetUnit3D();
	if (unit)
	{
		_ges=Class_New2(CSkillGesture_Plunge);
		_ges->Create(_target.ObjID(),param->speed,_owner->GetLevel());
		_ges->AddRef();
		unit->SetGesture(_ges);
		bOk=TRUE;
	}

	if (!bOk)
		_SetState(SkillState_Fail);

}


void Skill_Plunge::_OnUpdate(AnimTick dt)
{
	if (_state==SkillState_Casting)
	{
		if (_ges)
		{
			if (!_ges->IsAlive())
				_Finish();
		}
	}
}


void Skill_Plunge::_Finish()
{
	SAFE_RELEASE(_ges);
	_SetState(SkillState_Finished);
	_owner->DeferDestroy();
}
