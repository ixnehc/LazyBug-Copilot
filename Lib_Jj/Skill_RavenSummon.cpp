
#include "stdh.h"


#include "Skill_RavenSummon.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelDecider.h"
#include "Buff_FlyBirth.h"

#include "LevelRtnus.h"

#include "LevelRecordUnit.h"

#include "Buff_Dead.h"

//根据起点和瞄准点,产生若干个散射的方向,为normalized过的
void ScatterDirs(LevelPos &src,LevelPos &target,LevelPos *dirs,DWORD c)
{
	LevelPos dir=target-src;
	if (c==1)
	{
		dir.safe_normalize();
		dirs[0]=dir;
		return;
	}

	float MaxRange=90.0f*(float)i_math::GRAD_PI2;
	float MinRange=10.0f*(float)i_math::GRAD_PI2;
	float MaxDist=12.0f;

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
		dirs[i].x=cosf(r);
		dirs[i].y=sinf(r);
	}
}


//////////////////////////////////////////////////////////////////////////
//Skill_RavenSummon

BIND_SKILLPARAM(Skill_RavenSummon,SkillParam_RavenSummon);


void Skill_RavenSummon::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);

	_tCasting=0;
	_UpdateSummon(0);
}

void Skill_RavenSummon::_UpdateSummon(AnimTick dt)
{
	_tCasting+=dt;

	if(_bSummon)
	{
		if (_tCasting>=_rec->CastTime)
		{
			_SetState(SkillState_Finished);
		}
	}
	else
	{
		if (_tCasting>=_rec->HitDelay)
		{
			_DoSummon();
			_bSummon=TRUE;
		}
	}

}

void Skill_RavenSummon::_OnUpdate(AnimTick dt)
{
	_UpdateSummon(dt);

}

void Skill_RavenSummon::_DoSummon()
{
	CLevelRecords *records=_owner->GetLevel()->GetRecords();
	CLevel *level=_owner->GetLevel();

	SkillParam_RavenSummon*param=_rec->GetParam<SkillParam_RavenSummon>();

	if (param)
	{
		CLevelDecider *decider=level->GetDecider();
		SkillGradeInfo_RavenSummon *info=param->GetGrdInfo(_grd);

		DWORD count=0;
		LevelPos dirs[32];
		if (TRUE)
		{
			count=info->count;
			if (count>ARRAY_SIZE(dirs))
				count=ARRAY_SIZE(dirs);

			ScatterDirs(_owner->GetFramePos(),_target.Aim(),dirs,count);
		}

		LevelPos3D posInit;
		posInit=_owner->GetFramePos3D();
		posInit.y+=_owner->GetCastHeight();
		for (int i=0;i<1;i++)
		{
			CLoUnit* lo=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));
			lo->PostCreate(LevelPlayerID_Wild,NULL,param->idUnit,_grd,NULL,EquipSetPick_None,posInit);
			level->AddToActives(lo);

			if (TRUE)
			{
				CLevelPlayer *player=level->GetPlayer((LevelPlayerID)_owner->GetPlayerID());
				if (player)
				{
					if (player->GetRtnus())
						player->GetRtnus()->Add_New(lo,FALSE);
				}
			}


			BuffArg_FlyBirth arg;
			arg.dir=dirs[i];
			arg.posInit=posInit;

			decider->MakeBuff(lo,param->idBirthBuff,ANIMTICK_FROM_SECOND(1.0f),&arg,LevelOpLinkID_Invalid);

			_UpdateRetinueCount(info->count,param->idUnit,param->idVanishBuff,lo);

			SAFE_RELEASE(lo);
		}

	}


}


//杀掉一定数量的Retinue,使该类型的Retinue的数量不超过最大值
void Skill_RavenSummon::_UpdateRetinueCount(DWORD nMax,RecordID idUnit,RecordID idVanishBuff,CLoUnit *loIgnore)
{
	CLevel *level=_owner->GetLevel();

	if (loIgnore)
		nMax--;
	CLoUnit*loRtnu[32];
	DWORD nRtnus=0;
	if (TRUE)
	{
		if (_owner->IsPlayer())
		{
			extern CLevelRtnus *LevelUtil_GetOwnerRtnus(CLevelObj *lo);
			CLevelRtnus *rtnus=LevelUtil_GetOwnerRtnus(_owner);
			if (rtnus)
			{
				DWORD c;
				CLevelRtnu **buf=rtnus->GetValidRetinues(c);
				for (int i=0;i<c;i++)
				{
					CLevelRtnu *rtnu=buf[i];
					CLoUnit *lo=rtnu->GetLo();
					if (lo==loIgnore)
						continue;
					if (lo)
					{
						LevelRecordUnit *rec=lo->GetRec();
						if (rec)
						{
							if (rec->GetID()==idUnit)
							{
								if (nRtnus<ARRAY_SIZE(loRtnu))
								{
									loRtnu[nRtnus]=lo;
									nRtnus++;
								}
							}
						}
					}
				}
			}
		}
	}

	while(nRtnus>nMax)
	{
		//找血最少的那一个
		CLoUnit *loUnitMin=NULL;
		DWORD vMin=65535;
		for (int i=0;i<nRtnus;i++)
		{
			CLoUnit *loUnit=loRtnu[i];
			LevelAttr_Base *attr=loUnit->GetAttr_Base();
			if (!attr)
				continue;

			if (attr->hp.GetCur_Int()<vMin)
			{
				loUnitMin=loUnit;
				vMin=attr->hp.GetCur_Int();
			}
		}

		if (loUnitMin)
		{
			CLevel *level=_owner->GetLevel();
			CLevelDecider *decider=level->GetDecider();

			BuffArg_Dead arg;
			decider->MakeBuff(loUnitMin,idVanishBuff,ANIMTICK_INFINITE,&arg,LevelOpLinkID_Invalid);
		}
		else
			break;

		nRtnus--;
	}



}
