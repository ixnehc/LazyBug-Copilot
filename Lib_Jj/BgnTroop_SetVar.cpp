/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:设置Troop内单位的变量
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnTroop_SetVar.h"

#include "LevelObj.h"
#include "LevelBGs.h"
#include "LevelTroops.h"

#include "LoGeneralAgent.h"

#include "LoUnit.h"



#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgnTroop_SetVar
BIND_BGN_CLASS(CBgnTroop_SetVar,CBgpTroop_SetVar);

void CBgnTroop_SetVar::_SetVar(SetVarInfo &info)
{
	if (info.nmVar!=StringID_Invalid)
	{
		CLevel *level=_GetLevel();
		if (level)
		{
			if (info.idUnit!=LevelObjID_Invalid)
			{
				extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
				CLevelObj *lo=LevelUtil_GetAliveLo(level,info.idUnit);
				if (lo)
				{
					CLevelBehavior *bhv=lo->GetBehaviorAI();
					if (bhv)
					{
						CBehaviorMem *mem=bhv->GetMem(0);
						if (mem)
						{
							if (FALSE==mem->SetNumber(info.nmVar,(short)info.value))
							{
								if (FALSE==mem->SetBit(info.nmVar,(BOOL)info.value))
								{
									LOG_DUMP_2P("CBgnTroop_SetVar",Log_Error,"无法在行为图\"%s\"中找到名为\"%s\"的变量!",StrLib_GetStr(bhv->GetName()),info.nmVar);
								}
							}
						}
					}
				}
			}
		}
	}
}



void CBgnTroop_SetVar::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_SetVar*pad=_GetPad<CBgpTroop_SetVar>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	BP_SetTroopVar *param=&pad->_param;
	CLevelTroop *troop=_GetTroop(pad->_troop);

	LevelRecordBuff *recBuff=level->GetRecords()->GetBuff(_idBuff);

	BOOL bInstantly=TRUE;
	if (param)
	{
		LevelBehaviorContext *ctx=_GetCtx();

		if (troop)
		{
			if (param->speed>0.0f)
				bInstantly=FALSE;

			SetVarInfo info;

			DWORD nFrames=troop->GetFrameCount();
			for (int i=0;i<nFrames;i++)
			{
				LevelTroopFrame *frm=troop->GetFrame(i);
				extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
				CLevelObj *lo=LevelUtil_GetAliveLo(level,frm->idUnit);
				if (!lo)
					continue;

				extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
				if (LevelUtil_CheckDead(lo))
					continue;

				info.idUnit=frm->idUnit;
				info.nmVar=param->nmVar;
				info.value=param->value;

				if (bInstantly)
					_SetVar(info);
				else
					_infos.push_back(info);
			}
		}
	}

	if ((bInstantly)||(_infos.size()<=0))
	{
		_OutputOk(outputs,1,"结束");
		return;
	}

	CSysRandom::GenRandomIndices(_indices,_infos.size());

	_tStart=_GetT();

	Update(outputs);
	return;
}

void CBgnTroop_SetVar::Update(BGNOutputs &outputs)
{
	CBgpTroop_SetVar*pad=_GetPad<CBgpTroop_SetVar>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();
	BP_SetTroopVar *param=&pad->_param;

	AnimTick tCur=_GetT();
	tCur=ANIMTICK_SAFE_MINUS(tCur,_tStart);

	DWORD nToSpawn=0;
	if (param->speed<=0.0f)
		nToSpawn=100000;
	else
		nToSpawn=(DWORD)(param->speed*ANIMTICK_TO_SECOND(tCur));

	if (nToSpawn>_indices.size())
		nToSpawn=_indices.size();

	for (int i=_nMade;i<nToSpawn;i++)
		_SetVar(_infos[_indices[i]]);

	_nMade=nToSpawn;

	if (_nMade>=_indices.size())
	{
		_OutputOk(outputs,1,"结束");
	}
	else
		_SetResult(A_Pending);
}


