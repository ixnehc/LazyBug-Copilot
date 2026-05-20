/********************************************************************
	created:	2023/2/13 

	purpose:	更新索尔的心情
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "BgnGA_RollAwards.h"

#include "BgnGA_Vendor.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"
#include "Random/Random.h"

#define Vendor_Base 5000
#define Vendor_DeltaOnRefreshList 1000
#define Vendor_PerHonor 100
#define Vendor_LevelCount 4 

 
////////////////////////////////////////////////////////////////////////
//CBgnGA_Vendor
BIND_BGN_CLASS(CBgnGA_Vendor,CBgpGA_Vendor);

void CBgnGA_Vendor::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpGA_Vendor*pad=_GetPad<CBgpGA_Vendor>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	CLevelPlayer *player=_GetTalkPlayer();
	if (!player)
	{
		extern CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level);
		player=LevelUtil_GetFirstPlayer(level);
	}

	LevelBehaviorContext *ctx=_GetCtx();

	LevelPlayerStates *lps=NULL;
	if (player)
		lps=player->GetLPS();

	if (lps)
	{
		extern WORD LevelUtil_GetHonor(CLevelObj *lo);
		int honor=LevelUtil_GetHonor((CLevelObj*)player->GetLoUnit());
		short moodMax=(short)(Vendor_Base+Vendor_PerHonor*honor);
		short moodMin=Vendor_DeltaOnRefreshList;

		RollAwardsPrice *priceAward=_GetMem()->GetObj<RollAwardsPrice>(pad->price);
		short mood;
		mood=lps->misc.moodVendor;

		switch(pad->op)
		{
			case 0://初始化
			{
				if (lps->base.iDay>lps->misc.iDayVendor)
				{
					mood=moodMax;
					lps->misc.iDayVendor=(BYTE)lps->base.iDay;
					lps->misc.moodVendor=mood;
					lps->misc.SetDirtyDB_Urgent();
				}
				break;
			}
			case 1://玩家请求刷新商品列表
			{
				mood-=Vendor_DeltaOnRefreshList;
				if (mood<0)
					mood=0;
				lps->misc.moodVendor=mood;
				lps->misc.SetDirtyDB_Urgent();
				break;
			}
			case 2://玩家购买商品
			{
				if (priceAward)
				{
					if (pad->idxPrice<priceAward->prices.size())
					{
						mood+=(short)((1.0f+0.01f*(float)honor)*(float)priceAward->prices[pad->idxPrice].count);
						if (mood>moodMax)
							mood=moodMax;
						lps->misc.moodVendor=mood;
						lps->misc.SetDirtyDB_Urgent();
					}
				}
				break;
			}
			case 3://计算等级
			{
				int lvl=0;
				if (mood>=moodMin)
				{
					int gap=(moodMax-moodMin)/(Vendor_LevelCount-1);
					lvl=1+(mood-moodMin)/gap;
					if (lvl>Vendor_LevelCount-1)
						lvl=Vendor_LevelCount-1;
				}

				_SetNumber(pad->lvlMood,lvl);
				
				break;
			}
		}
	}

	_OutputOk(outputs,1,"结束");
	return;
}
