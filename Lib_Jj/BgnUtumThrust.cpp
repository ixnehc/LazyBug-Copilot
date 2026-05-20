/********************************************************************
	created:	2016/04/18 
	author:		cxi
	
	purpose:	Utum的冲刺行为
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"


#include "BgnUtumThrust.h"
#include "LevelRecords.h"

#include "Skill_UtumSummon.h"
#include "Buff_UtumBirth.h"

#include "Random/Random.h"

#include "LevelUtil.h"

#include "Level.h"


BIND_BGN_CLASS(CBgn_UtumThrust,CBgp_UtumThrust);

void CBgn_UtumThrust::_FireFail(BGNOutputs &outputs)
{
	_OutputFail(outputs,2,"失败");
}

SkillParam_UtumSummon *CBgn_UtumThrust::_GetUtumSummonParam()
{
	CLevelPlayer *player=_GetOwnerPlayer();
	if (player)
	{
		CLevelAbility *ability=player->GetAbilities().GetActiveAbility(LevelAbilityType_UtumTide);
		if (ability)
		{
			CLevel *level=_GetLevel();
			if (level)
			{
				if (level->GetRecords())
				{
					LevelRecordSkill *rec=ability->GetSkillRecordRT(LevelAbilityAction_AttackA);
					if (rec)
						return rec->GetParam<SkillParam_UtumSummon>();
				}
			}
		}
	}

	return NULL;
}

Buff_UtumBirth *CBgn_UtumThrust::_GetBirthBuff()
{
	CLevelObj *lo=_GetLo();
	if (lo)
	{
		extern CLevelBuff *LevelUtil_FindBuff(CLevelObj *lo,CClass *clssBuff);
		CLevelBuff *buff=LevelUtil_FindBuff(lo,Class_Ptr2(Buff_UtumBirth));
		if (buff)
			return (Buff_UtumBirth*)buff;
	}
	return NULL;
}

CLevelObj *CBgn_UtumThrust::_Detect(BOOL bReturn)
{
	CBgp_UtumThrust*pad=_GetPad<CBgp_UtumThrust>();
	CLevel *level=_GetLevel();

	LevelPos pos=_owner->GetFramePos();
	LevelPos3D pos3D=_owner->GetFramePos3D();

	float range=i_math::clampup_f(_paramSummon->rangeAttack-pos.getDistanceFrom(_posOrg),0.0f);
	float dFov;
	if (TRUE)
	{
		float r=1.0f-range/_paramSummon->rangeAttack;
		float fov=i_math::lerp(_paramSummon->fovMin,_paramSummon->fovMax,r);
		dFov=cosf(fov/2.0f*i_math::GRAD_PI2);
	}
	range+=4.0f;
	float dDescend=tan(_paramSummon->descendMax*i_math::GRAD_PI2);

	LevelPos dir;
	if (!bReturn)
		dir.setEuler(_euler);
	else
		dir.setEuler(_euler+i_math::Pi);

	LevelPos posEnd=pos+dir*range;

	i_math::rectf rc(pos.x,pos.y,posEnd.x,posEnd.y);
	rc.repair();

	LevelUtilDetectParam param;
	param.loSrc=_owner;
	param.pos=(pos+posEnd)*0.5f;//中点
	param.rangeMin=0.0f;
	param.rangeMax=range/2.0f;
	param.flags=&_recAttack->flagsDetect[0];
	param.nFlags=_recAttack->flagsDetect.size();
	param.requires=&_recAttack->requires[0];
	param.nRequires=_recAttack->requires.size();
	// 		param.weights.CopyFrom(recSkill->weightsDetect);

	DWORD c;
	extern CLevelObj **LevelUtil_Detect(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,DWORD &c);
	CLevelObj **buf=LevelUtil_Detect(param,NULL,c);

	CLevelObj *loDetect=NULL;
	float distMin=1000000000.0f;
	for (int i=0;i<c;i++)
	{
		CLevelObj *loTarget=buf[i];
		if (loTarget)
		{
			LevelPos posTarget=loTarget->GetFramePos();
			LevelPos dirTarget=posTarget-pos;

			float dist2=dirTarget.getLengthSQ();
			if (dist2>range*range)
				continue;
			if (dist2<0.01f)
				continue;

			float dist=sqrtf(dist2);
			dirTarget/=dist;
			float d=dirTarget.dotProduct(dir);
// 			if (d<dFov)
// 				continue;//视野范围外
// 
// 			if (dist>distMin)
// 				continue;

			LevelPos3D pos3DTarget=loTarget->GetFramePos3D();
			pos3DTarget.y+=loTarget->GetHeight();

			float distDescend=pos3D.y-pos3DTarget.y;
			if (distDescend/dist>dDescend)
				continue;//下降幅度太大

			distMin=dist;
			loDetect=loTarget;
		}
	}

	return loDetect;

}

BOOL CBgn_UtumThrust::_DetectPos(LevelPos &pos)
{
	const float distDetect=10.0f;

	float dist=6.0f;

	if (TRUE)
	{
		LevelPos dir;
		dir.setEuler(_euler);
		pos=_owner->GetFramePos();
		pos+=dir*dist;
		return TRUE;
	}


	extern BOOL LevelUtil_DetectPos(CLevelObj *lo,float &euler,float &dist,float fov,int nSideSteps,LevelPos &pos);

	if (LevelUtil_DetectPos(_owner,_euler,dist,30.0f*i_math::GRAD_PI2,2,pos))
		return TRUE;
	if (dist>3.0f)
		return TRUE;
	return FALSE;
}

BOOL CBgn_UtumThrust::_MakeReturn(CLevelSkillDriver *driver,AnimTick t,BOOL bDirectlyReturn)
{
	CLevelObj *loPlayer=LevelUtil_GetOwnerLo(_owner);
	assert(loPlayer);

	_verCast=driver->GetCastVer();
	_tStart=t;
	LevelSkillTarget target;
	target.SetObjID(loPlayer->GetID());
	if (FALSE==driver->Start(LevelSkillType(bDirectlyReturn?_paramSummon->idDirectlyReturnSkill:_paramSummon->idReturnSkill),target,FALSE,ClientSkillID_Invalid,LevelSkillGrade_Invalid,NULL))
		return FALSE;
	_stage=bDirectlyReturn?DirectlyReturn:Return;

	return TRUE;
}

BOOL CBgn_UtumThrust::_MakeAttack(CLevelSkillDriver *driver,CLevelObj *loTarget)
{
	LevelSkillTarget target;
	target.SetObjID(loTarget->GetID());
	_verCast=driver->GetCastVer();
	if (FALSE==driver->Start(LevelSkillType(_paramSummon->idAttackSkill),target,FALSE,ClientSkillID_Invalid,LevelSkillGrade_Invalid,NULL))
		return FALSE;
	_stage=Attack;
	_bAttacked=TRUE;
	return TRUE;
}




//返回是否成功了
void CBgn_UtumThrust::_Update(AnimTick t,BGNOutputs &outputs)
{
	CBgp_UtumThrust*pad=_GetPad<CBgp_UtumThrust>();

	CLevel *level=_GetLevel();

	CLevelSkillDriver *driver=_owner->GetSkillDriver();
	if (!driver)
	{
		assert(FALSE);
		_FireFail(outputs);
		return;
	}

	if (_stage==None)
	{
		CLevelBuff *buff=_GetBirthBuff();
		if (buff)
		{
			_stage=Birth;
			return;
		}
	}

	if (_stage==Birth)
	{
		CLevelBuff *buff=_GetBirthBuff();
		if (buff)
			return;

		//Birth结束了
		_posOrg=_owner->GetFramePos();
		_euler=pad->eulerThrust;
		_stage=Thrust;
		_tStart=t;
		_tNextThrustUpdate=t;
	}

	if (_stage==Thrust)
	{
		CLevelObj *loDetect=_Detect(FALSE);
		if (loDetect)
		{
			if (!_MakeAttack(driver,loDetect))
			{
				assert(FALSE);
				_FireFail(outputs);
				return;
			}
		}
		else
		{
			if (t>=_tStart+_paramSummon->durThrust)
			{
				LevelPos pos=_owner->GetFramePos();
				CLevelObj *loPlayer=LevelUtil_GetOwnerLo(_owner);
				assert(loPlayer);
				float dist2;
				if (loPlayer)
					dist2=pos.getDistanceSQFrom(loPlayer->GetFramePos());
				else
					dist2=pos.getDistanceSQFrom(_posOrg);

				if (dist2>_paramSummon->distReturn*_paramSummon->distReturn)
				{
					if (!_MakeReturn(driver,t,FALSE))
					{
						assert(FALSE);
						_FireFail(outputs);
						return;
					}
					return;
				}
			}

			if (_stage==Thrust)
			{
				if (t>=_tNextThrustUpdate)
				{
					LevelPos pos;
					if (_DetectPos(pos))
					{
						_tNextThrustUpdate=t+ANIMTICK_FROM_SECOND(0.1f);

						LevelSkillTarget target;
						target.SetPos(pos);
						driver->StartFollow(target);
					}
					else
					{
						LevelSkillTarget target;
						target.SetObjID(_owner->GetID());
						driver->StartFollow(target);
					}
				}
			}
		}
	}

	if (_stage==Return)
	{
		if (!_bAttacked)
		{
			if (driver->GetCastVer()==_verCast)
			{//尚未Cast
				CLevelObj *loDetect=_Detect(TRUE);
				if (loDetect)
				{
					if (!_MakeAttack(driver,loDetect))
					{
						assert(FALSE);
						_FireFail(outputs);
						return;
					}
				}
			}
		}

		if (_stage==Return)
		{
			if (driver->GetCastVer()==_verCast)
			{//尚未释放技能(还在跟踪)
				if (t>_tStart+_durReturnFollow)
				{
					//直接返回
					if (!_MakeReturn(driver,t,TRUE))
					{
						assert(FALSE);
						_FireFail(outputs);
						return;
					}
				}
			}
		}
	}

	if (_stage==PostAttack)
	{
		if (t>=_tStart+_paramSummon->durPostAttack)
		{
			if (!_MakeReturn(driver,t,FALSE))
			{
				assert(FALSE);
				_FireFail(outputs);
				return;
			}
		}
	}

	if (_stage==Attack)
	{
		if (!driver->IsWorking())
		{
			if (driver->GetCastVer()!=_verCast)
			{
				//已经释放了,PostAttack
				_stage=PostAttack;
				_tStart=t;
			}
			else
			{
				//尚未施法,继续Thrust
				_stage=Thrust;
			}
		}
	}

}


void CBgn_UtumThrust::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_UtumThrust*pad=_GetPad<CBgp_UtumThrust>();
	_owner=_GetLo();

	_paramSummon=_GetUtumSummonParam();
	if (_paramSummon)
		_recAttack=LevelUtil_GetSkillRecord(_owner,_paramSummon->idAttackSkill);

	_durReturnFollow=CSysRandom::RandRangeInt(_paramSummon->durMinReturnFollow,_paramSummon->durMaxReturnFollow);

	assert(_paramSummon);
	assert(_recAttack);

	if ((!_paramSummon)||(!_recAttack))  
	{
		_OutputFail(outputs,2,"失败");
		return;
	}

	_Update(_GetT(),outputs);
}

void CBgn_UtumThrust::Update(BGNOutputs &outputs)
{
	_Update(_GetT(),outputs);
}

