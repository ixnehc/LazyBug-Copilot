

#include "stdh.h"


#include "Skill_Summon.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "Random/Random.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_Throw

BIND_SKILLPARAM(Skill_Summon,SkillParam_Summon);

void Skill_Summon::_OnFinish()
{
}

std::vector<LevelPos> &Skill_Summon::_GetSites()
{
	if (_arg)
	{
		if (_arg->sites.size()>0)
			return _arg->sites;
	}
	return _sites;
}


void Skill_Summon::_OnStart()
{
	SkillParam_Summon*param=(SkillParam_Summon*)_param;


	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);
	_casting.Init(this);

}

void Skill_Summon::_BuildSummonSites()
{
	if (_sites.size()>0)
		return;//Already built

	SkillParam_Summon*param=(SkillParam_Summon*)_param;
	if (param->count<=0)
		return;

	LevelPos posSrc=_owner->GetFramePos();

	LevelPos posTarget;
	if (_target.tp!=LevelSkillTarget::Target_None)
	{
		extern BOOL LevelUtil_CalcTargetPos(CLevel *level,LevelSkillTarget &target,LevelPos&pos);
		LevelUtil_CalcTargetPos(GetLevel(),_target,posTarget);
	}
	else
	{
		float rad=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
		posTarget.x=posSrc.x+cosf(rad);
		posTarget.y=posSrc.y+sinf(rad);
	}

	_sites.resize(param->count);
	_dirs.resize(param->count);
	extern void LevelUtil_ScatterDirs(LevelPos &src,LevelPos &target,LevelPos *dirs,float degFov,DWORD c);
	LevelUtil_ScatterDirs(posSrc,posTarget,_dirs.data(),param->fov,_sites.size());

	assert((param->mode==SkillParam_Summon::Mode_AroundMe)||(param->mode==SkillParam_Summon::Mode_AroundTarget));

	if (param->mode==SkillParam_Summon::Mode_AroundMe)
	{
		for (int i=0;i<_dirs.size();i++)
			_sites[i]=posSrc+_dirs[i]*param->range;
	}
	else
	{
		for (int i=0;i<_dirs.size();i++)
			_sites[i]=posTarget+_dirs[i]*param->range;
	}

	//Clip到navmesh中
	if (TRUE)
	{
		LevelPos posStart;
		if (param->mode==SkillParam_Summon::Mode_AroundMe)
			posStart=posSrc;
		else
			posStart=posTarget;

		LevelPos posHit;
		CLevel *level=_owner->GetLevel();
		CUnitMgrNavMesh *unitmgr=level->GetUnitMgr();
		for (int i=0;i<_dirs.size();i++)
		{
			if (unitmgr->StaticRayCast(UnitFindPath_Walkable,posStart,_sites[i],posHit))
				_sites[i]=posHit;
		}
	}


}


void Skill_Summon::_Update(AnimTick dt)
{
	extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
	SkillParam_Summon*param=(SkillParam_Summon*)_param;
	_casting.UpdateToFinished(dt);

	float durAfterFire=0.0f;
	if (_casting.GetCastTime()>_casting.GetFireTime())
		durAfterFire=ANIMTICK_TO_SECOND(_casting.GetCastTime()-_casting.GetFireTime());
	float durTotal=0.0f;


	if (_casting.NeedFire())
	{
		if (_GetSites().size()<=0)
		{
			BOOL bNeedBuildSites=TRUE;
			if (_arg)
			{
				if (_arg->sites.size()>0)
					bNeedBuildSites=FALSE;
			}

			if(bNeedBuildSites)
				_BuildSummonSites();

			if (param->dur>0.0f)
			{
				std::vector<LevelPos> &sites=_GetSites();
				if (sites.size()>0)
					CSysRandom::GenRandomIndices(_indicesRandom,_arg->sites.size());
			}
		}

		std::vector<LevelPos> &sites=_GetSites();
		if (sites.size()>0)
			durTotal=param->dur*(float)sites.size();
		std::vector<LevelPos> &dirs=_GetDirs();


		DealArg arg;
		arg.dir.set(0,0,0);
		arg.link.id=GetLevel()->GenOpLinkID();
		arg.grd=_grd;

		if (durTotal<=0.0f)
		{
			for (int i=0;i<sites.size();i++)
			{
				if (i<dirs.size())
					arg.dir.setXZ(dirs[i]);

				LevelPos3D posSite=LevelUtil_GetGroundHeight(GetLevel(),sites[i].x,sites[i].y,TRUE);
				_MakeDeals(posSite,arg);
			}
		}
		else
		{
			DWORD nToSummon=0;
			if (sites.size()>0)
			{
				nToSummon=1+(int)(durAfterFire*(float)(sites.size()-1)/durTotal);
				for (int i=_nSummoned;i<nToSummon;i++)
				{
					if (i<dirs.size())
						arg.dir.setXZ(dirs[i]);
					LevelPos3D posSite=LevelUtil_GetGroundHeight(GetLevel(),sites[_indicesRandom[i]].x,sites[_indicesRandom[i]].y,TRUE);
					_MakeDeals(posSite,arg);
				}
				_nSummoned=nToSummon;
			}
		}
	}

	if (_casting.NeedFinished()&&(durAfterFire>=durTotal))
		_SetState(SkillState_Finished);
}


void Skill_Summon::_OnUpdate(AnimTick dt)
{
	_Update(dt);
}
