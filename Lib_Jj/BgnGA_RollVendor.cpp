/********************************************************************
	created:	2017/01/03 
	author:		cxi
	
	purpose:	随机产生Vender的位置
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelOSB.h"

#include "BgnGA_RollVendor.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoUnit.h"
#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

////////////////////////////////////////////////////////////////////////
//CBgnGA_RollVendor
BIND_BGN_CLASS(CBgnGA_RollVendor,CBgpGA_RollVendor);

void CBgnGA_RollVendor::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_RollVendor*pad=_GetPad<CBgpGA_RollVendor>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	LevelBehaviorContext *ctx=_GetCtx();

	int iDayVendor=-1;
	LevelPlayerStates *lps=NULL;
	CLoUnit *loUnit=NULL;
	CLevelPlayer *player=_GetTalkPlayer();
	if (player)
	{
		lps=player->GetLPS();
		loUnit=player->GetLoUnit();
	}

	if (lps)
	{
		iDayVendor=lps->misc.iDayVendor;

		RollVendorParam *param=&pad->param;
		if (param->idVendor!=RecordID_Invalid)
		{
			if (loUnit)
			{
				if (lps->base.iDay>iDayVendor)
				{//今天还没有出现过
					if (param)
					{
						if (param->sites.size()>0)
						{
							int nCandi=param->nCandidates;
							if (nCandi>param->sites.size())
								nCandi=param->sites.size();

							extern int GenPrimeStep(int seed);

							DWORD seed=level->GetWorld()->GetUniqueID()+lo->GetGUID()+lps->base.iDay;
							int step=GenPrimeStep(seed);

							LevelPos posPlayer;
							posPlayer=loUnit->GetFramePos();

							int idx=step%param->sites.size();
							for (int i=0;i<nCandi;i++)
							{
								i_math::matrix43f &mat=param->sites[idx];
								LevelPos pos=mat.getTranslationP()->getXZ();

								if (pos.getDistanceSQFrom(posPlayer)<param->radiusDetect*param->radiusDetect)
								{
									if (TRUE)
									{
										CLoGeneralAgent* loAgent=(CLoGeneralAgent*)level->CreateObj(Class_Ptr2(CLoGeneralAgent));

										loAgent->PostCreate(mat,param->idVendor,lo->GetPlayerID());

										level->AddToActives(loAgent);

										SAFE_RELEASE(loAgent);
									}

									lps->misc.iDayVendor=(BYTE)lps->base.iDay;//标记为今天出现过了
									lps->misc.SetDirtyDB_High();

									_OutputOk(outputs,1,"成功");

									return;
								}
							}
						}
					}
				}
			}
		}
	}

	_OutputFail(outputs,2,"失败");
	return;
}
