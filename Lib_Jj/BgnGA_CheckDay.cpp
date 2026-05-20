/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 CheckDay
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordAgent.h"

#include "BgnGA_CheckDay.h"

#include "LevelObj.h"
#include "LevelObjMove.h"

#include "LevelSkillDriver.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_CheckDay
BIND_BGN_CLASS(CBgnGA_CheckDay,CBgpGA_CheckDay);


void CBgnGA_CheckDay::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_CheckDay*pad=_GetPad<CBgpGA_CheckDay>();

	CBehavior *bhv=_bhv;

	LevelSimpleMem *mem=_GetSimpleMem();

	if (mem)
	{
		CLevelPlayer *player=_GetTalkPlayer();
		if (player)
		{
			if (player->GetLPS())
			{
				if (pad->_bCheck)
				{
					if (mem->iCheckDay!=(BYTE)player->GetLPS()->base.iDay)
					{
						mem->iCheckDay=(BYTE)player->GetLPS()->base.iDay;
						mem->bSyncDirty=1;
						mem->bPersistDirty=1;
					}
				}
				else
				{
					mem->iCheckDay=0;
					mem->bSyncDirty=1;
					mem->bPersistDirty=1;
				}
			}
		}
	}

	_OutputOk(outputs,1,"结束");
}



////////////////////////////////////////////////////////////////////////
//CBgnGA_TestCheckDay
BIND_BGN_CLASS(CBgnGA_TestCheckDay,CBgpGA_TestCheckDay);


void CBgnGA_TestCheckDay::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBehavior *bhv=_bhv;

	LevelSimpleMem *mem=_GetSimpleMem();

	CLevelObjSrc *los=_GetLos();
	if (los)
	{
		LevelRecordAgent *rec=los->GetRecord();
		if (rec)
		{
			BOOL bResetCheckDay=rec->bResetCheckDay;

			if (mem)
			{
				CLevelPlayer *player=_GetTalkPlayer();
				if (player)
				{
					if (player->GetLPS())
					{
						BOOL bYes=FALSE;
						if (!bResetCheckDay)
						{
							if (mem->iCheckDay!=0)
								bYes=TRUE;
						}
						else
						{
							if (mem->iCheckDay==(BYTE)player->GetLPS()->base.iDay)
								bYes=TRUE;	
						}
						if (bYes)
						{
							_OutputOk(outputs,1,"是");
						}
						else
						{
							_OutputFail(outputs,2,"否");
						}
						return;
					}
				}
			}
		}
	}

	_SetResult(A_Fail);
}
