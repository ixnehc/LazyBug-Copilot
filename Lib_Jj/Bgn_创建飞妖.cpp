/********************************************************************
	created:	2019/12/22 
	author:		cxi
	
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "Bgn_创建飞妖.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"



////////////////////////////////////////////////////////////////////////
//CBgn_创建飞妖
BIND_BGN_CLASS(CBgn_创建飞妖,CBgp_创建飞妖);


void CBgn_创建飞妖::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_创建飞妖*pad=_GetPad<CBgp_创建飞妖>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (pad->idUnit!=RecordID_Invalid)
	{
		LevelPos pos=lo->GetFramePos();
		LevelFace face=lo->GetFrameFace();

		CLoUnit* loSummon=(CLoUnit*)level->CreateObj(Class_Ptr2(CLoUnit));

		loSummon->PostCreate((LevelPlayerID)lo->GetPlayerID(),NULL,pad->idUnit,1,NULL,EquipSetPick_None,pos,face);//使用技能的等级
		level->AddToActives(loSummon);

		_idSummoned=loSummon->GetID();

		SAFE_RELEASE(loSummon);
	}

	_tStart=_GetT();

	Update(outputs);

	return;
}

void CBgn_创建飞妖::Update(BGNOutputs &outputs)
{
	CBgp_创建飞妖*pad=_GetPad<CBgp_创建飞妖>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	if (_GetT()>=_tStart+ANIMTICK_FROM_SECOND(0.0f))
	{
		extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
		CLevelObj *loSummon=LevelUtil_GetAliveLo(level,_idSummoned);
		if (loSummon)
		{
			if (pad->varSummoned!=StringID_Invalid)
				_SetID(pad->varSummoned,BehaviorMemType_ObjID,loSummon->GetID());
			if (pad->varSummoner!=StringID_Invalid)
			{
				CLevelBehavior *bhv=loSummon->GetBehaviorAI();
				if (bhv)
				{
					CBehaviorMem *mem=bhv->GetMem(0);
					if (mem)
						mem->SetID(pad->varSummoner,BehaviorMemType_ObjID,lo->GetID());
				}
			}
		}

		_OutputOk(outputs,1,"结束");

	}
}
