/********************************************************************
	created:	2015/05/01 
	author:		cxi
	
	purpose:	 得到当前天数
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObj.h"
#include "LevelBehavior.h"

#include "LevelPlayer.h"

#include "BgnGetDay.h"

////////////////////////////////////////////////////////////////////////
//CBgn_GetDay

BIND_BGN_CLASS(CBgn_GetDay,CBgp_GetDay);

void CBgn_GetDay::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_GetDay*pad=_GetPad<CBgp_GetDay>();

	if (pad->var!=StringID_Invalid)
	{
		CLevelPlayer *player=_GetTalkPlayer();

		if (player)
		{
			if (player->GetLPS())
			{
				if (_SetNumber(pad->var,(short)player->GetLPS()->base.iDay))
				{
					_OutputOk(outputs,1,"成功");
					return;
				}
			}
		}
	}

	_OutputFail(outputs,2,"失败");
}


