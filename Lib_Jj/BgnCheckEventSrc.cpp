/********************************************************************
	created:	2016/05/15 
	author:		cxi
	
	purpose:	 检测EventSrc
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelObj.h"

#include "LevelEventSrc.h"


#include "BgnCheckEventSrc.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckEventSrc

BIND_BGN_CLASS(CBgn_CheckEventSrc,CBgp_CheckEventSrc);

void CBgn_CheckEventSrc::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckEventSrc*pad=_GetPad<CBgp_CheckEventSrc>();

	if (!_Update(outputs))
	{
		if (!pad->bWait)
			_OutputFail(outputs,2,"未检测到");
	}
}

BOOL CBgn_CheckEventSrc::_Update(BGNOutputs &outputs)
{
	CBgp_CheckEventSrc*pad=_GetPad<CBgp_CheckEventSrc>();

	LevelBehaviorContext *ctx=_GetCtx();
	CLevel *level=ctx->level;
	CLevelObj *lo=NULL;

	switch(pad->mode)
	{
		case CBgp_CheckEventSrc::CheckMode_MeFromAnyone:
		case CBgp_CheckEventSrc::CheckMode_MeFromThreat:
			lo=_GetLo();
			break;
		case CBgp_CheckEventSrc::CheckMode_ThreatFromMe:
			lo=_GetThreat();
			break;
		case CBgp_CheckEventSrc::CheckMode_CustomFromCustom:
		case CBgp_CheckEventSrc::CheckMode_CustomFromAnyone:
		case CBgp_CheckEventSrc::CheckMode_CustomFromMe:
		{
			lo=_GetLoFromVar(pad->nmTo);
			break;
		}
	}

	if (!lo)
		return FALSE;

	CLevelEventSrc *src=lo->GetEventSrc();
	if (!src)
		return FALSE;

	AnimTick t=lo->GetT();
	t=ANIMTICK_SAFE_MINUS(t,pad->durGap);

	switch(pad->mode)
	{
		case CBgp_CheckEventSrc::CheckMode_MeFromAnyone:
		case CBgp_CheckEventSrc::CheckMode_CustomFromAnyone:
		{
			LevelObjID id;
			if (src->ExistWithMask(pad->maskType,t,&id))
			{
				if (pad->nmFromResult)
					_SetID(pad->nmFromResult,BehaviorMemType_ObjID,id);
				_OutputOk(outputs,1,"检测到");
				return TRUE;
			}
			break;
		}
		case CBgp_CheckEventSrc::CheckMode_MeFromThreat:
		{
			CLevelObj *threat=_GetThreat();
			if (threat)
			{
				if (src->ExistWithMask(pad->maskType,threat->GetID(),t))
				{
					_OutputOk(outputs,1,"检测到");
					return TRUE;
				}
			}
			break;
		}
		case CBgp_CheckEventSrc::CheckMode_ThreatFromMe:
		{
			if (_GetLo())
			{
				if (src->ExistWithMask(pad->maskType,_GetLo()->GetID(),t))
				{
					_OutputOk(outputs,1,"检测到");
					return TRUE;
				}
			}
			break;
		}
		case CBgp_CheckEventSrc::CheckMode_CustomFromCustom:
		{
			CLevelObj *loFrom=_GetLoFromVar(pad->nmFrom);
			if (loFrom)
			{
				if (src->ExistWithMask(pad->maskType,loFrom->GetID(),t))
				{
					_OutputOk(outputs,1,"检测到");
					return TRUE;
				}
			}
			break;
		}
		case CBgp_CheckEventSrc::CheckMode_CustomFromMe:
		{
			if (_GetLo())
			{
				if (src->ExistWithMask(pad->maskType,_GetLo()->GetID(),t))
				{
					_OutputOk(outputs,1,"检测到");
					return TRUE;
				}
			}
			break;
		}
	}

	return FALSE;
}


void CBgn_CheckEventSrc::Update(BGNOutputs &outputs)
{
	_Update(outputs);
}
