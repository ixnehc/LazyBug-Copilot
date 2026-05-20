/********************************************************************
	created:	2015/08/04
	filename: 	g:\IxEngine\Lib_Jj\BgnMonitorLoc.cpp
	author:		cxi
	
	purpose:	监控区域
*********************************************************************/
#include "stdh.h"
#include "Level.h"
#include "LoUnit.h"

#include "LevelBGs.h"

#include "LevelObj.h"

#include "BgnAreaOp.h"


////////////////////////////////////////////////////////////////////////
//CBgn_MonitorLoc
BIND_BGN_CLASS(CBgn_AreaOp,CBgp_AreaOp);

BOOL CBgn_AreaOp::_Check()
{
	CBgp_AreaOp*pad=_GetPad<CBgp_AreaOp>();
	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	BccArea *bcc=&pad->area;

	switch(pad->op)
	{
		case CBgp_AreaOp::PlayerEnter:
		case CBgp_AreaOp::PlayerLeave:
		{
			DWORD c;
			LevelPlayerID *ids=level->GetPlayerIDs(c);
			for (int i=0;i<c;i++)
			{
				CLevelPlayer *player=level->GetPlayer(ids[i]);
				if (player)
				{
					if (player->GetLoUnit())
					{
						LevelPos pos=player->GetLoUnit()->GetFramePos();

						if (pad->op==CBgp_AreaOp::PlayerEnter)
						{
							if (bcc->CheckIn(pos))
								return TRUE;
						}
						else
						{
							if (!bcc->CheckIn(pos))
								return TRUE;
						}
					}
				}
			}
			break;
		}
	}

	return FALSE;
}

void CBgn_AreaOp::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_AreaOp*pad=_GetPad<CBgp_AreaOp>();

	if (pad->op==CBgp_AreaOp::CheckEmpty)
	{
		if (pad->area.IsEmpty())
			_OutputOk(outputs,1,"是");
		else
			_OutputFail(outputs,2,"否");

		return;
	}

	float dur=pad->dur;
	if (dur==0)
	{
		if (_Check())
			_OutputOk(outputs,1,"是");
		else
			_OutputFail(outputs,2,"否");
		return;
	}

	_tStart=_GetT();
	if (dur>0.0f)
		_dur=ANIMTICK_FROM_SECOND(dur);
	else
		_dur=ANIMTICK_INFINITE;
	Update(outputs);
}

void CBgn_AreaOp::Update(BGNOutputs &outputs)
{
	CBgp_AreaOp*pad=_GetPad<CBgp_AreaOp>();

	if (_Check())
	{
		_OutputOk(outputs,1,"是");
		return;
	}

	if (_dur!=ANIMTICK_INFINITE)
	{
		if (_GetT()>_tStart+_dur)
		{
			_OutputFail(outputs,2,"否");
			return;
		}
	}

}
