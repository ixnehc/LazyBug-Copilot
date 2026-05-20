/********************************************************************
	created:	2016/09/04 
	author:		cxi
	
	purpose:	 检测Path是否在navmesh内
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelBGs.h"
#include "LevelResources.h"
#include "LevelRecords.h"

#include "LevelObj.h"

#include "Skill_General.h"

#include "BgnThreat_CheckCast.h"

////////////////////////////////////////////////////////////////////////
//CBgnThreat_CheckCast
BIND_BGN_CLASS(CBgnThreat_CheckCast,CBgpThreat_CheckCast);
void CBgnThreat_CheckCast::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpThreat_CheckCast*pad=_GetPad<CBgpThreat_CheckCast>();

	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();
	LevelRecordSkill *recSkill=ctx->level->GetRecords()->GetSkill(pad->idSkill);
	if (!recSkill)
	{
		_OutputFail(outputs,2,"失败");
		return;
	}

	if (_Check(pad))
	{
		_OutputOk(outputs,1,"成功");
		return;
	}

	if (pad->mode==CBgpThreat_CheckCast::Mode_Check)
	{
		_OutputFail(outputs,2,"失败");
		return;
	}

	_tStart=_GetT();
}

BOOL CBgnThreat_CheckCast::_Check(CBgpThreat_CheckCast*pad)
{

	CLevelObj *target=_GetThreat();
	if (!target)
		return FALSE;

	CLevelSkillDriver *driver=_GetLo()->GetSkillDriver();
	if (driver)
	{
		LevelSkillTarget tgt;
		tgt.SetObjID(target->GetID());
		if (driver->CheckStartCast(LevelSkillType(pad->idSkill),tgt,pad->bIgnoreFaceCheck))
			return TRUE;
	}

	return FALSE;
}

void CBgnThreat_CheckCast::Update(BGNOutputs &outputs)
{
	CBgpThreat_CheckCast*pad=_GetPad<CBgpThreat_CheckCast>();

	if (_Check(pad))
	{
		_OutputOk(outputs,1,"成功");
		return;
	}

	if (pad->dur>0)
	{
		if (_tStart+pad->dur<_GetT())
		{
			_OutputFail(outputs,2,"失败");
			return;
		}
	}
}
