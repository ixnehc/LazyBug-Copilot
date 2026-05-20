#include "stdh.h"

#include "LevelSkillDriver.h"

#include "LevelSkill.h"


#include "LoUnit.h"

#include "unitmgr/UnitMgr.h"

#include "LevelRecordSkill.h"

#include "Level.h"
#include "LevelClasses.h"
#include "LevelDecider.h"
#include "LevelRecords.h"
#include "LoAgent.h"

#include "LevelRecordUnit.h"

#include "Log/LogDump.h"

// class CLevelSkill;
// CLevelSkill*NewLevelSkill(ClassUID uid)
// {
// 	if (uid>=MAX_LEVEL_UID)
// 		return NULL;
// 	CClass *clss=GetEntries()[uid].clssSkill;
// 	return clss?(CLevelSkill*)clss->New():NULL;
// }


CLevelSkill *NewSkill(LevelRecordSkill *recSkill)
{
	if (recSkill->param)
	{
		CClass *clss=recSkill->param->GetSkillClass();
		return clss?(CLevelSkill*)clss->New():NULL;
	}
	return NULL;
}


//////////////////////////////////////////////////////////////////////////
//CSkillDriver
extern CLevelObj *LevelUtil_GetTargetObj(CLevel *level,LevelSkillTarget &target);

void CLevelSkillDriver::ClearWorking()
{

	if (!_bWorking)
		return;

	if (!_skill)
	{
//		_owner->MoveCmd_ResetIdle();
		if (_owner->GetMove())
		{
			if (_owner->GetMove()->IsMovingEnd())
				_owner->MoveCmd_ResetIdle();
		}
		_owner->MoveCmd_RequestNoTarget();
	}

	Safe_Class_Delete(_arg);

	SAFE_RELEASE(_targetObj);

	SAFE_RELEASE(_skill);
	SAFE_RELEASE(_skillIntend);

	SAFE_RELEASE(_rec);

	ZeroWorking();
}

static void _ClearPending(PendingSkill *pending)
{
	if (pending)
		Safe_Class_Delete(pending->arg);
	Safe_Class_Delete(pending);
}

void CLevelSkillDriver::ClearPending()
{
	_ClearPending(_pending);
	_pending=NULL;
}

void CLevelSkillDriver::_DoIgnoreCastSkill()
{
	_owner->MoveCmd_ResetIdle();

	_verCast++;

	SAFE_RELEASE(_skillIntend);

//	_owner->StartCD(_rec);
}


BOOL CLevelSkillDriver::_DoCastSkill(ClientSkillID idClient,LevelOpLink *link)
{
	_owner->MoveCmd_ResetIdle();

	_verCast++;

	_skill=_skillIntend;
	_skillIntend=NULL;

	if (!_skill)
		return FALSE;//没这号技能,直接失败

	if (!_skill->PreInitStartCheck(GetOwner(),_rec,_target))
	{
		SAFE_RELEASE(_skill);
		return FALSE;
	}

	_skill->Init(this,GetOwner()->GetLevel()->GenSkillID(),idClient,_grd);

	if (TRUE)
	{
		LevelSkillTarget::TypeMask mask=_skill->GetTargetTypes();
		if (!SkillTarget_CheckType(mask,_target.tp))
		{
			_skill->Finish();
			SAFE_RELEASE(_skill);
			return FALSE;//Target 类型不匹配
		}
	}

	if (_bPassive)
		GetOwner()->GetLevel()->GetDecider()->MakeCost(_skill);

	if (!link)
		_skill->Start();
	else
		_skill->Start(*link);

	if (!_skill)
	{//技能在释放过程中,自己中断了自己,目前当成释放失败处理
		return FALSE;
	}

	if ((_skill->GetState()==SkillState_None)||(_skill->GetState()==SkillState_Fail))
	{
		return FALSE;
	}

	//技能没有立即失败的话,说明已经释放出来了,我们开始CoolDown
	_owner->StartCD(_rec);

	//加到CLevelSkills里去
	_owner->GetLevel()->GetSkills()->AddSkill(_skill);

	return TRUE;

}

float CLevelSkillDriver::_CalcDirTol(LevelRecordSkill *rec)
{
	if (rec)
		return rec->tolFace*i_math::GRAD_PI2;
	return i_math::Pi*2.0f;
}

BOOL CLevelSkillDriver::_CheckInRange(CLevelObj *loTarget,LevelSkillTarget &target,float range)
{

	VerifyLevelObjAlive(loTarget);

	BOOL bInRange=FALSE;
	if (target.tp==LevelSkillTarget::Target_DefObj)
	{
		if (!loTarget)
			return FALSE;

		if (CLevelDecider::CheckInRange(_owner,loTarget,range))
			bInRange=TRUE;
	}
	else
	{
		if (target.tp==LevelSkillTarget::Target_Pos)
		{
			if (CLevelDecider::CheckInRange(_owner,target.Pos(),range))
				bInRange=TRUE;
		}
		else
		{
			if (target.tp==LevelSkillTarget::Target_Pos3D)
			{
				if (CLevelDecider::CheckInRange(_owner,target.Pos3D(),range))
					bInRange=TRUE;
			}
			else
			{
				if (target.tp==LevelSkillTarget::Target_FixPosAndObj)
				{
					if (CLevelDecider::CheckInRange(_owner,target.Pos(),0.01f))//很近
						bInRange=TRUE;
				}
				else
					bInRange=TRUE;
			}
		}
	}
	return bInRange;
}


BOOL CLevelSkillDriver::CheckInRange()
{
	if (!IsWorking())
		return FALSE;

	return _CheckInRange(_targetObj,_target,_rangeFollow);
}

BOOL CLevelSkillDriver::_CalcDirTargetPos(LevelPos &pos,CLevelObj *loTarget,LevelSkillTarget &target)
{
	switch(target.tp)
	{
		case LevelSkillTarget::Target_DefObj:
		case LevelSkillTarget::Target_FixPosAndObj:
		{
			if (!loTarget)
				return FALSE;
			pos=loTarget->GetFramePos();
			break;
		}
		case LevelSkillTarget::Target_Pos:
		{
			pos=target.Pos();
			break;
		}
		case LevelSkillTarget::Target_Pos3D:
		{
			LevelPos3D pos3D=target.Pos3D();
			pos.set(pos3D.x,pos3D.z);
			break;
		}
		case LevelSkillTarget::Target_Aim:
		{
			pos=target.Aim();
			break;
		}
		case LevelSkillTarget::Target_None:
		{
			assert(_owner);
			pos=_owner->GetFramePos();
			break;
		}
		default:
		{
			assert(FALSE);
			return FALSE;
		}
	}
	return TRUE;
}

BOOL CLevelSkillDriver::_CheckInDir(BOOL bIgnoreTol,LevelRecordSkill *rec,CLevelObj *loTarget,LevelSkillTarget &target)
{
	if (_owner->IsPlayer())
		return TRUE;

	float tolDir=_CalcDirTol(rec);
	if (tolDir>=i_math::Pi)
		return TRUE;

	LevelPos pos;
	if (!_CalcDirTargetPos(pos,loTarget,target))
		return FALSE;

	if (bIgnoreTol)
		tolDir=0.5f*i_math::GRAD_PI2;//设为很小的角度

	return CLevelDecider::CheckInDir(_owner,pos,tolDir);
}


BOOL CLevelSkillDriver::CheckInDir(BOOL bIgnoreTol)
{
	if (!IsWorking())
		return FALSE;

	return _CheckInDir(bIgnoreTol,_rec,_targetObj,_target);
}


BOOL CLevelSkillDriver::_CheckCastSpacetime(LevelSkillDriverMode mode,LevelRecordSkill *rec,CLevelObj *loTarget,
											LevelSkillTarget &target,float range,BOOL bIgnoreFaceCheck,BOOL &bInRange,BOOL &bInDir)
{
	bInDir=bInRange=FALSE;
	if ((target.tp==LevelSkillTarget::Target_DefObj)&&(mode==LevelSkillDriverMode_Follow))
	{
		//如果为LevelSkillDriverMode_Follow跟随一个Obj,则永远不Ready(永远跟随)
		return FALSE;
	}

	BOOL bReady=FALSE;
	bInRange=_CheckInRange(loTarget,target,range);
	if (bInRange)
	{
		bInDir=bIgnoreFaceCheck?TRUE:_CheckInDir(FALSE,rec,loTarget,target);
		if (bInDir)
		{
			if (rec)
			{
				if (CLevelDecider::CheckSkillCDOver(_owner,rec))
					bReady=TRUE;
			}
			else
				bReady=TRUE;//Dummy技能,不考虑CD,直接施放(虽然必定施放失败)
		}
	}

	return bReady;
}

BOOL CLevelSkillDriver::_CheckCastSpacetime(BOOL &bInRange,BOOL &bInDir)
{
	return _CheckCastSpacetime((LevelSkillDriverMode)_mode,_rec,_targetObj,_target,_rangeFollow,FALSE,bInRange,bInDir);
}


BOOL CLevelSkillDriver::StopMove()
{
	if (_IsSkillCasting())
		return TRUE;
	ClearWorking();
	return TRUE;
}



void CLevelSkillDriver::_SetPending_Default(LevelSkillType &tpSkill,LevelSkillTarget &target,LevelSkillGrade grd,LevelSkillArg *arg,float rangeFollowOverride)
{
	if (!_pending)
		_pending=Class_New2(PendingSkill);
	else
		_pending->Zero();

	_pending->mode=LevelSkillDriverMode_Default;

	_pending->tpSkill=tpSkill;
	_pending->target=target;
	_pending->grd=grd;
	if (arg)
		_pending->arg=arg->Clone();
	_pending->rangeFollowOverride=rangeFollowOverride;
}

void CLevelSkillDriver::_SetPending_Follow(LevelSkillTarget &target,float rangeFollowOverride)
{
	if (!_pending)
		_pending=Class_New2(PendingSkill);
	else
		_pending->Zero();

	_pending->mode=LevelSkillDriverMode_Follow;
	_pending->target=target;
	_pending->rangeFollowOverride=rangeFollowOverride;

}

void CLevelSkillDriver::_SetPending_Approach(LevelSkillType &tpSkill,LevelSkillTarget &target,float rangeFollowOverride)
{
	if (!_pending)
		_pending=Class_New2(PendingSkill);
	else
		_pending->Zero();

	_pending->mode=LevelSkillDriverMode_Approach;

	_pending->tpSkill=tpSkill;
	_pending->target=target;
	_pending->rangeFollowOverride=rangeFollowOverride;
}



BOOL CLevelSkillDriver::_CheckOwnerCanStartSkill()
{
	BOOL bCanStart=TRUE;
	if (!CLevelDecider::CanStartSkill(_owner))
		bCanStart=FALSE;
	else
	{
		if (_skill)
			bCanStart=FALSE;
		else
		{
			if (_pause!=NotPaused)
				bCanStart=FALSE;
		}
	}
	return bCanStart;
}


BOOL CLevelSkillDriver::Combine(LevelSkillTarget &target,ClientSkillID idClient)
{
	if (!_bWorking)
		return FALSE;
	if (!_IsSkillCasting())
		return FALSE;
	if (_skill->GetClientID()!=idClient)
		return FALSE;

	_target=target;
	return _skill->Combine(_target);
}

float CLevelSkillDriver::_CalcCastRange(LevelRecordSkill *rec,CLevelSkill *skill,LevelSkillTarget &target)
{
	if ((!rec)||(!skill))
		return 0.0f;

	if (skill->IsInvokingAgent())
	{
		if (target.tp==LevelSkillTarget::Target_DefObj)
		{
			CLevelObj *lo=GetOwner()->GetLevel()->GetIDs()->LoFromID(target.ObjID());
			if (lo->GetType()==LevelObjType_Agent)
			{
				return ((CLoAgent*)lo)->GetInvokeRange();
			}
		}
	}

	return rec->CastRange;
}


float CLevelSkillDriver::_CalcCastRange()
{
	return _CalcCastRange(_rec,_skillIntend,_target);
}

BOOL CLevelSkillDriver::_CanFollow()
{
	//由客户端驱动的,或者_owner为无法移动的Obj,或者为固定施放的技能
	if ((_bPassive)||(!_owner->GetMove())||(_bNoFollow))
		return FALSE;
	if (!_owner->GetMove()->IsMovingMethod())
		return FALSE;
	return TRUE;
}


BOOL CLevelSkillDriver::Start(LevelSkillType &tpSkill,LevelSkillTarget &target,BOOL bPassive,ClientSkillID idClient,LevelSkillGrade grd,LevelSkillArg *arg,float rangeFollowOverride)
{
	extern LevelRecordSkill *LevelUtil_GetSkillRecord(CLevelObj *lo,LevelSkillType tpSkill);
	LevelRecordSkill *recSkill=LevelUtil_GetSkillRecord(_owner,tpSkill);
	if (!recSkill)
		return FALSE;

	if (_skill)
	{
		if (!_IsSkillCasting())
		{
			_owner->StartCD(_skill->GetRec());
			SAFE_RELEASE(_skill);//这个_skill已经不归我管了
		}
	}

	if (bPassive)
	{
		extern BOOL LevelUtil_TestSkillCost(LevelSkillType &tpSkill,CLevelObj *owner);

		if (!LevelUtil_TestSkillCost(tpSkill,_owner))
			return FALSE;
	}

	if (!_CheckOwnerCanStartSkill())
	{//暂时不能开始

		if (_pause==NotPaused)
		{
			if (!bPassive)
				_SetPending_Default(tpSkill,target,grd,arg,rangeFollowOverride);//记录下来,留待以后处理
		}

		return FALSE;
	}

	ClearWorking();//清除已有的

	_bWorking=TRUE;
	_mode=LevelSkillDriverMode_Default;

	SAFE_REPLACE(_rec,recSkill);

	if (TRUE)//准备一个skill指针
	{
		_skillIntend=NewSkill(_rec);
		if (!_skillIntend)
		{
			ClearWorking();
			return FALSE;
		}
		_skillIntend->AddRef();
	}
	_target=target;
	if (arg)
		_arg=arg->Clone();

	_bContinuous=_rec->Continuous;
	_bNoFollow=_rec->bNoFollow;

	if (bPassive)
		_bContinuous=0;
	_bPassive=bPassive;
	_grd=grd;

	if (TRUE)
	{
		_rangeFollow=_CalcCastRange();
		if (TRUE)
		{
			float tolerance=0.2f;
			if (bPassive)
				tolerance=_rec->CastRangeTolerance+3.0f;//被动方式的话,误差容忍度大一些,毕竟如果误判的话无法补救
			_rangeFollow+=tolerance;
		}
	}

	if (rangeFollowOverride>=0.0f)
		_rangeFollow=rangeFollowOverride;

	//确保target有效
	_targetObj=LevelUtil_GetTargetObj(_owner->GetLevel(),_target);
	SAFE_ADDREF(_targetObj);
	if (!bPassive)
	{
		if (_CheckLostTarget())
		{
			LOG_DUMP_1P("LevelSkillDriver",Log_Error,"技能在施放时发现无效的目标对象!(%s)",recSkill->Name.c_str());
			ClearWorking();
			return FALSE;
		}
	}

	BOOL bInRange=FALSE;
	BOOL bInDir=FALSE;
	BOOL bSpacetimeReady=bPassive?TRUE:_CheckCastSpacetime(bInRange,bInDir);
	if (!bSpacetimeReady)
	{
		if ((!bInRange)||(!bInDir))
		{//不在范围内
			if (!_CanFollow())
			{//又无法跟随
				ClearWorking();
				return FALSE;
			}
		}
	}

	if (bSpacetimeReady)
	{
		if(FALSE==_DoCastSkill(bPassive?idClient:ClientSkillID_Invalid,NULL))
		{
			ClearWorking();
			return FALSE;
		}

		assert(_skill);

		//如果这个技能仍然在Casting,我们要等待
		if (_IsSkillCasting())
			return TRUE;

		_owner->StartCD(_skill->GetRec());
		SAFE_RELEASE(_skill);//这个_skill已经不归我管了

		if (!_bContinuous)
		{
			ClearWorking();
			return TRUE;//只施放一次的话,我们就结束了
		}
	}

	//设定跟随
	_DoFollow();

	return TRUE;
}

BOOL CLevelSkillDriver::StartFollow(LevelSkillTarget &target,float rangeFollowOverride,BOOL bClosestFollow)
{
	if (_skill)
	{
		if (!_IsSkillCasting())
		{
			_owner->StartCD(_skill->GetRec());
			SAFE_RELEASE(_skill);//这个_skill已经不归我管了
		}
	}

	if (!_CheckOwnerCanStartSkill())
	{//暂时不能开始

		if (_pause==NotPaused)
		{
			_SetPending_Follow(target,rangeFollowOverride);//记录下来,留待以后处理
		}

		return FALSE;
	}

	ClearWorking();//清除已有的

	_bWorking=TRUE;
	_mode=LevelSkillDriverMode_Follow;

	SAFE_RELEASE(_rec);
	_target=target;

	_bPassive=FALSE;
	_grd=LevelSkillGrade_Invalid;

	_bClosestFollow=bClosestFollow;
	_rangeFollow=2.0f;

	if (rangeFollowOverride>=0.0f)
		_rangeFollow=rangeFollowOverride;

	//如果目标为一个Obj的话,确保它是有效的,如果无效,直接失败
	if ((_target.tp==LevelSkillTarget::Target_DefObj)||(_target.tp==LevelSkillTarget::Target_FixPosAndObj))
	{
		if(_target.ObjID()!=LevelObjID_Invalid)
		{
			_targetObj=_owner->GetLevel()->GetIDs()->LoFromID(target.ObjID());
			if (!_targetObj)
			{
				ClearWorking();
				return FALSE;
			}
			_targetObj->AddRef();
		}
	}

	BOOL bInRange=FALSE;
	BOOL bInDir=FALSE;
	BOOL bSpacetimeReady=_CheckCastSpacetime(bInRange,bInDir);
	if (!bSpacetimeReady)
	{
		if ((!bInRange)||(!bInDir))
		{//不在范围内
			if (!_CanFollow())
			{//又无法跟随
				ClearWorking();
				return FALSE;
			}
		}
	}

	if (bSpacetimeReady)
	{
		_owner->MoveCmd_ResetIdle();

		_verCast++;

		ClearWorking();
		return FALSE;
	}

	//设定跟随
	_DoFollow();

	return TRUE;


	return Start(LevelSkillType(),target,FALSE,ClientSkillID_Invalid,LevelSkillGrade_Invalid,NULL,rangeFollowOverride);//启动一个Dummy技能
}

BOOL CLevelSkillDriver::StartApproach(LevelSkillType &tpSkill,LevelSkillTarget &target,float rangeFollowOverride)
{
	extern LevelRecordSkill *LevelUtil_GetSkillRecord(CLevelObj *lo,LevelSkillType tpSkill);
	LevelRecordSkill *recSkill=LevelUtil_GetSkillRecord(_owner,tpSkill);
	if (!recSkill)
		return FALSE;

	if (_skill)
	{
		if (!_IsSkillCasting())
		{
			_owner->StartCD(_skill->GetRec());
			SAFE_RELEASE(_skill);//这个_skill已经不归我管了
		}
	}

	if (!_CheckOwnerCanStartSkill())
	{//暂时不能开始

		if (_pause==NotPaused)
			_SetPending_Approach(tpSkill,target,rangeFollowOverride);//记录下来,留待以后处理

		return FALSE;
	}

	ClearWorking();//清除已有的

	_bWorking=TRUE;
	_mode=LevelSkillDriverMode_Approach;

	SAFE_REPLACE(_rec,recSkill);
	if (_rec)//准备一个skill指针
	{
		_skillIntend=NewSkill(_rec);
		if (!_skillIntend)
		{
			ClearWorking();
			return FALSE;
		}
		_skillIntend->AddRef();
	}
	_target=target;

	if (_rec)
		_bNoFollow=_rec->bNoFollow;

	if (_rec)
	{
		_rangeFollow=_CalcCastRange();
		if (TRUE)
		{
			float tolerance=0.2f;
			_rangeFollow+=tolerance;
		}
	}
	else
	{
		_bClosestFollow=1;//Dummy技能,使用ClosestFollow模式移动到目的点
		_rangeFollow=2.0f;
	}

	if (rangeFollowOverride>=0.0f)
		_rangeFollow=rangeFollowOverride;

	//确保target有效
	_targetObj=LevelUtil_GetTargetObj(_owner->GetLevel(),_target);
	SAFE_ADDREF(_targetObj);
	if (_CheckLostTarget())
	{
		ClearWorking();
		LOG_DUMP_1P("LevelSkillDriver",Log_Error,"技能在施放时发现无效的目标对象!(%s)",recSkill->Name.c_str());
		return FALSE;
	}

	BOOL bInRange=FALSE;
	BOOL bInDir=FALSE;
	BOOL bSpacetimeReady=_CheckCastSpacetime(bInRange,bInDir);
	if (!bSpacetimeReady)
	{
		if ((!bInRange)||(!bInDir))
		{//不在范围内
			if (!_CanFollow())
			{//又无法跟随
				ClearWorking();
				return FALSE;
			}
		}
	}

	if (bSpacetimeReady)
	{
		_DoIgnoreCastSkill();
		ClearWorking();
		return TRUE;
	}

	//设定跟随
	_DoFollow();

	return TRUE;
}

BOOL CLevelSkillDriver::CheckStartCast(LevelSkillType &tpSkill,LevelSkillTarget &target,BOOL bIgnoreFaceCheck)
{
	extern LevelRecordSkill *LevelUtil_GetSkillRecord(CLevelObj *lo,LevelSkillType tpSkill);
	LevelRecordSkill *recSkill=LevelUtil_GetSkillRecord(_owner,tpSkill);
	if (!recSkill)
		return FALSE;

	if (!_CheckOwnerCanStartSkill())
	{//暂时不能开始
		return FALSE;
	}

	CLevelSkill *skill=NewSkill(recSkill);
	if (!skill)
		return FALSE;

	skill->AddRef();
	CLevelObj *loTarget=LevelUtil_GetTargetObj(_owner->GetLevel(),target);

	if (_CheckLostTarget(target,recSkill,loTarget,LevelSkillDriverMode_Cast))
	{
		SAFE_RELEASE(skill);
		return FALSE;
	}

	if (TRUE)
	{
		float rangeFollow=_CalcCastRange(recSkill,skill,target);
		BOOL bInRange=FALSE;
		BOOL bInDir=FALSE;
		if (!_CheckCastSpacetime(LevelSkillDriverMode_Cast,recSkill,loTarget,target,rangeFollow,bIgnoreFaceCheck,bInRange,bInDir))
		{
			SAFE_RELEASE(skill);
			return FALSE;
		}
	}

	if (!skill->PreInitStartCheck(GetOwner(),recSkill,target))
	{
		SAFE_RELEASE(skill);
		return FALSE;
	}

	SAFE_RELEASE(skill);
	return TRUE;
}


BOOL CLevelSkillDriver::StartCast(LevelSkillType &tpSkill,LevelSkillTarget &target,LevelSkillGrade grd,LevelSkillArg *arg,LevelOpLink *link)
{
	extern LevelRecordSkill *LevelUtil_GetSkillRecord(CLevelObj *lo,LevelSkillType tpSkill);
	LevelRecordSkill *recSkill=LevelUtil_GetSkillRecord(_owner,tpSkill);
	if (!recSkill)
		return FALSE;

	if (_skill)
	{
		if (!_IsSkillCasting())
		{
			_owner->StartCD(_skill->GetRec());
			SAFE_RELEASE(_skill);//这个_skill已经不归我管了
		}
	}

	if (!_CheckOwnerCanStartSkill())
	{//暂时不能开始
		return FALSE;
	}

	ClearWorking();//清除已有的

	_bWorking=TRUE;
	_mode=LevelSkillDriverMode_Cast;

	SAFE_REPLACE(_rec,recSkill);
	if (TRUE)//准备一个skill指针
	{
		_skillIntend=NewSkill(_rec);
		if (!_skillIntend)
		{
			ClearWorking();
			return FALSE;
		}
		_skillIntend->AddRef();
	}
	_target=target;
	if (arg)
		_arg=arg->Clone();

	_grd=grd;

	_rangeFollow=10000000.0f;
	_bClosestFollow=0;

	//确保target有效
	_targetObj=LevelUtil_GetTargetObj(_owner->GetLevel(),_target);
	SAFE_ADDREF(_targetObj);
	if (_CheckLostTarget())
	{
		ClearWorking();
		LOG_DUMP_1P("LevelSkillDriver",Log_Error,"技能在施放时发现无效的目标对象!(%s)",recSkill->Name.c_str());
		return FALSE;
	}

	if (TRUE)
	{
		if(FALSE==_DoCastSkill(ClientSkillID_Invalid,link))
		{
			ClearWorking();
			return FALSE;
		}

		assert(_skill);

		//如果这个技能仍然在Casting,我们要等待
		if (_IsSkillCasting())
			return TRUE;

		_owner->StartCD(_skill->GetRec());
		SAFE_RELEASE(_skill);//这个_skill已经不归我管了

		ClearWorking();
		return TRUE;//只施放一次的话,我们就结束了
	}

	return TRUE;
}


BOOL CLevelSkillDriver::_IsSkillCasting()
{
	if (!_skill)
		return FALSE;
	if (_skill->GetState()==SkillState_Casting)
		return TRUE;

	return FALSE;
}

BOOL CLevelSkillDriver::_CheckLostTarget(LevelSkillTarget &target,LevelRecordSkill *rec,CLevelObj *loTarget,LevelSkillDriverMode mode)
{
	if ((target.tp==LevelSkillTarget::Target_DefObj)||(target.tp==LevelSkillTarget::Target_FixPosAndObj))
	{
		if (!loTarget)
			return TRUE;
		if (mode!=LevelSkillDriverMode_Follow)
		{
			extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
			if (!LevelUtil_CheckSkillTarget(rec,_owner,loTarget))
				return TRUE;
		}
	}
	return FALSE;
}


BOOL CLevelSkillDriver::_CheckLostTarget()
{
	return _CheckLostTarget(_target,_rec,_targetObj,(LevelSkillDriverMode)_mode);
}


BOOL CLevelSkillDriver::_Continue()
{

	//判断目标是否失去
	if (_CheckLostTarget())
	{
		ClearWorking();
		return FALSE;
	}

	BOOL bInRange=FALSE;
	BOOL bInDir=FALSE;
	if (_CheckCastSpacetime(bInRange,bInDir))
	{
		if (_mode!=LevelSkillDriverMode_Approach)
		{
			if(FALSE==_DoCastSkill(ClientSkillID_Invalid,NULL))
			{
				ClearWorking();
				return FALSE;
			}
		}
		else
		{
			_DoIgnoreCastSkill();
			ClearWorking();
			return TRUE;
		}

		assert(_skill);

		//如果这个技能仍然在Casting,我们要等待
		if (_IsSkillCasting())
			return TRUE;

		_owner->StartCD(_skill->GetRec());
		SAFE_RELEASE(_skill);//这个_skill已经不归我管了

		if (!_bContinuous)
		{
			ClearWorking();
			return TRUE;//只施放一次的话,我们就结束了
		}
	}
	else
	{
		if ((!bInRange)||(!bInDir))
		{
			if (!_CanFollow())
			{
				ClearWorking();
				return TRUE;
			}
		}
		_DoFollow();
	}

	return TRUE;
}


BOOL CLevelSkillDriver::Update(AnimTick dt)
{
	VerifyLevelObjAlive(_targetObj);

	if (_pending)
	{
		if (_CheckOwnerCanStartSkill())
		{
			PendingSkill *pending=_pending;
			_pending=NULL;

			BOOL bRet=FALSE;

			switch(pending->mode)
			{
				case LevelSkillDriverMode_Default:
					bRet=Start(pending->tpSkill,pending->target,FALSE,ClientSkillID_Invalid,pending->grd,pending->arg,pending->rangeFollowOverride);
					break;
				case LevelSkillDriverMode_Follow:
					bRet=StartFollow(pending->target,pending->rangeFollowOverride);
					break;
				case LevelSkillDriverMode_Approach:
					bRet=StartApproach(pending->tpSkill,pending->target,pending->rangeFollowOverride);
					break;
			}
			_ClearPending(pending);
			return bRet;
		}
	}

	if  (!_bWorking)
		return TRUE;//没有在工作

	if (_pause==Paused)
		return TRUE;//暂停工作

	if (_skill)
	{//当前有技能在执行
		if (_IsSkillCasting())
			return TRUE;

		_owner->StartCD(_skill->GetRec());
		SAFE_RELEASE(_skill);//这个技能已经和我们没关系了

		if (!_bContinuous)
		{
			ClearWorking();
			return TRUE;
		}
		else
		{
			_skillIntend=NewSkill(_rec);//为下一次施放做准备
			SAFE_ADDREF(_skillIntend);
		}

		_DoFollow();
	}
	else
	{//当前在Follow

		//判断目标是否失去
		if (_CheckLostTarget())
		{
			ClearWorking();
			return FALSE;
		}

		if (_target.tp!=LevelSkillTarget::Target_None)
		{
			BOOL bInRangeAndDir=FALSE;
			BOOL bChecked=FALSE;
			if (_targetObj)
			{
				if (_targetObj->GetShapeType()==LevelObjShape_MultiCircle)
				{
					if (_target.tp==LevelSkillTarget::Target_DefObj)
					{
						bChecked=TRUE;
						if (CheckInRange()&&CheckInDir(FALSE))
							bInRangeAndDir=TRUE;
					}
				}
			}
			BOOL bUnitMoving=FALSE;
			if (!bChecked)
			{
				CUnit *unit=_owner->GetUnit();
				if (unit)
				{
					if (unit->GetSession()>=_sessionFollow)//发出的请求已经被执行了
					{
						if ((unit->GetStage()==UnitStage_Reached)||(unit->GetStage()==UnitStage_Faced))
						{
							bInRangeAndDir=TRUE;
							bChecked=TRUE;
						}
					}
					bUnitMoving=unit->IsMoving();
				}
				else
				{
					CUnit3D *unit3D=_owner->GetUnit3D();
					if (unit3D)
					{
						if (unit3D->GetState()==CUnit3D::PostFollow)
						{
							bInRangeAndDir=TRUE;
							bChecked=TRUE;
						}
						else
							bUnitMoving=unit3D->GetState()!=CUnit3D::Idle;
					}
				}
			}

			if (!bChecked)
			{
				if (_rec)
				{
					//不是dummy技能
					if (bUnitMoving)
					{
						if (CheckInRange()&&CheckInDir(TRUE))//使用严格的朝向判断,
							bInRangeAndDir=TRUE;
					}
					else
					{
						if (CheckInRange()&&CheckInDir(FALSE))//单位在静止状态,使用带tolerance的朝向判断,
							bInRangeAndDir=TRUE;
					}
					bChecked=TRUE;
				}
			}

			if (!bInRangeAndDir)
			{
				BOOL bFailFollow=FALSE;
				BOOL bFailFollowCannotReach=FALSE;
				if (!_CanFollow())
					bFailFollow=TRUE;
				else
				{
					CUnit *unit=_owner->GetUnit();
					if (unit)
					{
						if (unit->GetSession()>=_sessionFollow)//发出的请求已经被执行了
						{
							if (unit->IsFailure())
							{
								bFailFollow=TRUE;
								if (unit->GetAbortReason()==UnitAbortReason_CannotReach)
									bFailFollowCannotReach=TRUE;
							}
						}
					}
				}

				if (bFailFollow)
				{//跟丢了
					LevelObjID idFailFollow=LevelObjID_Invalid;
					if (_targetObj)
						idFailFollow=_targetObj->GetID();
					ClearWorking();
					_bFailFollow=1;//标记为跟丢了
					_bFailFollowCannotReach=bFailFollowCannotReach;
					_idFailFollow=idFailFollow;
					return FALSE;
				}

				_DoFollow();

				return TRUE;//继续follow
			}

			//在范围内了

			if (_mode!=LevelSkillDriverMode_Follow)
			{
				assert(_rec);
				if (!CLevelDecider::CheckSkillCDOver(_owner,_rec))
				{
					_DoFollow();
					return TRUE;//CD没到时间
				}
			}
			else
			{
				//Follow模式
				if (_target.tp==LevelSkillTarget::Target_DefObj)
				{
					_DoFollow();
					return TRUE;//对Obj的Follow永远不Ready
				}
			}
		}

		if (_pause!=NotPaused)
			return TRUE;//SkillPaused状态下,禁止使用技能

		//可以释放技能了
		if (TRUE)
		{
			if (_mode!=LevelSkillDriverMode_Approach)
			{
				if (FALSE==_DoCastSkill(ClientSkillID_Invalid,NULL))
				{
					ClearWorking();
					return FALSE;
				}
			}
			else
			{
				_DoIgnoreCastSkill();
				ClearWorking();
				return TRUE;
			}

			assert(_skill);

			if (_IsSkillCasting())
				return TRUE;

			_owner->StartCD(_skill->GetRec());
			SAFE_RELEASE(_skill);//这个_skill已经不归我管了

			if (!_bContinuous)
			{
				ClearWorking();
				return TRUE;//只施放一次的话,我们返回成功
			}
		}
		_DoFollow();//此时肯定在释放范围内
	}

	//设定跟随

	return TRUE;
}

LevelSkillID CLevelSkillDriver::PauseSkill()
{
	VerifyLevelObjAlive(_targetObj);

	LevelSkillID broken=LevelSkillID_Invalid;

	if (_pause!=NotPaused)
		return broken;

	_pause=SkillPaused;
	if (_bWorking)
	{
		if (_skill)
		{
			//当前正在释放技能

			//如果技能仍在casting过程中,要Cancel掉(如果已经Casting完了,那我们也做不了什么)
			if (_skill->GetState()==SkillState_Casting)
				_skill->Break();
			broken=_skill->GetID();

			_owner->StartCD(_skill->GetRec());
			SAFE_RELEASE(_skill);

			if (!_bContinuous)
			{
				ClearWorking();
				return broken;//只施放一次的话,可以停止工作了
			}
		}
	}
	return broken;
}


void CLevelSkillDriver::Pause()
{
	VerifyLevelObjAlive(_targetObj);

	if (_pause==Paused)
		return;

//	assert(_pause==SkillPaused);
	if (_pause!=SkillPaused)
		PauseSkill();

	//停止移动
	_owner->MoveCmd_ResetIdle();

	_pause=Paused;
	ClearPending();
}


BOOL CLevelSkillDriver::Continue()
{
	VerifyLevelObjAlive(_targetObj);

	if (_pause==NotPaused)
		return TRUE;
	if (_pause==SkillPaused)
	{
		_pause=NotPaused;
		return TRUE;
	}

	_pause=NotPaused;

	if (!_bWorking)
		return TRUE;//没有在工作的话,不用做什么

	return _Continue();
}


void CLevelSkillDriver::_DoFollow()
{
	if (!_CanFollow())
		return;
	if (CheckInRange()&&CheckInDir(FALSE))//已经在范围内了
		return;

// 	if (_owner)
// 	{
// 		CUnit *unit=_owner->GetUnit();
// 		if (unit)
// 		{
// 			if (unit->IsMovingOrRotating())
// 				return;//正在follow
// 		}
// 		else
// 		{
// 			CUnit3D *unit3D=_owner->GetUnit3D();
// 			if (unit3D)
// 			{
// 				if (unit3D->GetState()==CUnit3D::Follow)
// 					return;
// 			}
// 		}
// 	}

	//设定跟随
	if (_target.tp==LevelSkillTarget::Target_DefObj)
	{
		if (_targetObj)
		{
			_sessionFollow=_owner->MoveCmd_RequestTarget(_targetObj,_rangeFollow,
				_rec?_rec->bClosestFollow:_bClosestFollow,
				_rec?TRUE:FALSE,
				_rec?_rec->b3DFollowObj:FALSE);
		}
	}

	if ((_target.tp==LevelSkillTarget::Target_Pos))
		_sessionFollow=_owner->MoveCmd_RequestTarget(_target.Pos(),_rangeFollow,_bClosestFollow,_rec?TRUE:FALSE);

	if ((_target.tp==LevelSkillTarget::Target_Pos3D))
		_sessionFollow=_owner->MoveCmd_RequestTarget(_target.Pos3D(),_rangeFollow,FALSE,_rec?TRUE:FALSE);

	if ((_target.tp==LevelSkillTarget::Target_FixPosAndObj))
		_sessionFollow=_owner->MoveCmd_RequestTarget(_target.Pos(),0.0f,_rec?_rec->bClosestFollow:FALSE,_rec?TRUE:FALSE);

	if (_rec)
		_sessionFollow=_owner->MoveCmd_RequestFacing(_rec->rangeFace,_rec->tolFace*i_math::GRAD_PI2);
}

void CLevelSkillDriver::StopCast(AnimTick tCasting)
{
	if (!IsSkillCasting())
		return;

	_skill->StopCast(tCasting);

	_owner->StartCD(_skill->GetRec());

	ClearWorking();
}


