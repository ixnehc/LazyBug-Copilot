/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelBehavior.h"
#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelTroops.h"
#include "BgnTroop_Combat.h"

#include "LevelSkillDriver.h"
#include "LoUnit.h"



#include "LevelUtil.h"


////////////////////////////////////////////////////////////////////////
//CBgnTroop_Combat
BIND_BGN_CLASS(CBgnTroop_Combat,CBgpTroop_Combat);

void CBgnTroop_Combat::Destroy()
{
	_DiscardTroopControl();
}

void CBgnTroop_Combat::_OccupyTroopControl()
{
	CBgpTroop_Combat*pad=_GetPad<CBgpTroop_Combat>();
	CLevelTroop *troop=_GetTroop(pad-> _troop);
	if (troop)
	{
		//接管所有troop单位的控制权

		StringID idCmd=StringID_Invalid;
		if (pad->_idCmd==StringID_Invalid)
			idCmd=LevelAIContext::GetStdCmd_Combat();
		else
			idCmd=pad->_idCmd;

		troop->SetCmdToUnits(pad->_flagsRank,idCmd);

		_idCmd=idCmd;
	}
}

void CBgnTroop_Combat::_DiscardTroopControl()
{
	if (_idCmd!=StringID_Invalid)
	{
		CBgpTroop_Combat*pad=_GetPad<CBgpTroop_Combat>();
		CLevelTroop *troop=_GetTroop(pad->_troop);
		if (troop)
		{
			troop->DiscardCmdFromUnits(pad->_flagsRank,_idCmd);
		}
		_idCmd=StringID_Invalid;
	}
}



void CBgnTroop_Combat::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_Combat*pad=_GetPad<CBgpTroop_Combat>();
	CLevel *level=_GetLevel();
	LevelBehaviorContext *ctx=_GetCtx();

	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (troop)
	{
		//接管所有troop单位的控制权
		_OccupyTroopControl();

		_UpdateTcc(troop);

		return;
	}
	_OutputFail(outputs,2,"失败");
}


void CBgnTroop_Combat::Update(BGNOutputs &outputs)
{
	CBgpTroop_Combat*pad=_GetPad<CBgpTroop_Combat>();

	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (!troop)
	{
		_OutputFail(outputs,2,"失败");
		return;
	}

	troop->SetCmdToUnits(pad->_flagsRank,_idCmd);
	_UpdateTcc(troop);

// 	TroopCombatContext *tcc=troop->GetCombatContext();
// 	if (tcc->detects.empty())
// 	{
// 		_OutputFail(outputs,2,"失败");
// 		return;
// 	}

}

void CBgnTroop_Combat::_UpdateTcc(CLevelTroop *troop)
{
	CBgpTroop_Combat*pad=_GetPad<CBgpTroop_Combat>();
	CLevel *level=_GetLevel();

	TroopCombatContext *tcc=troop->GetCombatContext();
	tcc->Clear();

	if (!level)
		return;

	static std::vector<LevelPos> centers;
	centers.clear();

	const float radiusCluster=4.0f;

	LevelUtilDetectParam param;
	param.toIgnores=NULL;
	param.nIgnores=0;
	param.flags=&pad->_flags[0];
	param.nFlags=pad->_flags.size();
	param.requires=&pad->_requires[0];
	param.nRequires=pad->_requires.size();
	param.rangeMin=0.0f;
	param.rangeMax=pad->_range+radiusCluster;

	std::deque<LevelTroopFrame>::iterator it;
	DWORD nFrames=troop->GetFrameCount();
	for (int i=0;i<nFrames;i++)
	{
		LevelTroopFrame *frm=troop->GetFrame(i);
		if (!frm)
			continue;
		if (frm->idUnit!=LevelObjID_Invalid)
		{
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *lo=LevelUtil_GetAliveLo(level,frm->idUnit);
			if (lo)
			{
				if (!LevelUtil_CheckDead(lo))
				{
					LevelPos pos=lo->GetFramePos();

					int j=0;
					for (j=0;j<centers.size();j++)
					{
						if (pos.getDistanceSQFrom(centers[j])<radiusCluster*radiusCluster)
						{
							break;
						}
					}

					if (j>=centers.size())
					{
						centers.push_back(pos);

						param.loSrc=lo;
						param.pos=pos;

						extern CLevelObj **LevelUtil_Detect(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,DWORD &c);

						DWORD c;
						CLevelObj **buf=LevelUtil_Detect(param,NULL,c);

						for (int i=0;i<c;i++)
						{
							CLevelObj *lo=buf[i];
							if (lo->IsEnum())
								continue;
							SAFE_ADDREF(lo);
							tcc->detects.push_back(lo);
							lo->SetEnum(TRUE);
						}
					}
				}
			}
		}
	}

	//Clear all the enum flags for the detects
	if (TRUE)
	{
		for (int i=0;i<tcc->detects.size();i++)
		{
			tcc->detects[i]->SetEnum(FALSE);
		}
	}

}
