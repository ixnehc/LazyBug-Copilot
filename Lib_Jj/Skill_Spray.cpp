
#include "stdh.h"


#include "Skill_Spray.h"

#include "LevelRecordSkill.h"

#include "LoUnit.h"
#include "Level.h"

#include "LevelDecider.h"

#include "LevelObjMove.h"

#include "LevelOSB.h"



//////////////////////////////////////////////////////////////////////////
//CSkill_Spray
BIND_SKILLPARAM(Skill_Spray,SkillParam_Spray);


void Skill_Spray::_OnStart()
{
	_AddStartOp();

	GetLevel()->AddAffect(_owner);

	_SetState(SkillState_Casting);

}

BOOL Skill_Spray::Combine(LevelSkillTarget &target)
{
	_target=target;
	_AddCombineOp(target);
	GetLevel()->AddAffect(_owner);

	return TRUE;
}


void Skill_Spray::_OnUpdate(AnimTick dt)
{
	SkillParam_Spray*param=(SkillParam_Spray*)_param;

	if (GetState()==SkillState_Casting)
	{
		if (_tSpraying==ANIMTICK_INFINITE)
		{
			//累加Cast的时间
			extern void LevelUtil_AccumCastingTime(CLevelObj *lo,AnimTick dt,AnimTick &tCasting);
			LevelUtil_AccumCastingTime(_owner,dt,_tCasting);

			if (_tCasting>_rec->HitDelay)
			{
				_tSpraying=0;
			}
		}
		else
		{
			_tSpraying+=dt;

			//产生新的wave
			if (TRUE)
			{
				AnimTick durWaveSpawn;//多少时间产生一个Wave
				durWaveSpawn=(AnimTick)(((float)ANIMTICK_PER_SECOND)/param->speedWave);
				AnimTick tStart=_nWaves*durWaveSpawn;

				LevelPos dir;
				extern BOOL LevelUtil_CalcTargetDir(CLevelObj *loSrc,LevelSkillTarget &target,LevelPos&dir);
				LevelUtil_CalcTargetDir(_owner,_target,dir);

				float angleMin=atan2f(dir.y,dir.x);
				float spread=param->spread*i_math::GRAD_PI2;
				angleMin=i_math::wrap_radian(angleMin-spread/2.0f);
				float angleMax;
				angleMax=angleMin+spread;

				while (tStart+durWaveSpawn<_tSpraying)
				{
					//产生一个新的wave
					tStart+=durWaveSpawn;

					Wave wv;
					wv.angleMin=angleMin;
					wv.angleMax=angleMax;

					wv.distLast=0.0f;
					wv.tStart=tStart;

					_waves.push_back(wv);
					_nWaves++;
				}
			}

			//更新已有的wave
			if (TRUE)
			{
				LevelPos posMe=_owner->GetFramePos();

				DWORD c;
				extern CLevelObj **LevelUtil_DetectEnemies(CLevelObj *lo,LevelPos pos,float range,CLevelObj *toIgnore,LevelMoveMethodMask methods,DWORD &c);
				CLevelObj **buf=LevelUtil_DetectEnemies(_owner,posMe,param->range+1.0f,_owner,LevelMoveMethodMask_Ground,c);

				float angles[32];
				float dists2[32];
				CLevelObj *bufLo[32];
				if (c>32)
					c=32;


				//检查每个单位是否可以对它造成伤害,如果可以,记录角度及距离
				DWORD n=0;
				if (TRUE)
				{
					LevelPos pos;
					for (int i=0;i<c;i++)
					{
						CLevelObj *lo=buf[i];

						extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
						if (!LevelUtil_CheckSkillTarget(_rec,_owner,lo))
							continue;

						bufLo[n]=lo;

						pos=lo->GetFramePos();
						angles[n]=i_math::wrap_radian(atan2f(pos.y-posMe.y,pos.x-posMe.x));
						dists2[n]=pos.getDistanceSQFrom(posMe);

						n++;
					}
				}

				int nDiscard=0;

				for (int i=0;i<_waves.size();i++)
				{
					Wave *wv=&_waves[i];

					BOOL bNeedEnd=FALSE;
					float distCur=ANIMTICK_TO_SECOND(_tSpraying-wv->tStart)*param->speed;
					if (distCur>param->range)
					{
						distCur=param->range;
						bNeedEnd=TRUE;
					}

					for (int j=0;j<n;j++)
					{
						//判断是否在这个波的角度范围内
						float angle=angles[j];
						float dist2=dists2[j];
						if (TRUE)
						{
							if (angle<wv->angleMin)
								angle+=i_math::Pi*2.0f;
							if (angle>wv->angleMax)
								continue;//不在角度范围内
						}

						//判断是否在这个波的移动范围内
						if (TRUE)
						{
							if (dist2<(wv->distLast*wv->distLast))
								continue;
							if (dist2>(distCur*distCur))
								continue;
						}

						//结算伤害
						if (TRUE)
						{
							DealArg arg;
							arg.dir.set(cosf(angle),0.0f,sinf(angle));
							arg.link.id=GetLevel()->GenOpLinkID();
							arg.grd=_grd;

							_MakeDeals(bufLo[j],arg);
						}
					}

					wv->distLast=distCur;

					if (bNeedEnd)
						nDiscard=i+1;
				}

				if (nDiscard>0)
					_waves.erase(_waves.begin(),_waves.begin()+nDiscard);
	
			}
		}
	}

}


void Skill_Spray::_Finish()
{
	if (_state==SkillState_Finished)
		return;


	//通知Client这个技能Cast完了
	if (TRUE)
	{
		LevelOp_SkillCasted *op=NewOp<LevelOp_SkillCasted>(LevelOpLink());
		_owner->AddOp(op);
	}

	_SetState(SkillState_Finished);


}
