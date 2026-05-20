/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:修改资源
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelOSB.h"

#include "BgnGA_Temple.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_ModTemple
BIND_BGN_CLASS(CBgnGA_ModTemple,CBgpGA_ModTemple);


void CBgnGA_ModTemple::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_ModTemple*pad=_GetPad<CBgpGA_ModTemple>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		CLevelDecider *decider=level->GetDecider();
		if (decider)
		{
			CLevelObj *lo=_GetTalkLo();
			if(lo)
				decider->RepairTemple(LevelOSB(ctx->lo),lo,pad->tp,(DWORD)pad->iAltar);
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}



////////////////////////////////////////////////////////////////////////
//CBgnGA_GetTemple
BIND_BGN_CLASS(CBgnGA_GetTemple,CBgpGA_GetTemple);


void CBgnGA_GetTemple::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_GetTemple*pad=_GetPad<CBgpGA_GetTemple>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (level)
	{
		CLevelDecider *decider=level->GetDecider();
		if (decider)
		{
			CLevelObj *lo=_GetTalkLo();
			if(lo)
			{
				LevelAttr_Temple *attr=lo->GetAttr_Temple();
				if (attr)
				{
					DWORD c=attr->GetCount(pad->tp);
					_SetNumber(pad->var,(short)c);
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}


////////////////////////////////////////////////////////////////////////
//CBgnGA_TempleSwitcher
BIND_BGN_CLASS(CBgnGA_TempleSwitcher,CBgpGA_TempleSwitcher);


void CBgnGA_TempleSwitcher::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_TempleSwitcher*pad=_GetPad<CBgpGA_TempleSwitcher>();

	switch(pad->tp)
	{
		case LevelTemple_Sun:
		{
			_OutputOk(outputs,2,"光之圣殿");
			break;
		}
		case LevelTemple_Moon:
		{
			_OutputOk(outputs,3,"月之圣殿");
			break;
		}
		case LevelTemple_Fire:
		{
			_OutputOk(outputs,4,"火之圣殿");
			break;
		}
		case LevelTemple_Star:
		{
			_OutputOk(outputs,5,"星之圣殿");
			break;
		}
		case LevelTemple_Sand:
		{
			_OutputOk(outputs,6,"沙之圣殿");
			break;
		}
		case LevelTemple_Craft:
		{
			_OutputOk(outputs,7,"工匠圣殿");
			break;
		}
		default:
		{
			_OutputFail(outputs,2,"无效");
		}
	}
}
