
#include "stdh.h"


#include "Skill_ChainSummon.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "Random/Random.h"

//////////////////////////////////////////////////////////////////////////
//ChainSummonQueue

void ChainSummonQueue::Build(SkillParam_ChainSummon*param,LevelSkillArg *arg)
{
	CRandom rnd;
	rnd.srand(arg->seedRnd);
	//计算要召唤几个
	nToSummon=0;
	BYTE counts[16];//各个category的个数
	if (TRUE)
	{
		DWORD sz=param->cats.size();
		if (sz>ARRAY_SIZE(counts))
			sz=ARRAY_SIZE(counts);
		for (int i=0;i<sz;i++)
		{
			counts[i]=(BYTE)rnd.RandVaryUInt(param->cats[i].count,param->cats[i].vary);
			nToSummon+=counts[i];
		}

		DWORD nSites=0;
		if (arg)
			nSites=arg->sites.size();

		if (nToSummon>nSites)
			nToSummon=nSites;
	}

	int nSummon=0;

	while(nToSummon>0)
	{
		int idx=rnd.RandRangeInt<int>(0,nToSummon);
		for (int i=0;i<param->cats.size();i++)
		{
			if (idx>=counts[i])
			{
				idx-=counts[i];
				continue;
			}

			ChainSummonCategory *cat=&param->cats[i];
			LevelPos pos;
			assert(arg);
			assert(nSummon<arg->sites.size());
			sites[nSummon]=arg->sites[nSummon];
			cats[nSummon]=i;

			counts[i]--;
			nToSummon--;
			nSummon++;

			break;
		}
	}

	nToSummon=nSummon;
}



//////////////////////////////////////////////////////////////////////////
//CSkill_ChainSummon

BIND_SKILLPARAM(Skill_ChainSummon,SkillParam_ChainSummon);

void Skill_ChainSummon::_OnFinish()
{
}


void Skill_ChainSummon::_OnStart()
{
	SkillParam_ChainSummon*param=(SkillParam_ChainSummon*)_param;

	_FillRndSeed();

	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_queue.Build(param,_arg);


	_SetState(SkillState_Casting);
	_casting.Init(this);

}

void Skill_ChainSummon::_Update(AnimTick dt)
{
	_casting.UpdateToCasted(dt);
	if (_casting.NeedCasted())
		_SetState(SkillState_Casted);
	if (_casting.NeedCasted())
		_SetState(SkillState_Finished);

	if (_casting.NeedFire())
	{
		SkillParam_ChainSummon*param=(SkillParam_ChainSummon*)_param;
		_tSummoning=0;
		_tNextSummon=param->durSummon;
	}

	if (_state!=SkillState_Finished)
	{
		if (_casting.IsFired())
		{
			_UpdateSummon(dt);
			if (_nSummon>=_queue.nToSummon)
				_SetState(SkillState_Finished);
		}
	}
}


void Skill_ChainSummon::_OnUpdate(AnimTick dt)
{
	_Update(dt);
}

void Skill_ChainSummon::_UpdateSummon(AnimTick dt)
{
	_tSummoning+=dt;
	if (_tSummoning<_tNextSummon)
		return;

	if (_nSummon>=_queue.nToSummon)
		return;

	CLevel *level=GetLevel();
	CLevelDecider *decider=level->GetDecider();

	SkillParam_ChainSummon*param=(SkillParam_ChainSummon*)_param;
	_tNextSummon=_tSummoning+param->durSummon;

	//召唤一个
	if (TRUE)
	{
		ChainSummonCategory *cat=&param->cats[_queue.cats[_nSummon]];
		LevelPos pos=_queue.sites[_nSummon];

		//创建单位
		if (TRUE)
		{
			CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
			lo->PostCreate((LevelPlayerID)_owner->GetPlayerID(),NULL,cat->idUnit,_grd,NULL,EquipSetPick_None,pos);//使用技能的等级
			level->AddToActives(lo);

			if (cat->idBirth)
			{
				LevelOpLink link;
				link.iSerial=(BYTE)_nSummon;//_nSummon为序号
				decider->MakeBirth(LevelOSB(this),lo,cat->idBirth,link);
			}

			SAFE_RELEASE(lo);
		}

		_nSummon++;
	}



}
