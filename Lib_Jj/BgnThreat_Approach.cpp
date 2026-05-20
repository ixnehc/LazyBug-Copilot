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

#include "BgnThreat_Approach.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"
#include "LevelTroops.h"

#include "LevelUtil.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_Approach
BIND_BGN_CLASS(CBgnThreat_Approach,CBgpThreat_Approach);

void CBgnThreat_Approach::Destroy()
{
	SAFE_DESTROY(_attrnode);
	SAFE_RELEASE(_target);
}


void CBgnThreat_Approach::_Start(CLevelObj *target)
{
	CBgpThreat_Approach*pad=_GetPad<CBgpThreat_Approach>();
	AnimTick t=_GetT();

	CLevelObj *lo=_GetLo();
	LevelBehaviorContext *ctx=_GetCtx();
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);

	if (target==_target)
		return;

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (driver)
	{
		//立即开始攻击
		LevelSkillTarget tgt;
		tgt.SetObjID(target->GetID());
		_verCast=driver->GetCastVer();
		if (recSkill)
			driver->StartApproach(LevelSkillType(pad->idSkill),tgt);

		SAFE_REPLACE(_target,target);

		return;
	}
}

void CBgnThreat_Approach::_Stop()
{
	if (_GetLo())
	{
		CLevelSkillDriver *driver=_GetLo()->GetSkillDriver();
		if (driver)
			driver->StopMove();
	}

	SAFE_RELEASE(_target);
}



void CBgnThreat_Approach::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpThreat_Approach*pad=_GetPad<CBgpThreat_Approach>();
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
	if (!recSkill)
	{
		_OutputFail(outputs,2,"失败");
		return;
	}

	CLevelObj *target=_GetThreat();
	if (!target)
	{
		_OutputFail(outputs,2,"失败");
		return;
	}

	if (pad->speed>0.0f)
	{
		CLevelObj *lo=_GetLo();
		if (lo)
		{
			CLevelObjMove *move=lo->GetMove();
			if (move)
			{
				SpeedMod *mod=move->ObtainSpeedMod();
				if (mod)
					_attrnode=mod->speed.Add(pad->speed,100);
			}
		}
	}

	_tStart=_GetT();

	_Start(target);
}


void CBgnThreat_Approach::Update(BGNOutputs &outputs)
{
	CBgpThreat_Approach*pad=_GetPad<CBgpThreat_Approach>();
	AnimTick t=_GetT();
	CLevelObj *lo=_GetLo();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	//判断只释放一次的技能是否施放完毕
	if (!driver->IsWorking())
	{
		SAFE_RELEASE(_target);
		if (_verCast!=(DWORD)driver->GetCastVer())
			_OutputOk(outputs,1,"成功");
		else
			_OutputFail(outputs,2,"失败");
		return;
	}
	else
	{
		if (pad->dur>0)
		{
			if (_GetT()>_tStart+pad->dur)
			{
				_OutputFail(outputs,2,"失败");
				return;
			}
		}
	}


	LevelBehaviorContext *ctx=_GetCtx();
	CLevelObj *target=_GetThreat();
	VerifyLevelObjAlive(_target);

	if (target!=_target)
	{
		//新的target
		if (target)
		{
			//逼近一个新的对象
			float avoid=-1.0f;
			CUnit *unit=lo->GetMove()->GetGroundUnit();
			if (unit)
				avoid=unit->GetLastAvoidRad();
			_Start(target);
			if (unit)
				unit->SetLastAvoidRad(avoid);
		}
		else
			_Stop();
	}
}


void CBgnThreat_Approach::Break(BGNOutputs &outputs)
{
	Destroy();
}



