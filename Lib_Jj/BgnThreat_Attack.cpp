/********************************************************************
	created:	2016/09/14 
	author:		cxi
	
	purpose:	 攻击Threat
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnThreat_Attack.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"
#include "LevelTroops.h"

#include "LevelUtil.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_Attack
BIND_BGN_CLASS(CBgnThreat_Attack,CBgpThreat_Attack);

void CBgnThreat_Attack::Destroy()
{
	SAFE_RELEASE(_target);
}


extern BOOL LevelUtil_Flee(CLevelObj *lo,CLevelObj *loEscapeFrom,float distKeep,int &signAvoid,CLevelObj *loCenter,float radius);
void CBgnThreat_Attack::_Start(CLevelObj *target,BOOL bEscapce)
{
	CBgpThreat_Attack*pad=_GetPad<CBgpThreat_Attack>();
	AnimTick t=_GetT();

	CLevelObj *lo=_GetLo();
	LevelBehaviorContext *ctx=_GetCtx();
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);

	if (target==_target)
	{
		if (bEscapce)
		{
			if (_state==Escape)
				return;
		}
		else
		{
			if (_state==Attack)
				return;
		}
	}

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (driver)
	{
		if (bEscapce)
		{
			//目标离自己太近,我们先要离远一点
			int signAvoid=0;
			LevelUtil_Flee(lo,target,pad->distKeep*1.5f,signAvoid,NULL,0);

			float spd=LevelUtil_GetSpeed(lo);
			if (spd<0.1f)
				spd=0.1f;

			_state=Escape;
			_tStateStart=t;
			_durEscape=ANIMTICK_FROM_SECOND(pad->distKeep/spd);

			SAFE_REPLACE(_target,target);

			return;
		}
		else
		{
			//立即开始攻击
			LevelSkillTarget tgt;
			tgt.SetObjID(target->GetID());
			_verCast=driver->GetCastVer();
			if (recSkill)
				driver->Start(LevelSkillType(pad->idSkill),tgt,FALSE,ClientSkillID_Invalid,pad->grd,NULL);

			_state=Attack;	
			_tStateStart=t;

			SAFE_REPLACE(_target,target);

			return;
		}
	}
}

void CBgnThreat_Attack::_Stop()
{
	if (_GetLo())
	{
		CLevelSkillDriver *driver=_GetLo()->GetSkillDriver();
		if (driver)
			driver->StopMove();
	}

	SAFE_RELEASE(_target);
	_state=None;

}



void CBgnThreat_Attack::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpThreat_Attack*pad=_GetPad<CBgpThreat_Attack>();
// 	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
// 	if (!recSkill)
// 	{
// 		_OutputOk(outputs,1,"结束");
// 		return;
// 	}

	Update(outputs);
}

BOOL CBgnThreat_Attack::_IsInKeepDist(CLevelObj *lo,CLevelObj *target)
{
	CBgpThreat_Attack*pad=_GetPad<CBgpThreat_Attack>();
	return (target->GetFramePos().getDistanceSQFrom(lo->GetFramePos())<pad->distKeep*pad->distKeep);
}


void CBgnThreat_Attack::Update(BGNOutputs &outputs)
{
	CBgpThreat_Attack*pad=_GetPad<CBgpThreat_Attack>();
	AnimTick t=_GetT();
	CLevelObj *lo=_GetLo();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	if (_state==Attack)
	{
		//判断只释放一次的技能是否施放完毕
		if (!driver->IsSkillCasting())
		{
			if (_verCast!=(DWORD)driver->GetCastVer())
			{
				LevelRecordSkill *recSkill=lo->GetLevel()->GetRecords()->GetSkill(pad->idSkill);
				if (recSkill)
				{
					if (!recSkill->Continuous)
					{
						SAFE_RELEASE(_target);
						_OutputOk(outputs,1,"结束");
						return;
					}
				}
			}
		}
		if (!driver->IsWorking())
		{
			_Stop();
		}
	}

	LevelBehaviorContext *ctx=_GetCtx();
	CLevelObj *target=_GetThreat();
	VerifyLevelObjAlive(_target);

	switch(_state)
	{
		case None:
		{
			if (target)
			{
				_tLastCheckEscape=t;
				_Start(target,_IsInKeepDist(lo,target));
			}
			break;
		}
		case Escape:
		{
			if (target==_target)
			{
				if (target)
				{
					if ((t>_tStateStart+_durEscape)||(!_IsInKeepDist(lo,target)))
						_Start(target,FALSE);
				}
			}
			else
			{
				if (target)
					_Start(target,_IsInKeepDist(lo,target));
				else
					_Stop();
			}
			break;
		}
		case Attack:
		{
			if (!driver->IsSkillCasting())
			{
				//在跟随
				BOOL bNewCast=FALSE;
				if (driver->GetCastVer()!=_verCast)
				{
					_verCast=driver->GetCastVer();
					bNewCast=TRUE;
				}
				if (target==_target)
				{
					//旧的target没有变
					if (target)
					{
						//看看要不要escape
						if (t>_tLastCheckEscape+pad->durCheckEscape)
						{
							_tLastCheckEscape=t;
							if (_IsInKeepDist(lo,target))
								_Start(target,TRUE);
						}
					}
				}
				else
				{
					//新的target
					if (target)
					{
						//攻击一个新的对象

						BOOL bKeepAvoid=FALSE;
						if (bNewCast)
							bKeepAvoid=FALSE;

						if (bKeepAvoid)
						{
							float avoid=-1.0f;
							CUnit *unit=lo->GetMove()->GetGroundUnit();
							if (unit)
								avoid=unit->GetLastAvoidRad();
							_Start(target,_IsInKeepDist(lo,target));
							if (unit)
								unit->SetLastAvoidRad(avoid);
						}
						else
							_Start(target,_IsInKeepDist(lo,target));
					}
					else
						_Stop();
				}
			}
			break;
		}
	}

	if (!_target)
		_OutputOk(outputs,1,"结束");
}


void CBgnThreat_Attack::Break(BGNOutputs &outputs)
{
	Destroy();
}



