/********************************************************************
	created:	2022/7/14 
	author:		cxi
*********************************************************************/

#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordUpgrade.h"

#include "LevelOSB.h"

#include "Bgn_Disciple_RollAwards.h"

#include "LevelObj.h"
#include "LevelBGs.h"



////////////////////////////////////////////////////////////////////////
//CBgn_Disciple_RollAwards
BIND_BGN_CLASS(CBgn_Disciple_RollAwards,CBgp_Disciple_RollAwards);

void CBgn_Disciple_RollAwards::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Disciple_RollAwards*pad=_GetPad<CBgp_Disciple_RollAwards>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	CLevelPlayer *player=_GetTalkPlayer();

	LevelBehaviorContext *ctx=_GetCtx();

	if ((pad->awards!=StringID_Invalid)&&(pad->prices!=StringID_Invalid))
	{
		//清空results
		_GetMem()->DepositObj(pad->awards,NULL);
		_GetMem()->DepositObj(pad->prices,NULL);

		DiscipleParam *param=NULL;
		if (pad->nmParamVar!=StringID_Invalid)
		{
			BMO_DiscipleData *data=_GetMem()->GetObj<BMO_DiscipleData>(pad->nmParamVar);
			param=&data->param;
		}

		if (param)
		{
			RollAwardsResult *result=Class_New(RollAwardsResult);
			RollAwardsPrice *price=Class_New(RollAwardsPrice);

			if (TRUE)
			{
				LevelAward award;
				award.tp=LevelAward::Upgrade;
				award.idRec=param->idSpell;
				award.bValid=1;
				award.bExpendable=0;

				LevelAwardPrice priceAward;
				priceAward.tpRes=LevelResource_Crystal;
				priceAward.count=param->price;
				
				result->awards.push_back(award);
				price->prices.push_back(priceAward);
			}

			if (TRUE)
			{
				LevelAward award;
				award.tp=LevelAward::Resource;
				award.tpRes=LevelResource_Crystal;
				award.count=param->nCrystal;
				award.bExpendable=0;
				award.bValid=1;

				LevelAwardPrice priceAward;

				result->awards.push_back(award);
				price->prices.push_back(priceAward);
			}

			if (player)
			{
				result->UpdateExpendable(player);
				price->UpdateAffordable(player);
			}

			_GetMem()->DepositObj(pad->awards,result);
			_GetMem()->DepositObj(pad->prices,price);

			_OutputOk(outputs,1,"成功");

		}
	}

	_OutputFail(outputs,2,"失败");
}
