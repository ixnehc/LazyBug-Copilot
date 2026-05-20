/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelBGs.h"

#include "LevelObj.h"

#include "BgnCheckDist.h"

////////////////////////////////////////////////////////////////////////
//CBgn_CheckDist
BIND_BGN_CLASS(CBgn_CheckDist,CBgp_CheckDist);
void CBgn_CheckDist::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckDist*pad=_GetPad<CBgp_CheckDist>();

	CLevelObj *lo=_GetLo();
	CLevelObj *loTarget=NULL;
	CLevelObj *loSrc=NULL;

	if (pad->tpSource==CBgp_CheckDist::Source_Me)
		loSrc=lo;
	if (pad->tpSource==CBgp_CheckDist::Source_Custom)
	{
		LevelObjID id=LevelObjID_Invalid;
		_GetID(pad->nmVarSrc,BehaviorMemType_ObjID,id);
		extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
		loSrc=LevelUtil_GetAliveLo(_GetLevel(),id);
	}

	if (pad->tpTarget==CBgp_CheckDist::Target_Ground)
	{
		float ht=0.0f;
		CUnit3D *unit3D=loSrc->GetUnit3D();
		if (unit3D)
		{
			LevelPos3D pos=loSrc->GetFramePos3D();
			extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
			LevelPos3D posGround=LevelUtil_GetGroundHeight(_GetLevel(),pos.x,pos.z,TRUE);
			ht=pos.y-posGround.y;
			if (ht<0.0f)
				ht=0.0f;
		}

		if ((ht<pad->radiusMax)&&(ht>pad->radiusMin))
		{
			_OutputOk(outputs,1,"范围内");
			return;
		}
		_OutputFail(outputs,2,"范围外");
		return;
	}

	if (pad->tpTarget==CBgp_CheckDist::Target_Custom)
	{
		LevelObjID id=LevelObjID_Invalid;
		_GetID(pad->nmVar,BehaviorMemType_ObjID,id);
		extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
		loTarget=LevelUtil_GetAliveLo(_GetLevel(),id);
	}
	if (pad->tpTarget==CBgp_CheckDist::Target_TalkPlayer)
		loTarget=_GetTalkLo();
	if (pad->tpTarget==CBgp_CheckDist::Target_LockPlayer)
		loTarget=_GetLockLo();
	if (pad->tpTarget==CBgp_CheckDist::Target_FirstPlayer)
	{
		extern CLevelPlayer *LevelUtil_GetFirstPlayer(CLevel *level);
		CLevelPlayer *player=LevelUtil_GetFirstPlayer(_GetLevel());
		if (player)
			loTarget=(CLevelObj *)player->GetLoUnit();
	}

	if (loSrc&&loTarget)
	{
		float dist2=loSrc->GetFramePos().getDistanceSQFrom(loTarget->GetFramePos());
		if ((dist2<pad->radiusMax*pad->radiusMax)&&(dist2>=pad->radiusMin*pad->radiusMin))
		{
			_OutputOk(outputs,1,"范围内");
			return;
		}
	}

	_OutputFail(outputs,2,"范围外");
}
