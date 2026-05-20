/********************************************************************
	created:	2016/06/10 
	author:		cxi
	
	purpose:	 检查Skill相关
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelRecordItemClass.h"

#include "BgnCheckSkill.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckSkill,CBgp_CheckSkill);

void CBgn_CheckSkill::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckSkill*pad=_GetPad<CBgp_CheckSkill>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=NULL;

	switch(pad->type)
	{
		case 0:
			lo=_GetLo();
			break;
		case 1:
			lo=_GetThreat();
			break;
		case 2:
		{
			if (pad->nmLo!=StringID_Invalid)
			{
				LevelObjID id;
				if (_GetID(pad->nmLo,BehaviorMemType_ObjID,id))
				{
					extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
					lo=LevelUtil_GetAliveLo(level,id);
				}
			}

			break;
		}

	}

	extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
	if (CLevelSkill *skill=LevelUtil_GetCastingSkill(lo))
	{
		if (pad->idSkill!=RecordID_Invalid)
		{
			if (skill->GetRec())
			{
				if (pad->idSkill==skill->GetRec()->GetID())
				{
					_OutputOk(outputs,1,"是");
					return;
				}
			}
		}
		else
		{
			_OutputOk(outputs,1,"是");
			return;
		}
	}
	_OutputFail(outputs,2,"否");
}

