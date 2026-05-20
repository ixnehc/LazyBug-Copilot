
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoLink.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


BIND_EOPARAM(EoLink,EoParamLink);

void EoLink::_OnPostCreate()
{
	EoParamLink *param=GetParam<EoParamLink>();
	_idTarget=_idHost;
	if (param->tpTarget==1)
	{
		LevelSkillTarget *target=_GetSkillTarget();
		if (target)
			_idTarget=target->ObjID();
	}

	_idSkill=_GetRootSkillID();

	if(param->nmFinishEvent!=StringID_Invalid)
		_level->RegisterSubframeUpdate(this);

}


void EoLink::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	bp->Data_WriteSimple(_owner.GetOwnerID());
	bp->Data_WriteSimple(_idTarget);
	bContent=TRUE;
}


void EoLink::_OnUpdate()
{
	EoParamLink *param=GetParam<EoParamLink>();
	if (!param)
		return;

	AnimTick t=_GetT();
	AnimTick tAge=ANIMTICK_SAFE_MINUS(t,_tCreate);

	CLevelObj *loTarget=LevelUtil_GetAliveLo(_level,_idTarget);
	BOOL bNeedFinish=FALSE;
	if ((tAge>ANIMTICK_FROM_SECOND(param->dur))&&(param->dur>0.0f))
		bNeedFinish=TRUE;
	else
	{
		if (!loTarget)
			bNeedFinish=TRUE;
		else
		{
			if (LevelUtil_CheckDead(loTarget))
				bNeedFinish=TRUE;
		}
		if (!bNeedFinish)
		{
			if (LevelUtil_CheckDead(_owner.GetOwner()))
				bNeedFinish=TRUE;
		}
	}

	if (!bNeedFinish)
	{
		if (param->nmFinishEvent!=StringID_Invalid)
		{
			if (_CheckSkillCastingEvent(param->nmFinishEvent))
				bNeedFinish=TRUE;
		}
	}
	if (!bNeedFinish)
	{
		if (param->bSkillBound)
		{
			if ((_idSkill!=LevelSkillID_Invalid)&&(_idSkill!=_GetRootSkillID()))
				bNeedFinish=TRUE;
		}
	}

	if (bNeedFinish)
	{
		DeferDestroy();
	}
	else
	{
		DealArg arg;
		arg.dir.set(0,0,0);
		arg.link.id=GetLevel()->GenOpLinkID();
		arg.link.t=_GetAge();
		extern LevelGrade LevelUtil_GetGrade(CLevelObj *lo);
		arg.grd=LevelUtil_GetGrade(this);
		arg.amount=0;

		int nCycle=tAge/ANIMTICK_FROM_SECOND(param->cycleDeal);
		while(nCycle>_nCycles)
		{
			if (loTarget)
				_MakeDeals(loTarget,arg);
			_nCycles++;
		}
	}
}


void EoLink::UpdateSubframe()
{
	_OnUpdate();
}
