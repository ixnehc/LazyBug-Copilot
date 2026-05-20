/********************************************************************
	created:	2022/6/15 
	author: chenxi
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnGA_圣旗_Assign.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelUtil.h"


////////////////////////////////////////////////////////////////////////
//CBgnGA_圣旗_Assign
BIND_BGN_CLASS(CBgnGA_圣旗_Assign,CBgpGA_圣旗_Assign);


void CBgnGA_圣旗_Assign::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_圣旗_Assign*pad=_GetPad<CBgpGA_圣旗_Assign>();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	CLevelObj *loMe=NULL,*loTarget=NULL;
	CBehaviorMem *memMe=NULL, *memTarget=NULL;
	if (level)
	{
		if (pad->nmLo!=StringID_Invalid)
		{
			LevelObjID idLo;
			if (_GetID(pad->nmLo,BehaviorMemType_ObjID,idLo))
			{
				loTarget=LevelUtil_GetAliveLo(level,idLo);
				if (loTarget)
				{
					CLevelBehavior *behavior=loTarget->GetBehaviorAI();
					if (behavior)
						memTarget=behavior->GetMem(0);
				}
			}
		}
	}
	loMe=_GetLo();
	if (loMe)
	{
		CLevelBehavior *behavior=ctx->behavior;
		if (behavior)
			memMe=behavior->GetMem(0);
	}

	extern void LevelUtil_RemoveBuffByRecordID(CLevelObj *lo,RecordID idBuff);

	if (loMe&&loTarget&&memMe&&memTarget)
	{
		RecordID idBuffMe=RecordID_Invalid;
		RecordID idBuffTarget=RecordID_Invalid;
		if (pad->nmBuffVar!=StringID_Invalid)
		{
			memMe->GetID(pad->nmBuffVar,BehaviorMemType_BuffRecord,idBuffMe);
			memTarget->GetID(pad->nmBuffVar,BehaviorMemType_BuffRecord,idBuffTarget);
		}

		if (idBuffMe!=RecordID_Invalid)
			LevelUtil_RemoveBuffByRecordID(loMe,idBuffMe);
		if (idBuffTarget!=RecordID_Invalid)
			LevelUtil_RemoveBuffByRecordID(loTarget,idBuffTarget);

		if (idBuffMe!=RecordID_Invalid)
			level->GetDecider()->MakeBuff(loTarget,idBuffMe,ANIMTICK_INFINITE,NULL,TRUE);
		if (idBuffTarget!=RecordID_Invalid)
			level->GetDecider()->MakeBuff(loMe,idBuffTarget,ANIMTICK_INFINITE,NULL,TRUE);

		if (pad->nmBuffVar!=StringID_Invalid)
		{
			memMe->SetID(pad->nmBuffVar,BehaviorMemType_BuffRecord,idBuffTarget);
			memTarget->SetID(pad->nmBuffVar,BehaviorMemType_BuffRecord,idBuffMe);
		}
	}



	_OutputOk(outputs,1,"结束");
	return;
}
