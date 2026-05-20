/********************************************************************
	created:	2012/12/16 
	author:		cxi
	
	purpose:	驻留
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "BgnReside.h"
#include "LevelRecords.h"
#include "LevelObjResidable.h"


#include "Level.h"


BIND_BGN_CLASS(CBgn_Reside,CBgp_Reside);

void CBgn_Reside::_ClearTarget()
{
	VerifyLevelObjAlive(_target);
	if (_target)
	{
		CLevelObjResidable *seats=_target->GetResidable();
		if (seats)
			seats->Cancel(_token);
	}
	SAFE_RELEASE(_target);
	_token=LevelObjSeatToken_Invalid;
}

BOOL CBgn_Reside::_DoReside()
{
	CBgp_Reside*pad=_GetPad<CBgp_Reside>();

	CLevelObj *lo=_GetLo();
	if (!lo)
		return FALSE;

	extern CLevelObj *LevelUtil_DetectClosestResidable(CLevelObj *lo,float range,CLevelObj *toIgnore,RecordID idAgent);
	CLevelObj *loDetect=LevelUtil_DetectClosestResidable(lo,pad->_radius,_target,pad->_idAgent);
	_ClearTarget();
	LevelObjSeatToken token=LevelObjSeatToken_Invalid;

	if (loDetect)
	{
		CLevelObjResidable *seats=loDetect->GetResidable();
		if (seats)
			token=seats->Preserve();

		if (token==LevelObjSeatToken_Invalid)
		{
			_ClearTarget();
			return FALSE;
		}

		CLevelSkillDriver *driver=lo->GetSkillDriver();
		LevelRecordSkill *recSkill=lo->GetLevel()->GetRecords()->GetSkill(pad->_idSkill);
		if (recSkill)
		{
			LevelSkillTarget target;
			target.SetObjID(loDetect->GetID());
			driver->Start(LevelSkillType(pad->_idSkill),target,FALSE,ClientSkillID_Invalid,LevelSkillGrade_Invalid,NULL);
		}
		SAFE_REPLACE(_target,loDetect);
		_token=token;
	}
	else
		return FALSE;

	return TRUE;
}

void CBgn_Reside::Destroy()
{
	_ClearTarget();
}

void CBgn_Reside::Start(DWORD iStb,BGNOutputs &outputs)
{

	_tStart=_GetT();

	if (!_DoReside())
	{
		_OutputOk(outputs,2,"无法驻留");
		return;
	}
}



void CBgn_Reside::Update(BGNOutputs &outputs)
{
	if (_result!=A_Pending)
		return;

	CLevelObj *lo=_GetLo();
	if (!lo)
		return;

	VerifyLevelObjAlive(_target);

	AnimTick t=_GetT();

	CBgp_Reside*pad=_GetPad<CBgp_Reside>();

	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (driver)
	{
		if (!driver->IsWorking())
		{
			extern BOOL LevelUtil_TestAnyBuff(CLevelObj *lo,DWORD flagBuff);
			if (LevelUtil_TestAnyBuff(lo,BuffFlag_Reside))
			{//进入了
				_ClearTarget();

				_OutputOk(outputs,1,"驻留成功");
				return;
			}

			if (t>_tStart+pad->_tSearching)
			{//时间太长仍没进入,失败
				_ClearTarget();

				_OutputOk(outputs,2,"无法驻留");
				return;
			}

			//继续尝试
			if (!_DoReside())
			{
				_OutputOk(outputs,2,"无法驻留");
				return;
			}
		}
	}
}

void CBgn_Reside::Break(BGNOutputs &outputs)
{
	Destroy();
}
