/********************************************************************
	created:	2012/10/24 
	author:		cxi
	
	purpose:	在三维空间内锁定某个Target的Act
*********************************************************************/
#include "stdh.h"

#include "Act_AttackOnce3D.h"
#include "LevelRecords.h"

#include "Random/Random.h"

#include "Level.h"

BIND_ACT_PARAM(Act_AttackOnce3D,ActParam_AttackOnce3D);

void Act_AttackOnce3D::_Update(AnimTick t)
{
	if (_result!=A_Pending)
		return;
	ActParam_AttackOnce3D *param=(ActParam_AttackOnce3D*)_param;

	if (_bAttacking)
	{
		CLevelSkillDriver *driver=_owner->GetSkillDriver();
		if (driver)
		{
			if (driver->IsWorking())
				return;
		}
		_result=A_Ok;
		return;
	}

	CLevelObj *lo=_owner->GetLevel()->GetIDs()->LoFromID(_idTarget);
	if (!lo)
	{
		_result=A_Fail;
		return;
	}

	LevelPos3D posSrc=_owner->GetFramePos3D();
	LevelPos3D posTarget=lo->GetFramePos3D();

	float distHor=(float)(posTarget-posSrc).getLengthXZ();
	float ht=posSrc.y-posTarget.y;
	float rangeMin,rangeMax,rangeVerMin,rangeVerMax;
	rangeMin=param->rangeMin;
	rangeMax=param->rangeMax;
	rangeVerMin=param->rangeVerMin;
	rangeVerMax=param->rangeVerMax;
	if (rangeMax<rangeMin)
		Swap(rangeMin,rangeMax);
	if (rangeVerMax<rangeVerMin)
		Swap(rangeVerMax,rangeVerMin);

	float rangeVerMinClamp,rangeVerMaxClamp;
	GameTileMap *gtm=_owner->GetLevel()->GetGtm();
	if (gtm)
	{
		rangeVerMinClamp=rangeVerMin;
		rangeVerMaxClamp=rangeVerMax;
		gtm->Clamp(posSrc.x,posSrc.z,rangeVerMinClamp);
		gtm->Clamp(posSrc.x,posSrc.z,rangeVerMaxClamp);
	}

	//加一点容忍度
	float endure=0.01f;
	if ((distHor>=rangeMin-0.01f)&&(distHor<=rangeMax+0.01f)&&
		(ht>=rangeVerMinClamp-0.01f)&&(ht<=rangeVerMaxClamp+0.01f))
	{
		CLevelSkillDriver *driver=_owner->GetSkillDriver();
		if (driver)
		{
			LevelSkillTarget target;
			target.SetObjID(_idTarget);
			if (FALSE==driver->Start(param->idSkill,target,FALSE,ClientSkillID_Invalid))
			{
				_result=A_Fail;
				return;
			}
		}
		else
		{
			_result=A_Fail;
			return;
		}

		_bAttacking=TRUE;
		return;
	}

	float distHorClamp=i_math::clamp_f(distHor,rangeMin,rangeMax);
	distHorClamp=distHor;
	if (distHorClamp<rangeMin)
		distHorClamp=rangeMin+10.0f;
	else
	{
		if (distHorClamp>rangeMax)
			distHorClamp=rangeMax-10.0f;
	}
	if (ht<rangeVerMin)
		ht=rangeMin+10.0f;
	else
	{
		if (ht>rangeVerMax)
			ht=rangeMax-10.0f;
	}

	LevelPos3D posToGo;
	posToGo.y=ht;
	posToGo.x=(posSrc.x-posTarget.x)/distHor*distHorClamp;
	posToGo.z=(posSrc.z-posTarget.z)/distHor*distHorClamp;
	posToGo+=posTarget;

	CLevelSkillDriver *driver=_owner->GetSkillDriver();
	if (driver)
	{
		LevelSkillTarget target;
		target.SetPos3D(posToGo);
		driver->StartMove(target);
	}
}


void Act_AttackOnce3D::Start(AnimTick t,LevelObjID idTarget)
{
	_tStart=t;
	_idTarget=idTarget;
	_Update(t);
}

void Act_AttackOnce3D::Update(AnimTick t)
{
	_Update(t);
}

