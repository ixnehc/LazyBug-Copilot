
#include "stdh.h"


#include "Skill_Zeal.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_MeleeAttack

BIND_SKILLPARAM(Skill_Zeal,SkillParam_Zeal);


void Skill_Zeal::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);


	_SetState(SkillState_Casting);

	SkillParam_Zeal *param=_rec->GetParam<SkillParam_Zeal>();
	SkillGradeInfo_Zeal *info=param->GetGrdInfo(_grd);
	_nToDamages=(BYTE)info->count;
	_dur=info->dur;

	_tCasting=0;
	_UpdateDamage(0);
}

LevelPos3D Skill_Zeal::_GetTarget(CLevelObj *&loRet,int iCast)
{
	loRet=NULL;

	LevelObjID idLo=LevelObjID_Invalid;
	if (iCast<_arg->objs.size())
		idLo=_arg->objs[iCast];
	else
	{
		if (_arg->objs.size()>0)
			idLo=_arg->objs[_arg->objs.size()-1];
	}

	if (idLo!=LevelObjID_Invalid)
	{
		extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
		CLevelObj *lo=LevelUtil_GetAliveLo(GetLevel(),idLo);
		if (lo)
		{
			LevelPos3D pos=lo->GetFramePos3D();
			_targetsitesCache[idLo]=pos;

			loRet=lo;
			return pos;
		}

		std::unordered_map<LevelObjID,LevelPos3D>::iterator it=_targetsitesCache.find(idLo);
		if (it!=_targetsitesCache.end())
			return (*it).second;
	}

	assert(_arg->sites.size()>0);
	if (iCast>=_arg->sites.size())
		iCast=_arg->sites.size()-1;
	LevelPos pos=_arg->sites[iCast];

	extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
	LevelPos3D pos3D=LevelUtil_GetGroundHeight(_owner->GetLevel(),pos.x,pos.y,TRUE);
	return pos3D;
}

#define MAX_BULLET (32)

void Skill_Zeal::_UpdateDamage(AnimTick dt)
{
	if (TRUE)
	{
		assert(_arg&&_arg->sites.size()>0);
		if (!(_arg&&_arg->sites.size()>0))
		{
			_SetState(SkillState_Finished);
			return;
		}
	}

	if (_nDamages>=_nToDamages)
		return;

	if (_state==SkillState_Casting)
	{
		if (_bBroken)
		{
			_SetState(SkillState_Finished);
			return;
		}

		//累加Cast的时间
		extern void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting);
		LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

		DWORD nDamages;
		nDamages=(_tCasting+(_dur/_nToDamages/2))*_nToDamages/_dur;
		if (nDamages>_nToDamages)
			nDamages=_nToDamages;

		SkillParam_Zeal *param=_rec->GetParam<SkillParam_Zeal>();

		DWORD nTargets=_arg->sites.size();

		CLevelDecider *decider=GetLevel()->GetDecider();
		while (nDamages>_nDamages)
		{
			if(_nDamages<nTargets)
			{
				CLevelObj *lo=NULL;
				LevelPos3D pos3D=_GetTarget(lo,_nDamages);

				if (lo)
				{
					if(CLevelDecider::CheckInRange(_owner,lo,_rec->CastRange+_rec->CastRangeTolerance+1.0f))//+1.0增加一点冗余量
					{
						if (TRUE)
						{
							DealArg arg;
							arg.dir=(lo->GetFramePos3D()-_owner->GetFramePos3D()).normalize();
							arg.link.id=GetLevel()->GenOpLinkID();
							arg.grd=_grd;
							arg.link.iSerial=(BYTE)_nDamages;

							if (param->bMelee)
							{
								extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
								if (LevelUtil_CheckSkillTarget(_rec,_owner,lo))
									_MakeDeals(lo,arg);
							}
							else
							{
								LevelPos3D posCast=_owner->GetFramePos3D();
								posCast.y+=param->htCast;

								LevelPos3D posAim=lo->GetFramePos3D();

								if (param->modeAim==SkillParam_Zeal::Aim_ToAimPos)
								{
									posAim.y+=lo->GetAimHeight();
									arg.dir=(posAim-posCast).normalize();
								}
								else
									posAim.y+=param->htCast;

								if (param->countCast<=1)
									_MakeDeals(posCast,arg);
								else
								{
									DWORD c=param->countCast;
									if (c>MAX_BULLET)
										c=MAX_BULLET;
									extern void ScatterBulletDirs(LevelPos3D &src,LevelPos3D &aim,LevelPos3D *dirs,DWORD c);
									LevelPos3D dirs[MAX_BULLET];
									ScatterBulletDirs(posCast,posAim,dirs,c);

									for (int i=0;i<c;i++)
									{
										arg.dir=dirs[i];
										_MakeDeals(posCast,arg);
									}
								}
							}
						}
					}
				}
				else
				{
					DealArg arg;
					arg.dir=(pos3D-_owner->GetFramePos3D()).normalize();
					arg.link.id=GetLevel()->GenOpLinkID();
					arg.grd=_grd;
					arg.link.iSerial=(BYTE)_nDamages;

					LevelPos3D posCast=_owner->GetFramePos3D();
					posCast.y+=param->htCast;

					if (param->countCast<=1)
						_MakeDeals(posCast,arg);
					else
					{
						DWORD c=param->countCast;
						if (c>MAX_BULLET)
							c=MAX_BULLET;
						extern void ScatterBulletDirs(LevelPos3D &src,LevelPos3D &aim,LevelPos3D *dirs,DWORD c);
						LevelPos3D dirs[MAX_BULLET];
						ScatterBulletDirs(_owner->GetFramePos3D(),pos3D,dirs,c);

						for (int i=0;i<c;i++)
						{
							arg.dir=dirs[i];
							_MakeDeals(posCast,arg);
						}
					}
				}
			}
			_nDamages++;
		}

		if (_nDamages>=_nToDamages)
			_SetState(SkillState_Finished);
	}
}


void Skill_Zeal::_OnUpdate(AnimTick dt)
{
	_UpdateDamage(dt);

}
