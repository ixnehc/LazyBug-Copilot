/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"


#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LevelEventSrc.h"

#include "BgnWaitStun.h"

////////////////////////////////////////////////////////////////////////
//CBgn_WaitStun

BIND_BGN_CLASS(CBgn_WaitStun,CBgp_WaitStun);
void CBgn_WaitStun::Start(DWORD iStb,BGNOutputs &outputs)
{
	if (!_CheckStun())
	{
		_OutputOk(outputs,1,"成功");
		return;
	}

	CBgp_WaitStun*pad=_GetPad<CBgp_WaitStun>();

	extern void LevelUtil_SetWeaksFilter(WeaksEx &weaks,CLevelBgn *bgn);
	LevelUtil_SetWeaksFilter(pad->_weaksFilter,this);

	_tStart=_GetT();

}

BOOL CBgn_WaitStun::_CheckStun()
{
	CLevelObj *lo=_GetLo();
	CLevelEventSrc *srcEvent=lo->GetEventSrc();
	if (!srcEvent)
		return FALSE;

	if (!srcEvent->ExistStun())
		return FALSE;

	return TRUE;
}

void CBgn_WaitStun::_ClearFilter()
{
	extern void LevelUtil_ClearWeaksFilter(CLevelBgn *bgn);
	LevelUtil_ClearWeaksFilter(this);
}


void CBgn_WaitStun::Update(BGNOutputs &outputs)
{
	if (!_CheckStun())
	{
		_ClearFilter();
		_OutputOk(outputs,1,"成功");
		return;
	}

	CBgp_WaitStun*pad=_GetPad<CBgp_WaitStun>();
	if (pad->_dur>0)
	{
		if (_GetT()>_tStart+pad->_dur)
		{
			_ClearFilter();
			_OutputFail(outputs,2,"失败");
			return;
		}
	}
}

void CBgn_WaitStun::Break(BGNOutputs &outputs)
{
	_ClearFilter();
}

