/********************************************************************
	created:	2012/10/24 
	author:		cxi
	
	purpose:	在三维空间内锁定某个Target的Act
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"


#include "BgnFlyThrust.h"
#include "LevelRecords.h"

#include "Random/Random.h"

#include "Level.h" 


BIND_BGN_CLASS(CBgn_FlyThrust,CBgp_FlyThrust);

void CBgn_FlyThrust::_FireFail(BGNOutputs &outputs)
{
	_OutputFail(outputs,2,"失败");
}

//返回是否成功了
void CBgn_FlyThrust::_Update(AnimTick t,BGNOutputs &outputs)
{
	CBgp_FlyThrust*pad=_GetPad<CBgp_FlyThrust>();

	if (_bAttacking)
	{
		CLevelSkillDriver *driver=_owner->GetSkillDriver();
		if (driver)
		{
			if (driver->IsWorking())
				return;
		}
		_OutputOk(outputs,1,"结束");
		return;
	}

	CLevelObj *lo=_owner->GetLevel()->GetIDs()->LoFromID(_idTarget);
	if (!lo)
	{
		_FireFail(outputs);
		return;
	}

	LevelPos3D posSrc=_owner->GetFramePos3D();
	LevelPos3D posTarget=lo->GetFramePos3D();

	float distHor=(float)(posTarget-posSrc).getLengthXZ();
	float ht=posSrc.y-posTarget.y;
	float rangeMin,rangeMax,rangeVerMin,rangeVerMax;
	rangeMin=pad->_rangeMin;
	rangeMax=pad->_rangeMax;
	rangeVerMin=pad->_rangeVerMin;
	rangeVerMax=pad->_rangeVerMax;
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
			if (FALSE==driver->Start(LevelSkillType(pad->_idSkill),target,FALSE,ClientSkillID_Invalid,LevelSkillGrade_Invalid,NULL))
			{
				_FireFail(outputs);
				return;
			}
		}
		else
		{
			_FireFail(outputs);
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
		driver->StartFollow(target);
	}
}


void CBgn_FlyThrust::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_FlyThrust*pad=_GetPad<CBgp_FlyThrust>();
	_owner=_GetLo();
	_tStart=_GetT();
	_idTarget=LevelObjID_Invalid;
	if (pad->_nmVar!=StringID_Invalid)
		_GetID(pad->_nmVar,BehaviorMemType_ObjID,_idTarget);
	if (_idTarget==LevelObjID_Invalid)
	{
		_FireFail(outputs);
		return;
	}
	_Update(_tStart,outputs);
}

void CBgn_FlyThrust::Update(BGNOutputs &outputs)
{
	_Update(_GetT(),outputs);
}

