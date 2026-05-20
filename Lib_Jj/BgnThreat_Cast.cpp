/********************************************************************
	created:	2016/09/14 
	author:		cxi
	
	purpose:	 向Threat释放技能
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnThreat_Cast.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"

#include "LevelUtil.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_Cast
BIND_BGN_CLASS(CBgnThreat_Cast,CBgpThreat_Cast);

void CBgnThreat_Cast::Destroy()
{
}

void CBgnThreat_Cast::_ClearWeaksFilter()
{
	if (!_bWeaksFiltered)
		return;
	CBgpThreat_Cast*pad=_GetPad<CBgpThreat_Cast>();
	if (pad->bFilterWeaks)
	{
		extern void LevelUtil_ClearWeaksFilter(CLevelBgn *bgn);
		LevelUtil_ClearWeaksFilter(this);
	}

	_bWeaksFiltered=FALSE;

}


void CBgnThreat_Cast::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpThreat_Cast*pad=_GetPad<CBgpThreat_Cast>();
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);

	_tStart=_GetT();

	if (!recSkill)
	{
		_OutputFail(outputs,2,"失败");
		return;
	}

	if (pad->bFilterWeaks)
	{
		extern void LevelUtil_SetWeaksFilter(WeaksEx &weaks,CLevelBgn *bgn);
		LevelUtil_SetWeaksFilter(pad->weaksFilter,this);
		_bWeaksFiltered=TRUE;
	}


	CLevelSkillDriver *driver=_GetLo()->GetSkillDriver();
	if (driver)
	{
		//立即开始攻击
		LevelSkillTarget tgt;
		if (pad->tpSkillTarget==LevelSkillTarget::Target_DefObj)
		{
			CLevelObj *target=_GetLevelSkillTarget_Obj(pad->target);
			if (!target)
			{
				_OutputFail(outputs,2,"失败");
				return;
			}

			tgt.SetObjID(target->GetID());
		}
		if (pad->tpSkillTarget==LevelSkillTarget::Target_FixPosAndObj)
		{
			CLevelObj *target=_GetLevelSkillTarget_Obj(pad->target);
			if (!target)
			{
				_OutputFail(outputs,2,"失败");
				return;
			}

			LevelPos posTarget;
			if(_GetPos(pad->varFixPos,posTarget))
			{
				tgt.SetFixPosAndObj(posTarget,0.0f,target->GetID());
			}
			else
			{
				assert(FALSE);
				_ClearWeaksFilter();
				_OutputFail(outputs,2,"失败");
				return;
			}
		}
		if (pad->tpSkillTarget==LevelSkillTarget::Target_Pos)
		{
			LevelPos posTarget;
			if (!_GetLevelSkillTarget_Pos(pad->target,posTarget))
			{
				_OutputFail(outputs,2,"失败");
				return;
			}

			if (pad->durPredict>0.0f)
			{
				CLevelObj *target=_GetLevelSkillTarget_Obj(pad->target);
				if (target)
				{
					extern LevelPos LevelUtil_CalcPredictedPos(CLevelObj *loSrc,CLevelObj *loTarget,float dtPredict);
					posTarget=LevelUtil_CalcPredictedPos(_GetLo(),target,pad->durPredict);
				}
			}
			tgt.SetPos(posTarget);
		}

		if (pad->bInterruptCurSkill)
		{
			extern BOOL LevelUtil_InterruptCastingSkill(CLevelObj *lo);
			if (!LevelUtil_InterruptCastingSkill(_GetLo()))
			{
				_OutputFail(outputs,2,"失败");
				return;
			}
		}

		_verCast=driver->GetCastVer();

		if (ctx->idxSlate==LevelSlateIdx_Invalid)
			driver->StartCast(LevelSkillType(pad->idSkill),tgt,pad->grd,NULL,NULL);
		else
		{
			LevelSkillArg arg;
			VEC_SET_BUFFER(arg.data,&ctx->idxSlate,sizeof(ctx->idxSlate));
			driver->StartCast(LevelSkillType(pad->idSkill),tgt,pad->grd,&arg,NULL);
		}

		return;
	}
}


void CBgnThreat_Cast::Update(BGNOutputs &outputs)
{
	CBgpThreat_Cast*pad=_GetPad<CBgpThreat_Cast>();
	CLevelObj *lo=_GetLo();

	if (pad->bFilterWeaks&&pad->durWeaks>0)
	{
		if (_GetT()>=_tStart+pad->durWeaks)
			_ClearWeaksFilter();
	}

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	//判断技能是否施放成功
	if (!driver->IsWorking())
	{
		if (_verCast!=(DWORD)driver->GetCastVer())
		{
			_ClearWeaksFilter();
			_OutputOk(outputs,1,"成功");
		}
		else
		{
			_ClearWeaksFilter();
			_OutputFail(outputs,2,"失败");
		}
		return;
	}
	else
	{
		extern BOOL LevelUtil_CanCancelSkill(CLevelObj *lo);
		if (pad->bFinishAtCanCancel)
		{
			if (LevelUtil_CanCancelSkill(lo))
			{
				_ClearWeaksFilter();
				_OutputOk(outputs,1,"成功");
				return;
			}
		}
	}

}


void CBgnThreat_Cast::Break(BGNOutputs &outputs)
{
	_ClearWeaksFilter();
	Destroy();
}



