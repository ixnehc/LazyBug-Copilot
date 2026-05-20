/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnAttack.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"
#include "LevelTroops.h"

#include "LevelUtil.h"

#include "LevelEventSrc.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Attack
BIND_BGN_CLASS(CBgn_Attack,CBgp_Attack);

void CBgn_Attack::Destroy()
{
	SAFE_RELEASE(_target);

	SAFE_RELEASE(_fail);
}


extern CLevelObj *DetectFirstTarget(CLevelObj *lo,float rangeMin,float rangeMax,std::vector<LevelDetectTargetFlag>&flags,std::vector<LevelObjRequire>*requires,CLevelObj **loIgnore=NULL,DWORD nIgnores=0);
extern float LevelUtil_GetSpeed(CLevelObj *lo);
extern BOOL LevelUtil_Flee(CLevelObj *lo,CLevelObj *loEscapeFrom,float distKeep,int &signAvoid,CLevelObj *loCenter,float radius);
extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,std::vector<CLevelObj *>&candidates);

BOOL CBgn_Attack::_CheckOwnerSight(CLevelObj *lo,float dist2)
{
	if (!_owner)
		return TRUE;

	if (_owner->GetFramePos().getDistanceSQFrom(lo->GetFramePos())>=_distOwnerSight*_distOwnerSight)
		return FALSE;
	return TRUE;
}


LevelObjMapEnumCallBack CBgn_Attack::_GetDetectDlgt()
{
	if (_owner)
	{
		LevelObjMapEnumCallBack dlgt;
		dlgt.bind(this,&CBgn_Attack::_CheckOwnerSight);
		return dlgt;
	}
	return NULL;
}


static void FillDetectParam(LevelUtilDetectParam &param,CLevelObj *lo,CBgp_Attack *pad,LevelRecordSkill *recSkill)
{
	param.loSrc=lo;
	param.pos=lo->GetFramePos();
	param.rangeMin=pad->radiusMin;
	param.rangeMax=pad->radius;
	param.flags=&recSkill->flagsDetect[0];
	param.nFlags=recSkill->flagsDetect.size();
	param.requires=&recSkill->requires[0];
	param.nRequires=recSkill->requires.size();
	param.weights.CopyFrom(recSkill->weightsDetect);
	param.weights.OverrideFrom(pad->weights);

	param.bTouching=TRUE;
}


CLevelObj *CBgn_Attack::_DetectBest(CBgp_Attack*pad,DetectMethod method,DetectSightType tpSight,LevelRecordSkill *recSkill)
{
	if (!recSkill)
		return NULL;
	CLevelObj *lo=_GetLo();
	if (!lo)
		return NULL;

	if ((method==Detect_Closer)&&(!_target))//当前没有目标,切换为在视野内侦测
		method=Detect_Sight;

	if (method==Detect_Sight)
	{
		if ((tpSight==DetectSightType_Me)||(tpSight==DetectSightType_Owner))
		{
			CLevelObj *loSrc=lo;
			extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
			if (tpSight!=DetectSightType_Me)
			{
				loSrc=LevelUtil_GetOwnerLo(lo);
				if (!loSrc)
					loSrc=lo;
			}

			extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
			LevelUtilDetectParam param;
			FillDetectParam(param,loSrc,pad,recSkill);
			param.fail=_fail;
			param.recent=_target;
			if (tpSight==DetectSightType_Owner)
				param.rangeMax=_distOwnerSight;
			return LevelUtil_DetectBest(param,NULL);
		}
		if (tpSight==DetectSightType_Troop)
		{
			TroopCombatContext *tcc=_GetTcc();
			if (!tcc)
				return NULL;

			LevelUtilDetectParam param;
			FillDetectParam(param,lo,pad,recSkill);
			param.rangeMax=10000000.0f;//very big value
			return LevelUtil_DetectBest(param,NULL,tcc->detects);
		}
	}

	if ((method==Detect_Range)||(method==Detect_Closer))
	{
		float radius=pad->radius;
		if (method==Detect_Closer)
			radius=_target->GetFramePos().getDistanceFrom(lo->GetFramePos());
		else
			radius=recSkill->CastRange+lo->GetRadius_()+1.0f;

		if (radius>pad->radiusMin)
		{
			if ((tpSight==DetectSightType_Me)||(tpSight==DetectSightType_Owner))
			{
				extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
				if (tpSight==DetectSightType_Owner)
					_owner=LevelUtil_GetOwnerLo(lo);

				extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
				LevelUtilDetectParam param;
				FillDetectParam(param,lo,pad,recSkill);
				param.rangeMax=radius;
				param.fail=_fail;
				param.recent=_target;
				CLevelObj *loDetected=LevelUtil_DetectBest(param,_GetDetectDlgt());
				_owner=NULL;
				return loDetected;
			}

			if (tpSight==DetectSightType_Troop)
			{
				TroopCombatContext *tcc=_GetTcc();
				if (!tcc)
					return NULL;

				LevelUtilDetectParam param;
				FillDetectParam(param,lo,pad,recSkill);
				param.rangeMax=radius;
				return LevelUtil_DetectBest(param,NULL,tcc->detects);
			}

		}
	}

	return NULL;
	
}


void CBgn_Attack::_RecordTarget(CLevelObj *loTarget)
{
	CBgp_Attack*pad=_GetPad<CBgp_Attack>();

	SAFE_REPLACE(_target,loTarget);
	if (pad->nmVar!=StringID_Invalid)
	{
		if (loTarget)
			_SetID(pad->nmVar,BehaviorMemType_ObjID,loTarget->GetID());
		else
			_SetID(pad->nmVar,BehaviorMemType_ObjID,LevelObjID_Invalid);
	}

}



void CBgn_Attack::_Start(BGNOutputs &outputs,BOOL bCheckEscapce)
{
	CBgp_Attack*pad=_GetPad<CBgp_Attack>();
	AnimTick t=_GetT();

	CLevelObj *lo=_GetLo();
	LevelBehaviorContext *ctx=_GetCtx();
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);

	_distOwnerSight=20.0f;

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (driver)
	{
		CLevelObj *loDetect=_DetectBest(pad,Detect_Sight,pad->tpSight,recSkill);

		_tLastCheckEscape=t;
		if (loDetect)
		{
			float spd=LevelUtil_GetSpeed(lo);
			if (spd<0.1f)
				spd=0.1f;
			if (bCheckEscapce&&(loDetect->GetFramePos().getDistanceSQFrom(lo->GetFramePos())<pad->distKeep*pad->distKeep))
			{
				//目标离自己太近,我们先要离远一点
				int signAvoid=0;
				LevelUtil_Flee(lo,loDetect,pad->distKeep*1.5f,signAvoid,NULL,0);
				_RecordTarget(loDetect);

				_state=Escape;
				_tStart=t;
				_durReach=ANIMTICK_FROM_SECOND(pad->distKeep/spd);

				return;
			}
			else
			{
				//立即开始攻击
				LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
				if (recSkill)
				{
					LevelSkillTarget target;
					target.SetObjID(loDetect->GetID());
					_verCast=driver->GetCastVer();
					driver->Start(LevelSkillType(pad->idSkill),target,FALSE,ClientSkillID_Invalid,pad->grd,NULL);
				}
				_RecordTarget(loDetect);

				_state=Attack;	
				_tStart=t;
				_durReach=ANIMTICK_FROM_SECOND(lo->GetFramePos().getDistanceFrom(_target->GetFramePos())/spd);

				return;
			}
		}
	}

	//找不到对象,结束
	_OutputOk(outputs,1,"结束");
}


void CBgn_Attack::Start(DWORD iStb,BGNOutputs &outputs)
{
	_Start(outputs,TRUE);
}

void CBgn_Attack::Update(BGNOutputs &outputs)
{
	CBgp_Attack*pad=_GetPad<CBgp_Attack>();
	AnimTick t=_GetT();
	CLevelObj *lo=_GetLo();
	CLevelEventSrc *dmgsrc=lo?lo->GetEventSrc():NULL;

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	LevelBehaviorContext *ctx=_GetCtx();

	VerifyLevelObjAlive(_target);
	VerifyLevelObjAlive(_fail);

	float spd=LevelUtil_GetSpeed(lo);
	if (spd<0.1f)
		spd=0.1f;

	if (_state==Escape)
	{
		if (_target)
		{
			if ((t>_tStart+_durReach)||(_target->GetFramePos().getDistanceSQFrom(lo->GetFramePos())>=pad->distKeep*pad->distKeep))
			{//在范围之外了或者时间到,
				LevelRecordSkill *recSkill=lo->GetLevel()->GetRecords()->GetSkill(pad->idSkill);
				if (recSkill)
				{
					LevelSkillTarget target;
					target.SetObjID(_target->GetID());
					_verCast=driver->GetCastVer();
					driver->Start(LevelSkillType(pad->idSkill),target,FALSE,ClientSkillID_Invalid,pad->grd,NULL);
				}
				_state=Attack;
				_tStart=t;
				_tAttackTarget=t;
				_durReach=ANIMTICK_FROM_SECOND(lo->GetFramePos().getDistanceFrom(_target->GetFramePos())/spd);
				return;
			}
		}
		else
		{
			_state=None;
			_Start(outputs,FALSE);
			return;
		}
	}


	if (_state==Attack)
	{
		LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
		//先检查要不要切换到Escape
		if ((t>_tLastCheckEscape+pad->durCheckEscape)&&(pad->distKeep>0.0f))
		{
			if (driver->IsWorking())
			{
				if (_target)
				{
					if (_target->GetFramePos().getDistanceSQFrom(lo->GetFramePos())<pad->distKeep*pad->distKeep)
					{
						extern BOOL LevelUtil_Flee(CLevelObj *lo,CLevelObj *loEscapeFrom,float distKeep,int &signAvoid,CLevelObj *loCenter,float radius);
						int signAvoid=0;
						LevelUtil_Flee(lo,_target,pad->distKeep*1.5f,signAvoid,NULL,0);
						_tLastCheckEscape=t;

						_state=Escape;
						_tStart=t;
						_durReach=ANIMTICK_FROM_SECOND(pad->distKeep/spd);
						return;
					}
				}
			}
		}

		//判断是不是要取消_fail
		if (_fail)
		{
			AnimTick durForgetFail=ANIMTICK_FROM_SECOND(2.0f);
			if (t>_tFail+durForgetFail)
				SAFE_RELEASE(_fail);
		}

		//判断只释放一次的技能是否施放完毕
		if (driver)
		{
			if (!driver->IsSkillCasting())
			{
				if (_verCast!=driver->GetCastVer())
				{
					LevelRecordSkill *recSkill=lo->GetLevel()->GetRecords()->GetSkill(pad->idSkill);
					if (recSkill)
					{
						if (!recSkill->Continuous)
						{
							_OutputOk(outputs,1,"结束");

							return;
						}
					}
				}
			}
		}

		//判断是否需要重新寻找攻击目标
		if (driver)
		{
			DetectMethod methodReDetect=Detect_None;
			BOOL bKeepAvoid=FALSE;//是否要保持旧的绕路方向
			if (!driver->IsWorking())
			{//技能已经结束了
				methodReDetect=Detect_Sight;
				if (driver->IsFailReach())
				{
					if (_target)
					{
						SAFE_REPLACE(_fail,_target);
						_tFail=t;
					}
				}
				_RecordTarget(NULL);
			}
			else
			{
				if (!driver->IsSkillCasting())
				{//当前不在释放技能
					if (_target)
					{
						if (dmgsrc)
						{
							if (dmgsrc->Exist(LET_Damage,t-ANIMTICK_FROM_SECOND(0.2f)))
							{
								methodReDetect=Detect_Sight;
								bKeepAvoid=TRUE;
							}
						}

						if (methodReDetect==Detect_None)
						{
							if (!driver->CheckInRange())
							{//目标不在释放范围之内
								if (t>_tStart+_durReach+ANIMTICK_FROM_SECOND(0.5f))
								{//预期应该到了,但还没能释放技能,估计在绕远路
									methodReDetect=Detect_Closer;
									bKeepAvoid=TRUE;//尝试寻找其它目标,我们保持旧的绕路方向
								}
								else
								{//预期还没到,但我们尝试找有没有立即就可以攻击的
									if (t>_tAttackTarget+ANIMTICK_FROM_SECOND(2.0f))
									{
										methodReDetect=Detect_Sight;//找范围内的
										bKeepAvoid=TRUE;//尝试寻找其它目标,我们保持旧的绕路方向
									}
								}
							}
						}
					}
					else
						methodReDetect=Detect_Sight;//找范围内的
				}
			}
			if (methodReDetect!=Detect_None)
			{
				CLevelObj *loDetect=_DetectBest(pad,methodReDetect,pad->tpSight,recSkill);

				if (loDetect)
				{
					if (loDetect==_target)
						loDetect=NULL;
				}

				if (loDetect)
				{
					LevelRecordSkill *recSkill=lo->GetLevel()->GetRecords()->GetSkill(pad->idSkill);
					if (recSkill)
					{
						float avoid=-1.0f;
						if (bKeepAvoid)
						{
							CUnit *unit=lo->GetMove()->GetGroundUnit();
							if (unit)
								avoid=unit->GetLastAvoidRad();
						}
						LevelSkillTarget target;
						target.SetObjID(loDetect->GetID());
						_verCast=driver->GetCastVer();
						driver->Start(LevelSkillType(pad->idSkill),target,FALSE,ClientSkillID_Invalid,pad->grd,NULL);


						if (bKeepAvoid)
						{
							CUnit *unit=lo->GetMove()->GetGroundUnit();
							if (unit)
								unit->SetLastAvoidRad(avoid);
						}
					}
					_RecordTarget(loDetect);
					_tStart=t;
					_durReach=ANIMTICK_FROM_SECOND(lo->GetFramePos().getDistanceFrom(_target->GetFramePos())/spd);
				}
				else
				{
					//除了_toIgnore,找不到对象了
// 						if (_toIgnore)
// 						{
// 							if(_toIgnore->IsAlive())
// 							{
// 								if (_toIgnore->TestBuffFlag(BuffFlag_Dead|BuffFlag_LayDown))
// 									SAFE_RELEASE(_toIgnore);
// 							}
// 							else
// 								SAFE_RELEASE(_toIgnore);
// 						}
// 
// 						if (!_toIgnore)
					if (!driver->IsWorking())
					{//真的没有对象了
						driver->ClearWorking();
						_OutputOk(outputs,1,"结束");
					}
				}
			}
		}
	}
}


void CBgn_Attack::Break(BGNOutputs &outputs)
{
	Destroy();
}




////////////////////////////////////////////////////////////////////////
//CBgn_AttackNoTarget
BIND_BGN_CLASS(CBgn_AttackNoTarget,CBgp_AttackNoTarget);


void CBgn_AttackNoTarget::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_AttackNoTarget*pad=_GetPad<CBgp_AttackNoTarget>();
	CLevelObj *lo=_GetLo();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	LevelBehaviorContext *ctx=_GetCtx();

	//立即开始攻击
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
	if (recSkill)
	{
		LevelSkillTarget target;
		driver->StartCast(LevelSkillType(pad->idSkill),target,pad->grd,NULL,NULL);
		_bCasted=TRUE;
	}
}

void CBgn_AttackNoTarget::Update(BGNOutputs &outputs)
{
	CBgp_AttackNoTarget*pad=_GetPad<CBgp_AttackNoTarget>();

	CLevelObj *lo=_GetLo();
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	if (driver)
	{
		BOOL bNext=FALSE;
		if (!driver->IsWorking())
			bNext=TRUE;
		if (bNext)
		{
			if (_bCasted)
			{
				LevelRecordSkill *recSkill=lo->GetLevel()->GetRecords()->GetSkill(pad->idSkill);
				if (recSkill)
				{
					if (!recSkill->Continuous)
					{
						_OutputOk(outputs,1,"结束");
						return;
					}
				}
			}
		}
		if (bNext)
		{
			LevelRecordSkill *recSkill=lo->GetLevel()->GetRecords()->GetSkill(pad->idSkill);
			if (recSkill)
			{
				LevelSkillTarget target;
				driver->StartCast(LevelSkillType(pad->idSkill),target,pad->grd,NULL,NULL);
				_bCasted=TRUE;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////
//CBgn_AttackTargetPos
BIND_BGN_CLASS(CBgn_AttackTargetPos,CBgp_AttackTargetPos);


void CBgn_AttackTargetPos::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_AttackTargetPos*pad=_GetPad<CBgp_AttackTargetPos>();
	CLevelObj *lo=_GetLo();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	LevelBehaviorContext *ctx=_GetCtx();

	//立即开始攻击
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
	if (recSkill)
	{
		LevelSkillTarget target;

		if (TRUE)
		{
			LevelPos pos;
			CBehaviorMem *mem=_GetMem();
			if (mem)
			{
				if (TRUE==mem->GetPos(pad->varFixPos,pos))
					target.SetPos(pos);
			}
		}

		driver->Start(LevelSkillType(pad->idSkill),target,FALSE,ClientSkillID_Invalid,pad->grd,NULL);
		_bCasted=TRUE;
		_verCast=driver->GetCastVer();

		if (!pad->bWaitSkillFinish)
		{
			_OutputOk(outputs,1,"结束");
			return;
		}
	}

}

void CBgn_AttackTargetPos::Update(BGNOutputs &outputs)
{
	CBgp_AttackTargetPos*pad=_GetPad<CBgp_AttackTargetPos>();

	CLevelObj *lo=_GetLo();
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	if (driver)
	{
		BOOL bNext=FALSE;
		if (!driver->IsWorking())
			bNext=TRUE;
		else
		{
			if (_bCasted)
			{
				if (_verCast!=driver->GetCastVer())
					bNext=TRUE;
			}
		}
		if (bNext)
		{
			if (_bCasted)
			{
				_OutputOk(outputs,1,"结束");
				return;
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////
//CBgn_AttackTargetFace
BIND_BGN_CLASS(CBgn_AttackTargetFace,CBgp_AttackTargetFace);


void CBgn_AttackTargetFace::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_AttackTargetFace*pad=_GetPad<CBgp_AttackTargetFace>();
	CLevelObj *lo=_GetLo();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	LevelBehaviorContext *ctx=_GetCtx();

	//立即开始攻击
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
	if (recSkill)
	{
		LevelSkillTarget target;

		if (TRUE)
		{
			LevelFace face=lo->GetFrameFace(); 
			LevelFaceApplyYaw(face,pad->yaw*i_math::GRAD_PI2);

			LevelPos pos=lo->GetFramePos();
			pos+=LevelFaceToDir(face)*1.0f;

			target.SetAim(pos);
		}

		driver->StartCast(LevelSkillType(pad->idSkill),target,pad->grd,NULL,NULL);
		_bCasted=TRUE;
	}
}

void CBgn_AttackTargetFace::Update(BGNOutputs &outputs)
{
	CBgp_AttackTargetFace*pad=_GetPad<CBgp_AttackTargetFace>();

	CLevelObj *lo=_GetLo();
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	if (driver)
	{
		BOOL bNext=FALSE;
		if (!driver->IsWorking())
			bNext=TRUE;
		if (bNext)
		{
			if (_bCasted)
			{
				_OutputOk(outputs,1,"结束");
				return;
			}
		}
	}
}


////////////////////////////////////////////////////////////////////////
//CBgn_AttackTarget
BIND_BGN_CLASS(CBgn_AttackTarget,CBgp_AttackTarget);


void CBgn_AttackTarget::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_AttackTarget*pad=_GetPad<CBgp_AttackTarget>();
	CLevelObj *lo=_GetLo();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	LevelBehaviorContext *ctx=_GetCtx();

	//立即开始攻击
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
	if (recSkill)
	{
		LevelObjID idTarget=LevelObjID_Invalid;
		if (pad->_nmVar!=StringID_Invalid)
			_GetID(pad->_nmVar,BehaviorMemType_ObjID,idTarget);

		if (idTarget!=LevelObjID_Invalid)
		{
			_idTarget=idTarget;
			LevelSkillTarget target;
			target.SetObjID(idTarget);
			driver->Start(LevelSkillType(pad->idSkill),target,FALSE,ClientSkillID_Invalid,pad->grd,NULL);
			_bCasted=TRUE;
		}
	}

	if(!_bCasted)
	{
		LOG_DUMP_1P("CBgn_AttackTarget",Log_Error,"无法施放技能(没有指定技能ID或者无法找到对象!)(行为图:%s)",StrLib_GetStr(ctx->bg->GetName()));

		_OutputOk(outputs,1,"结束");
		return;
	}
}

void CBgn_AttackTarget::Update(BGNOutputs &outputs)
{
	CBgp_AttackTarget*pad=_GetPad<CBgp_AttackTarget>();

	CLevelObj *lo=_GetLo();
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	LevelBehaviorContext *ctx=_GetCtx();

	if (driver)
	{
		BOOL bNext=FALSE;//是否可以放下一次技能
		if (!driver->IsWorking())
			bNext=TRUE;
		if (bNext)
		{
			if (_bCasted)
			{
				LevelRecordSkill *recSkill=lo->GetLevel()->GetRecords()->GetSkill(pad->idSkill);
				if (recSkill)
				{
					if (!recSkill->Continuous)
					{
						_OutputOk(outputs,1,"结束");
						return;
					}
				}
			}
		}
		if (bNext)
		{
			LevelRecordSkill *recSkill=lo->GetLevel()->GetRecords()->GetSkill(pad->idSkill);
			if (recSkill)
			{
				CLevelObj *loTarget=ctx->level->GetIDs()->LoFromID(_idTarget);
				extern BOOL LevelUtil_CheckSkillTarget(LevelRecordSkill *recSkill,CLevelObj *loSrc,CLevelObj *loTarget);
				if (LevelUtil_CheckSkillTarget(recSkill,ctx->lo,loTarget))
				{
					LevelSkillTarget target;
					target.SetObjID(_idTarget);
					driver->Start(LevelSkillType(pad->idSkill),target,FALSE,ClientSkillID_Invalid,pad->grd,NULL);
					_bCasted=TRUE;
					return;
				}
			}
		}
		else
			return;//当前技能还未结束,
	}
	_OutputOk(outputs,1,"结束");
	return;
}



////////////////////////////////////////////////////////////////////////
//CBgn_AttackHoldPos
BIND_BGN_CLASS(CBgn_AttackHoldPos,CBgp_AttackHoldPos);



BOOL CBgn_AttackHoldPos::_Check(CLevelObj *lo,float dist2)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgp_AttackHoldPos*pad=_GetPad<CBgp_AttackHoldPos>();

	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);

	float range=lo->GetRadius_()+recSkill->CastRange+ctx->lo->GetRadius_();
	range+=0.1f;

	if (range*range>dist2)
		return TRUE;
	return FALSE;
}


void CBgn_AttackHoldPos::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_AttackHoldPos*pad=_GetPad<CBgp_AttackHoldPos>();
	AnimTick t=_GetT();

	CLevelObj *lo=_GetLo();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	extern BOOL LevelUtil_IsMovingOrRotating(CLevelObj *lo);
	if (!LevelUtil_IsMovingOrRotating(lo))
	{
		LevelBehaviorContext *ctx=_GetCtx();

		CLevelObj *loDetect=NULL;

		LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
		if (recSkill)
		{
			LevelObjMapEnumCallBack dlgt;
			dlgt.bind(this,&CBgn_AttackHoldPos::_Check);
			CLevelObj *DetectFirstTarget(CLevelObj *lo,float rangeMin,float rangeMax,std::vector<LevelDetectTargetFlag>&flags,
				std::vector<LevelObjRequire>*requires,LevelObjMapEnumCallBack dlgt,CLevelObj **loIgnore,DWORD nIgnores);
			loDetect=DetectFirstTarget(lo,0.0f,recSkill->CastRange+lo->GetRadius_()+2.0f,pad->flagsDetect,&recSkill->requires,dlgt,NULL,0);
		}

		if (loDetect)
		{
			LevelSkillTarget target;
			target.SetObjID(loDetect->GetID());
			driver->Start(LevelSkillType(pad->idSkill),target,TRUE,ClientSkillID_Invalid,pad->grd,NULL);
			return;
		}
	}

	_OutputOk(outputs,1,"结束");
}

void CBgn_AttackHoldPos::Update(BGNOutputs &outputs)
{
	CLevelSkillDriver *driver=_GetSkillDriver();
	if (driver)
	{
		if (driver->IsWorking())
			return;
	}
	_OutputOk(outputs,1,"结束");
}


////////////////////////////////////////////////////////////////////////
//CBgn_CancelSkill
BIND_BGN_CLASS(CBgn_CancelSkill,CBgp_CancelSkill);

void CBgn_CancelSkill::Start(DWORD iStb,BGNOutputs &outputs)
{
	extern LevelSkillID LevelUtil_CancelSkill(CLevelObj *lo,BOOL bStopAct);
	CBgp_CancelSkill*pad=_GetPad<CBgp_CancelSkill>();

	CLevelObj *lo=_GetLo();

	LevelSkillID idSkill=LevelSkillID_Invalid;
	if (lo)
	{
		if (pad->idSkills.size()>0)
		{
			CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
			if (skill)
			{
				LevelRecordSkill *rec=skill->GetRec();
				if (rec)
				{
					int idx;
					VEC_FIND(pad->idSkills,rec->GetID(),idx);
					if (idx!=-1)
					{
						idSkill=LevelUtil_CancelSkill(lo,pad->bResetAct);
					}
				}
			}
		}
		else
			idSkill=LevelUtil_CancelSkill(lo,pad->bResetAct);
	}

	_OutputOk(outputs,1,"结束");
}

////////////////////////////////////////////////////////////////////////
//CBgn_CanCancelSkill
BIND_BGN_CLASS(CBgn_CanCancelSkill,CBgp_CanCancelSkill);

void CBgn_CanCancelSkill::Start(DWORD iStb,BGNOutputs &outputs)
{
	extern BOOL LevelUtil_CanCancelSkill(CLevelObj * lo);
	CBgp_CanCancelSkill*pad=_GetPad<CBgp_CanCancelSkill>();

	CLevelObj *lo=_GetLo();

	LevelSkillID idSkill=LevelSkillID_Invalid;
	if (lo)
	{
		CLevelSkillDriver *driver=lo->GetSkillDriver();
		if (driver)
		{
			if (pad->idSkills.size()>0)
			{
				CLevelSkill *skill=LevelUtil_GetCastingSkill(lo);
				if (skill)
				{
					LevelRecordSkill *rec=skill->GetRec();
					if (rec)
					{
						int idx;
						VEC_FIND(pad->idSkills,rec->GetID(),idx);
						if (idx!=-1)
						{
							idSkill=LevelUtil_CanCancelSkill(lo);
						}
					}
				}
			}
			else
			{
				idSkill=LevelUtil_CanCancelSkill(lo);
			}
		}
	}

	if (idSkill==LevelSkillID_Invalid)
		_OutputFail(outputs,2,"否");
	else
		_OutputOk(outputs,1,"是");
}
