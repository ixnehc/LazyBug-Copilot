/********************************************************************
	created:	2016/12/31 
	author:		cxi
	
	purpose:	 发送信号
*********************************************************************/
#include "stdh.h"

#include "LevelObj.h"
#include "Level.h"
#include "LevelBGs.h"
#include "LevelBehavior.h"
#include "LevelOSB.h"


#include "BgnDeal.h"

BIND_BGN_CLASS(CBgn_Deal,CBgp_Deal);
void CBgn_Deal::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_Deal*pad=_GetPad<CBgp_Deal>();

	CLevelObj *lo=_GetLo();
	if (lo)
	{
		DealArg arg;
		arg.link.id=_GetLevel()->GenOpLinkID();
		arg.grd=0;

		MakeDeals(pad->deals,LevelOSB(lo),lo,arg,NULL);
	}

	_OutputOk(outputs,1,"结束");
}

