/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"
#include "LevelBGs.h"

#include "LevelBasis.h"
#include "LevelBehavior.h"
#include "LevelObj.h"
#include "BgnMoveAlong.h"

#include "LevelSkillDriver.h"
////////////////////////////////////////////////////////////////////////
//CBgn_MoveAlong


BIND_BGN_CLASS(CBgn_MoveAlong,CBgp_MoveAlong);
void CBgn_MoveAlong::_UpdateMove(CLevelObj *lo)
{
	if (!_route)
		return;
	CLevelSkillDriver *driver=lo->GetSkillDriver();
	if (!driver)
		return;

	BOOL bNeedStop=_IsTalkActive();

	if (bNeedStop)
		driver->StopMove();
	else
	{
		if (_iCurPos<_route->nodes.size())
		{
			LevelPos pos=lo->GetFramePos();
			if (pos.getDistanceSQFrom(_route->nodes[_iCurPos])<4.0f)
				_iCurPos++;
		}
		if (_iCurPos<_route->nodes.size())
		{
			LevelSkillTarget target;
			target.SetPos(_route->nodes[_iCurPos]);
			driver->StartFollow(target);
		}
	}

}


void CBgn_MoveAlong::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_MoveAlong*pad=_GetPad<CBgp_MoveAlong>();
	if (pad)
	{
		_iCurPos=0;
		_route=_GetRoute(pad->_route);
	}
	Update(outputs);
}

void CBgn_MoveAlong::Update(BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();

	CLevelObj *lo=ctx->lo;
	if (!lo)
	{
		_SetResult(A_Fail);
		return;
	}


	if (!_route)
	{
		_SetResult(A_Fail);
		return;
	}

	_UpdateMove(lo);

	if (_iCurPos>=_route->nodes.size())
	{
		_OutputOk(outputs,1,"结束");
	}


}
