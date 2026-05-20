/********************************************************************
	created:	2013/01/25 
	author:		cxi
	
	purpose:	 检查是否有某个Buff
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"
#include "LevelRecords.h"
#include "LevelRecordBuff.h"

#include "BgnCheckStunInfo.h"

//////////////////////////////////////////////////////////////////////////
//CBgp_CheckStunInfo
void CBgp_CheckStunInfo::FillDesc(std::string &s,FillDescAssist *assist)
{
	s="n/a";

	if (_tpCheck==Check_BrokenWeaks)
	{
		static std::string ss;
		s="检测是否以下弱点被击破:\n";
		_weaksBroken.ToString(ss);
		s+=ss;
	}
	if (_tpCheck==Check_StunSrcDist)
	{
		FormatString(s,"检测硬直Src离我的距离在(%.2f米~%.2f米)之间",_distMin,_distMax);
	}
	if (_tpCheck==Check_StrikeFromLeft)
	{
		s="Strike来自左侧";
	}
}


////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckStunInfo,CBgp_CheckStunInfo);

void CBgn_CheckStunInfo::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckStunInfo*pad=_GetPad<CBgp_CheckStunInfo>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();
	CLevelDecider *decider=level->GetDecider();

	CLevelDecider::MakeSkillStunContext *ctxMakeSkillStun=decider->GetSkillStunContext();
	CLevelObj *loStunSrc=NULL;
	if (ctxMakeSkillStun->osbSrc)
		loStunSrc=ctxMakeSkillStun->osbSrc->GetRootOwner();

	if (pad->_tpCheck==CBgp_CheckStunInfo::Check_BrokenWeaks)
	{
		LevelWeaksPack &wkpkCheck=pad->_weaksBroken.Get()->Cur();
		if (wkpkCheck.weaks[ctxMakeSkillStun->catBroken]&ctxMakeSkillStun->weaksBroken)
		{
			_OutputOk(outputs,1,"是");
			return;
		}
	}

	if (pad->_tpCheck==CBgp_CheckStunInfo::Check_StunSrcDist)
	{
		float dist2=loStunSrc->GetFramePos().getDistanceSQFrom(lo->GetFramePos());
		if (dist2>=pad->_distMin*pad->_distMin)
		{
			if (dist2<=pad->_distMax*pad->_distMax)
			{
				_OutputOk(outputs,1,"是");
				return;
			}
		}
	}

	if (pad->_tpCheck==CBgp_CheckStunInfo::Check_StrikeFromLeft)
	{
		if (ctxMakeSkillStun->strike)
		{
			LevelFace faceStrike=ctxMakeSkillStun->strike->GetFace();
			LevelFace faceMe=lo->GetFrameFace();
			if (LevelFaceCalcYaw(faceMe,faceStrike)>0.0f)
			{
				_OutputOk(outputs,1,"是");
				return;
			}
		}
	}


	_OutputFail(outputs,2,"否");
}

