
#include "stdh.h"

#include "LevelRecords.h"


#include "Skill_ThrowEo.h"

#include "LevelRecordSkill.h"
#include "LevelRecordEo.h"

#include "LoEffectObj.h"
#include "Level.h"

#include "Log/LogDump.h"

#include "LevelOSB.h"

//根据起点和瞄准点,产生若干个散射的方向
void ScatterThrowTarget(LevelPos &src,LevelPos &target,LevelPos *targets,DWORD c)
{
	if (c==1)
	{
		targets[0]=target;
		return;
	}

	float MaxRange=90.0f*(float)i_math::GRAD_PI2;
	float MinRange=10.0f*(float)i_math::GRAD_PI2;
	float MaxDist=12.0f;

	LevelPos dir=target-src;
	float dist=dir.getLength();
	if (dist>MaxDist)
		dist=MaxDist;
	float range=MaxRange-(MaxRange-MinRange)*dist/MaxDist;

	float rad=atan2f(dir.y,dir.x);

	float step=range*2.0f/(float)(c-1);

	float radFrom=rad-range;

	for (int i=0;i<c;i++)
	{
		float r=radFrom+step*(float)i;
		targets[i].x=src.x+cosf(r)*dist;
		targets[i].y=src.y+sinf(r)*dist;
	}

}


//////////////////////////////////////////////////////////////////////////
//CSkill_Throw

BIND_SKILLPARAM(Skill_ThrowEo,SkillParam_ThrowEo);


void Skill_ThrowEo::_ClearThrows()
{
	_nThrow=0;

}


void Skill_ThrowEo::_OnFinish()
{
	_ClearThrows();
}


void Skill_ThrowEo::_OnStart()
{
	SkillParam_ThrowEo*param=(SkillParam_ThrowEo*)_param;

	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);

	_casting.Init(this);

	extern BOOL LevelUtil_ConvertSkillTarget(CLevel *level,LevelSkillTarget &target,LevelSkillTarget::Type tp);
	LevelUtil_ConvertSkillTarget(_owner->GetLevel(),_target,LevelSkillTarget::Target_Pos);

	LevelPos src=_owner->GetFramePos();
	if (_target.bOrg)
		src=_target.org;

	LevelPos pos;
	assert(_target.tp==LevelSkillTarget::Target_Pos);

	pos=_target.Pos();

	_nThrow=param->Count;
	if (_nThrow>MAX_THROWS)
		_nThrow=MAX_THROWS;
	LevelPos targets[MAX_THROWS];
	ScatterThrowTarget(src,pos,targets,_nThrow);

	float dist=(pos-src).getLength();
	for (int i=0;i<_nThrow;i++)
	{
		_throws[i].pos=targets[i];
		if (param->Speed>0.0f)
			_throws[i].dur=ANIMTICK_FROM_SECOND(dist/param->Speed);
		else
			_throws[i].dur=0;
		_throws[i].bReached=FALSE;
	}
}

void Skill_ThrowEo::_UpdateThrows(AnimTick dt)
{
	_casting.UpdateToCasted(dt);
	if (_casting.NeedCasted())
		_SetState(SkillState_Casted);
	if (_casting.NeedFinished())
	{
		_ClearThrows();
		_SetState(SkillState_Finished);
	}

	if (_state!=SkillState_Finished)
	{
		if (_casting.IsFired())
		{
			CLevelDecider *decider=GetLevel()->GetDecider();
			AnimTick dt=LEVEL_SKILL_UPDATE_TICK;
			BOOL bAny=FALSE;

			LevelPos posMe=_owner->GetFramePos();
			for (int i=0;i<_nThrow;i++)
			{
				EoThrow *thr=&_throws[i];
				if (thr->bReached)
					continue;

				bAny=TRUE;

				thr->dur=ANIMTICK_SAFE_MINUS(thr->dur,dt);
				if (thr->dur<=0)
				{
					thr->bReached=TRUE;

					//在pos的位置释放一个Deal
					if (_rec->deal)
					{
						DealArg arg;
						arg.grd=_grd;
						arg.dir.setXZ(thr->pos-posMe);
						arg.link.id=GetLevel()->GenOpLinkID();
						arg.link.iSerial=i;
						extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
						LevelPos3D pos=LevelUtil_GetGroundHeight(GetLevel(),thr->pos.x,thr->pos.y,TRUE);
						_MakeDeals(pos,arg);
					}
				}
			}

			if ((!bAny)&&(_state==SkillState_Casted))
			{
				_SetState(SkillState_Finished);
				_ClearThrows();
			}
		}
	}
}


void Skill_ThrowEo::_OnUpdate(AnimTick dt)
{
	_UpdateThrows(dt);

}
