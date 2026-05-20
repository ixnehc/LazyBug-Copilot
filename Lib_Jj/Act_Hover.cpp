/********************************************************************
	created:	2012/10/23 
	author:		cxi
	
	purpose:	AI Actions
*********************************************************************/
#include "stdh.h"

#include "Act_Hover.h"
#include "LevelRecords.h"

#include "Random/Random.h"


#include "Level.h"

BIND_ACT_PARAM(Act_Hover,ActParam_Hover);


void Act_Hover::Start(AnimTick t)
{
	_posCur=_owner->GetFramePos();

	_tStart=t;

	ActParam_Hover *param=(ActParam_Hover *)_param;
	_dur=CSysRandom::RandVary(param->dur,param->durVary);


	_GenNewPos(t);
}

void Act_Hover::_GenNewPos(AnimTick t)
{
	ActParam_Hover *param=(ActParam_Hover *)_param;

	LevelPos pos=_posCur;

	float rangeX=CSysRandom::RandRange(-param->range,param->range);
	float rangeY=CSysRandom::RandRange(-param->range,param->range);

	pos.x+=rangeX;
	pos.y+=rangeY;

	_posCur=pos;

	AnimTick dur=CSysRandom::RandVaryUInt(param->gap,param->gapVary);
	_tNextMove=t+dur;

}

void Act_Hover::Update(AnimTick t)
{
	if (t>_tStart+_dur)
		_bTimeUp=TRUE;
	if (_tNextMove<=t)
		_GenNewPos(t);

	ActParam_Hover *param=(ActParam_Hover *)_param;

	i_math::vector3df pos,vel,dir;
	float dist,speed=0.0f;
	if (TRUE)
	{
		LevelPos posT=_owner->GetFramePos();
		pos.set(posT.x,0.0f,posT.y);
		if (TRUE)
		{
			CUnit3D *unit=_owner->GetUnit3D();
			if (unit)
			{
				vel=unit->GetVel();
				speed=unit->GetSpeed();
			}
		}

		LevelPos dirToTargetT=_posCur-LevelPos(pos.x,pos.z);
		dir.set(dirToTargetT.x,0.0f,dirToTargetT.y);
		dist=(float)dir.getLength();
		if (dist>0.001f)
			dir/=dist;
		else
		{
			dir.set(1,0,0);
			dist=0.0f;
		}
	}

	BOOL bZeroVel=FALSE;
	if (vel.getLengthSQ()<0.0001f)
		bZeroVel=TRUE;
	i_math::vector3df eulerVel,eulerDir;
	if (!bZeroVel)
	{
		eulerVel=vel;
		eulerVel.toEuler();
		eulerVel.y=eulerVel.z=0.0f;
	}

	eulerDir=dir;
	eulerDir.toEuler();
	eulerDir.y=eulerDir.z=0.0f;

	float rangeHover=param->rangeHover;
	float wtBack;//返回到target的倾向,1表示全力回到target
	wtBack=i_math::clamp_f((dist)/(rangeHover),0.0f,1.0f);

	float twistMin=i_math::clamp_f(param->twist-param->twistVary,0.0f,1.0f);
	float twistMax=i_math::clamp_f(param->twist+param->twistVary,0.0f,1.0f);

	float twist=CSysRandom::RandRange(twistMin,twistMax)*i_math::Pi*wtBack;

// 	//更新偏离值
// 	float twistActual;
// 	if (TRUE)
// 	{
// 		float twistrange=1.2f*i_math::Pi;
// 
// 		int n=(int)(_twist/twistrange);
// 		if (n%2==0)
// 			twistActual=_twist-twistrange*(float)n;
// 		else
// 			twistActual=twistrange*(float)(n+1)-_twist;
// 
// 		twistActual-=twistrange/2.0f;
// 	}
// 	twist+=twistActual;
// 	if ()


	if (bZeroVel)
		eulerVel=eulerDir;
	else
		rotate_limited(eulerVel.x,eulerDir.x,twist);


	i_math::quatf q;
	q.fromEuler(eulerVel);

	i_math::vector3df velToTarget(0,0,1);
	velToTarget=q*velToTarget;

	pos+=velToTarget*20.0f;
	
	CLevelSkillDriver *driver=_owner->GetSkillDriver();
	if (driver)
	{
		LevelSkillTarget target;
		target.SetPos(LevelPos(pos.x,pos.z));
		driver->StartMove(target);
	}

// 
// 	i_math::vector3df velToTarget=dir*speed*wtBack+(1.0f-wtBack)*vel;
// 	velToTarget.normalize();
// 
// 
// 	//根据离target的远近,约束偏离值(离得越远,越不偏离)
// 	twistActual=twistActual*(1.0f-i_math::clamp_f(dist/rangeHover,0.0f,1.0f));
// 
// 	//根据twistActual来偏移velToTarget;
// 	i_math::quatf q;
// 	q.fromAngleAxis(twistActual,i_math::vector3df(0,1,0));
// 	velToTarget=q*velToTarget;
// 
// 	pos+=velToTarget*20.0f;
// 
// 	CLevelSkillDriver *driver=_owner->GetSkillDriver();
// 	if (driver)
// 	{
// 		LevelSkillTarget target;
// 		target.SetPos(LevelPos(pos.x,pos.z));
// 		driver->StartMove(target);
// 	}

}

