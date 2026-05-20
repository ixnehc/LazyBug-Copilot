
#include "stdh.h"


#include "Skill_ChainSpurt.h"

#include "LevelRecordSkill.h"

#include "LevelUtil.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelOSB.h"

#include "LevelDecider.h"


//////////////////////////////////////////////////////////////////////////
//CSkill_MeleeAttack

BIND_SKILLPARAM(Skill_ChainSpurt,SkillParam_ChainSpurt);


void Skill_ChainSpurt::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);
	_casting.Init(this);

	SkillParam_ChainSpurt *param=_rec->GetParam<SkillParam_ChainSpurt>();
	SkillGradeInfo_ChainSpurt *info=param->GetGrdInfo(_grd);
	_nToDamages=(BYTE)info->count;
	_dur=info->dur;

	_UpdateDamage(0);
}

void Skill_ChainSpurt::_UpdateDamage(AnimTick dt)
{
	_casting.UpdateToCasted(dt);

	if (_casting.NeedCasted())
	{
		_SetState(SkillState_Casted);
	}
	if (_casting.NeedFinished())
	{
		_SetState(SkillState_Finished);
	}


	if (_state!=SkillState_Finished)
	{
		if(_casting.IsFired())
		{
			DWORD nDamages;
			AnimTick tFire=_casting.GetFireTime();
			nDamages=(tFire+(_dur/_nToDamages/2))*_nToDamages/_dur;
			if (nDamages>_nToDamages)
				nDamages=_nToDamages;

			SkillParam_ChainSpurt *param=_rec->GetParam<SkillParam_ChainSpurt>();
			SkillGradeInfo_ChainSpurt *info=param->GetGrdInfo(_grd);

			DWORD nTargets=0;
			if (_arg)
				nTargets=_arg->sites.size();

			LevelPos posMe=_owner->GetFramePos();

			while (nDamages>_nDamages)
			{
				if(_nDamages<nTargets)
				{
					LevelPos pos=_arg->sites[_nDamages];
					LevelPos dir=pos-posMe;
					dir.safe_normalize();

					LevelUtilDetectParam paramDetect;
					paramDetect.loSrc=_owner;
					paramDetect.pos=pos;
					paramDetect.rangeMin=0.0f;
					paramDetect.rangeMax=info->radius;
					paramDetect.flags=&_rec->flagsDetect[0];
					paramDetect.nFlags=_rec->flagsDetect.size();
					paramDetect.requires=&_rec->requires[0];
					paramDetect.nRequires=_rec->requires.size();

					DWORD c;
					CLevelObj **targets=LevelUtil_Detect(paramDetect,NULL,c);

					for (int i=0;i<c;i++)
					{
						CLevelObj *loTarget=targets[i];
						extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
						if (LevelUtil_CheckSkillTarget(_rec,_owner,loTarget))
						{
// 							if(FALSE)
// 							{
// 								LevelStrike strike;
// 								_owner->GetLevel()->GetDecider()->MakeDead(LevelOSB(this),loTarget,strike,LevelOpLink());
// 							}
// 							else
							{
								DealArg arg;

								arg.dir.setXZ(dir);
								arg.link.id=GetLevel()->GenOpLinkID();
								arg.link.iSerial=(BYTE)_nDamages;
								arg.grd=_grd;

								_MakeDeals(loTarget,arg);
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
}


void Skill_ChainSpurt::_OnUpdate(AnimTick dt)
{
	_UpdateDamage(dt);

}
