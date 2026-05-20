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

#include "BgnSiege.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"

#include "Log/LogDump.h"

#include "Random/Random.h"

#include "Buff_Siege.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Siege
BIND_BGN_CLASS(CBgn_Siege,CBgp_Siege);

void CBgn_Siege::Destroy()
{
	_Finish();
}

void CBgn_Siege::_Finish()
{
	SAFE_RELEASE(_target);
	SAFE_DESTROY(_attrnode);

	CLevelObj *lo=_GetLo();
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (driver)
		driver->StopMove();

	extern CLevelBuff *LevelUtil_FindBuffByID(CLevelObj *lo,LevelBuffID idBuff);
	CLevelBuff *buff=LevelUtil_FindBuffByID(_GetLo(),_idBuff);
	if (buff)
	{
		if (buff->GetClass()->IsSameWith(Class_Ptr2(Buff_Siege)))
		{
			((Buff_Siege*)buff)->Stop();
		}
	}
}


BOOL CBgn_Siege::_Step(BOOL bFirstStep)
{
	if (!_target)
		return FALSE;

	CBgp_Siege*pad=_GetPad<CBgp_Siege>();
	
	CLevelObj *lo=_GetLo();
	LevelPos posCur=lo->GetFramePos();
	LevelPos posTarget=_target->GetFramePos();

	if (_bSiegePos)
	{
		if (_posSiege.getDistanceSQFrom(posCur)<0.5f*0.5f)
		{//离预期的位置有一定差距,
			return TRUE;
		}
	}

	float dist=posCur.getDistanceFrom(posTarget);
	float factor=1.0f;
	if (dist>0.0f)
		factor=_radius/dist;
	factor=factor*factor;
	factor=factor*factor;
	factor=factor*factor;
	factor=factor*factor;
	factor=factor*factor;
	factor=factor*factor;

	float rad=atan2f(posCur.y-posTarget.y,posCur.x-posTarget.x);

	float radOff=30.0f*i_math::GRAD_PI2;
	if (_bRight)
		rad+=radOff;
	else
		rad-=radOff;

	_posSiege=posTarget+LevelPos(cosf(rad)*_radius*factor,sinf(rad)*_radius*factor);
	_bSiegePos=1;

	CUnitMgrNavMesh *unitmgr=_GetLevel()->GetUnitMgr();

	LevelPos posHit;
	if (TRUE)
	{
		float distAhead=1.0f;
		float dist=_posSiege.getDistanceFrom(posCur);
		if (distAhead>dist)
			distAhead=dist;
		LevelPos posAhead=_posSiege;
		if (bFirstStep)
		{
			posAhead=_posSiege+(_posSiege-posCur)*1.0f/dist;
			distAhead+=1.0f;
		}

		if (TRUE==unitmgr->StaticRayCast(UnitFindPath_Walkable,posCur,posAhead,posHit))
		{
			if (posHit.getDistanceSQFrom(posCur)<distAhead)
				return FALSE;//有阻挡
			else
				_posSiege=posHit;
		}
	}

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return FALSE;
	if (!bFirstStep)
	{
		LevelSkillTarget target;
		target.SetPos(_posSiege);
		driver->StartFollow(target,0.1f);
	}
	else
		driver->StopMove();

	if (pad->idBuff!=RecordID_Invalid)
	{
		if (_idBuff==LevelBuffID_Invalid)
		{
			BuffArg_Siege arg;
			arg.idTarget=_target->GetID();
			_idBuff=_GetLevel()->GetDecider()->MakeBuff(lo,pad->idBuff,ANIMTICK_INFINITE,&arg,TRUE);
		}
	}
	return TRUE;
}


void CBgn_Siege::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Siege*pad=_GetPad<CBgp_Siege>();
	_tStart=_GetT();
	CLevelObj *lo=_GetLo();

	BOOL bOk=FALSE;
	if (pad->nmVar!=StringID_Invalid)
	{
		LevelObjID id=LevelObjID_Invalid;
		_GetID(pad->nmVar,BehaviorMemType_ObjID,id);
		CLevelObj *loTarget=_GetLevel()->GetIDs()->LoFromID(id);
		if (loTarget)
		{
			_target=loTarget;
			SAFE_ADDREF(_target);

			LevelPos posCur=lo->GetFramePos();
			LevelPos posTarget=loTarget->GetFramePos();
			_radius=posCur.getDistanceFrom(posTarget);
			_posTargetRecent=posTarget;


			CLevelObjMove *move=lo->GetMove();
			if (move)
			{
				SpeedMod *mod=move->ObtainSpeedMod();
				if (mod)
					_attrnode=mod->speed.Add(pad->speed,100);
			}

			_bRight=CSysRandom::Roll(0.5f);

			if (_Step(TRUE))
				bOk=TRUE;
			else
			{
				_bRight=!_bRight;
				if (_Step(TRUE))
					bOk=TRUE;
			}
		}
	}

	if (bOk)
		return;

	_Finish();

	_OutputOk(outputs,2,"中断");
}

void CBgn_Siege::Update(BGNOutputs &outputs)
{
	CBgp_Siege*pad=_GetPad<CBgp_Siege>();
	AnimTick t=_GetT();
	CLevelObj *lo=_GetLo();

	if (t>_tStart+pad->dur)
	{
		_Finish();
		_OutputOk(outputs,1,"结束");
		return;
	}

	extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
	if ((!_target->IsAlive())||LevelUtil_CheckDead(_target))
	{
		_Finish();
		_OutputOk(outputs,1,"结束");
		return;
	}


	LevelPos posTarget=_target->GetFramePos();
	if (!posTarget.equals(_posTargetRecent))
	{
		LevelPos posCur=lo->GetFramePos();
		_radius=posCur.getDistanceFrom(posTarget);
		_posTargetRecent=posTarget;
	}

	if (!_Step(FALSE))
	{
		_Finish();

		_OutputOk(outputs,2,"中断");
		return;
	}
}


void CBgn_Siege::Break(BGNOutputs &outputs)
{
	_Finish();
}


