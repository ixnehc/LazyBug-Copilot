
#include "stdh.h"


#include "Skill_Dive.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"

#include "LevelObjMove.h"

////////////////////////////////////////////////////////////////////////
//CSkillGesture_Dive
void CSkillGesture_Dive::Create(i_math::vector3df &posCur,i_math::vector3df &posTarget,float dur,float lift,float slide,GameTileMap *gtm)
{
	_UpdateFormula(posCur,posTarget,lift,slide);
	_t=0.0f;
	_dur=dur;
	_gtm=gtm;
}

void CSkillGesture_Dive::_UpdateFormula(i_math::vector3df &posSrc,i_math::vector3df posTarget,float lift,float slide)
{
	_pos[0]=posSrc;
	_pos[1]=posTarget;
	i_math::vector3df dir=posTarget-posSrc;
	dir.setLength(slide);
	_pos[2]=posTarget+dir;
	_pos[2].y=posTarget.y+lift;

	float dist=(float)posTarget.getDistanceFrom(posSrc);

	i_math::vector3df posTargetDeep;
	posTargetDeep=posTarget;
	posTargetDeep.y+=(posTarget.y-posSrc.y)*2.0f;

	_vel[0]=posTargetDeep-posSrc;
	_vel[0].setLength(dist/3.0f);
	_vel[1]=posTarget-posSrc;
	_vel[1].y=0.0f;
	_vel[1].setLength(dist/3.0f);
	_vel[2]=_pos[2]-posTargetDeep;
	_vel[2].setLength(dist/3.0f);
}


void CSkillGesture_Dive::Update(CUnit3D *unit,float dt)
{
	_t+=dt;
	i_math::vector3df pos;
	extern i_math::vector3df GetPositionOnCubic(const i_math::vector3df &startPos, const i_math::vector3df &startVel, const i_math::vector3df &endPos, const i_math::vector3df &endVel, float time);
	if (_t<_dur/2.0f)
	{
		float time=_t/(_dur/2.0f);
		pos=GetPositionOnCubic(_pos[0],_vel[0],_pos[1],_vel[1],time);
	}
	else
	{
		_bPassBy=TRUE;
		float time=(_t-_dur/2.0f)/(_dur/2.0f);
		pos=GetPositionOnCubic(_pos[1],_vel[1],_pos[2],_vel[2],time);
	}

	float ht=_gtm->GetHeight(pos.x,pos.z);
	if (pos.y<ht)
		pos.y=ht;

	float blend=0.4f;
	i_math::vector3df vel=(pos-unit->_pos)/dt;
	vel=unit->_vel*(1.0f-blend)+vel*blend;
	unit->_pos+=vel*dt;
	unit->_vel=vel;

	unit->_ClampGround(unit->_pos);

	if (_t>=_dur)
		_bFinished=TRUE;

}



//////////////////////////////////////////////////////////////////////////
//CSkill_Charge
BIND_SKILLPARAM(Skill_Dive,SkillParam_Dive);


void Skill_Dive::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);
	_SetState(SkillState_Casting);


	BOOL bOk=FALSE;
	LevelPos3D posSrc=_owner->GetFramePos3D();
	CLevelObj *lo=_owner->GetLevel()->GetIDs()->LoFromID(_target.ObjID());
	if (lo)
	{
		LevelPos3D posTarget=lo->GetFramePos3D();
		posTarget.y+=lo->GetAimHeight();
//		posTarget.y+=_owner->GetCastHeight();

		SkillParam_Dive*param=_rec->GetParam<SkillParam_Dive>();
		if (param)
		{
			CUnit3D *unit=_owner->GetUnit3D();
			if (unit)
			{
				_ges=Class_New2(CSkillGesture_Dive);
				_ges->Create(posSrc,posTarget,ANIMTICK_TO_SECOND(_rec->CastTime),param->lift,param->slide,_owner->GetLevel()->GetGtm());
				_ges->AddRef();
				unit->SetGesture(_ges);
				bOk=TRUE;
			}
		}
	}

	if (!bOk)
		_SetState(SkillState_Fail);

}


void Skill_Dive::_OnUpdate(AnimTick dt)
{
	if (_state==SkillState_Casting)
	{
		if (_ges)
		{
			if (_ges->IsPassBy())
			{
				//在此结算伤害
				LevelObjID id=_target.ObjID();
				CLevelObj *lo=_owner->GetLevel()->GetIDs()->LoFromID(id);
				if (lo)
				{
					LevelPos3D posTarget=lo->GetFramePos3D();
					LevelPos3D posPassBy=_ges->GetPassByPos();

					SkillParam_Dive*param=_rec->GetParam<SkillParam_Dive>();
					if (param)
					{
						if ((posTarget-posPassBy).getLengthSQ_XZ()<param->radiusDamage*param->radiusDamage)
						{
							DealArg arg;
							i_math::vector3df vel=_ges->GetPassByVel();
							arg.dir=vel;
							arg.link.id=GetLevel()->GenOpLinkID();

							_MakeDeals(lo,arg);
						}
					}

				}

				_Finish();
			}
		}
	}
}


void Skill_Dive::_Finish()
{
	SAFE_RELEASE(_ges);
	_SetState(SkillState_Finished);
}
