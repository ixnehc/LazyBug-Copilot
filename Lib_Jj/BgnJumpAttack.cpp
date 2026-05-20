/********************************************************************
	created:	2016/06/10 
	author:		cxi
	
	purpose:	跳跃攻击
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "BgnJumpAttack.h"
#include "LevelRecords.h"


#include "Level.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgn_JumpAttack
BIND_BGN_CLASS(CBgn_JumpAttack,CBgp_JumpAttack);

void CBgn_JumpAttack::Destroy()
{
	_GetLevel()->GetLockers()->UnlockJumpAttack();
}


void CBgn_JumpAttack::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_JumpAttack*pad=_GetPad<CBgp_JumpAttack>();
	CLevelObj *lo=_GetLo();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	_GetLevel()->GetLockers()->LockJumpAttack();

	LevelBehaviorContext *ctx=_GetCtx();

	BOOL bCasted=FALSE;
	//立即开始攻击
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
	if (recSkill)
	{
		LevelObjID idTarget=LevelObjID_Invalid;
		if (pad->_nmVar!=StringID_Invalid)
			_GetID(pad->_nmVar,BehaviorMemType_ObjID,idTarget);

		if (idTarget!=LevelObjID_Invalid)
		{
			LevelSkillTarget target;

			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *loTarget=LevelUtil_GetAliveLo(_GetLevel(),idTarget);
			if (loTarget)
			{
				float radius=loTarget->GetRadius_();
				float radiusMe=_GetLo()->GetRadius_();

//				LevelPos posTarget=loTarget->GetFramePos();
				extern LevelPos LevelUtil_CalcPredictedPos(CLevelObj *loSrc,CLevelObj*loUnit,float dt);
				LevelPos posTarget=LevelUtil_CalcPredictedPos(lo,loTarget,LEVEL_FRAME_INTERVAL*5.0f);
				LevelPos posSrc=_GetLo()->GetFramePos();

				LevelPos dir=posTarget-posSrc;
				dir.safe_normalize();
				posTarget-=dir*(radiusMe+radius);

				target.SetObjID(idTarget);
				LevelSkillArg arg;
				arg.sites.push_back(posTarget);
				driver->Start(LevelSkillType(pad->idSkill),target,FALSE,ClientSkillID_Invalid,pad->grd,&arg);
				bCasted=TRUE;
			}
		}
	}

	if(!bCasted)
	{
		LOG_DUMP_1P("CBgn_JumpAttack",Log_Error,"无法施放技能(没有指定技能ID或者无法找到对象!)(行为图:%s)",StrLib_GetStr(ctx->bg->GetName()));

		_OutputOk(outputs,1,"结束");
		return;
	}
}

void CBgn_JumpAttack::Update(BGNOutputs &outputs)
{
	CBgp_JumpAttack*pad=_GetPad<CBgp_JumpAttack>();

	CLevelObj *lo=_GetLo();
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	LevelBehaviorContext *ctx=_GetCtx();

	if (driver)
	{
		if (!driver->IsWorking())
		{
			_OutputOk(outputs,1,"结束");
			return;
		}
		else
			return;//当前技能还未结束,
	}
	_OutputOk(outputs,1,"结束");
	return;
}
