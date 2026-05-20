/********************************************************************
	created:	2019/10/0 1
	author:		cxi
	
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "BgnChainedHammerOp.h"

#include "EoChainedHammer.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_ChainedHammerOp,CBgp_ChainedHammerOp);

void CBgn_ChainedHammerOp::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_ChainedHammerOp*pad=_GetPad<CBgp_ChainedHammerOp>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	EoChainedHammer *eoHammer=NULL;

	if (TRUE)
	{
		LevelObjID idHammer=LevelObjID_Invalid;
		if (pad->_nmVar!=StringID_Invalid)
			_GetID(pad->_nmVar,BehaviorMemType_ObjID,idHammer);
		if (idHammer!=LevelObjID_Invalid)
		{
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *lo=LevelUtil_GetAliveLo(level,idHammer);
			if (lo->GetClass()->IsSameWith(Class_Ptr2(EoChainedHammer)))
				eoHammer=(EoChainedHammer*)lo;
		}
	}

	if (pad->_op==CBgp_ChainedHammerOp::Op_CheckInHand)
	{
		if (eoHammer)
			_OutputFail(outputs,2,"NotOk");
		else
			_OutputOk(outputs,1,"Ok");
		return;
	}

	if (!eoHammer)
	{
		_OutputFail(outputs,2,"NotOk");
		return;
	}

	switch(pad->_op)
	{
		case CBgp_ChainedHammerOp::Op_Withdraw:
		{
			if (eoHammer->Withdraw())
				_OutputOk(outputs,1,"Ok");
			else
				_OutputFail(outputs,2,"NotOk");
			return;
		}
		case CBgp_ChainedHammerOp::Op_Pull:
		{
			if (eoHammer->Pull())
				_OutputOk(outputs,1,"Ok");
			else
				_OutputFail(outputs,2,"NotOk");
			return;
		}
		case CBgp_ChainedHammerOp::Op_CheckWithdrawn:
		{
			if (eoHammer->IsWithdrawn())
				_OutputOk(outputs,1,"Ok");
			else
				_OutputFail(outputs,2,"NotOk");
			return;
		}
		case CBgp_ChainedHammerOp::Op_CheckCanWithdraw:
		{
			if (_CheckCanWithdraw(eoHammer))
				_OutputOk(outputs,1,"Ok");
			else
				_OutputFail(outputs,2,"NotOk");
			return;
		}
		case CBgp_ChainedHammerOp::Op_CheckStuck:
		{
			if (eoHammer->IsStuck())
				_OutputOk(outputs,1,"Ok");
			else
				_OutputFail(outputs,2,"NotOk");
			return;
		}
		case CBgp_ChainedHammerOp::Op_CheckDetached:
		{
			if (eoHammer->IsDetached())
				_OutputOk(outputs,1,"Ok");
			else
				_OutputFail(outputs,2,"NotOk");
			return;
		}
		case CBgp_ChainedHammerOp::Op_Grab:
		{
			if (eoHammer->Grab())
				_OutputOk(outputs,1,"Ok");
			else
				_OutputFail(outputs,2,"NotOk");
			return;
		}
		case CBgp_ChainedHammerOp::Op_Break:
		{
			if (eoHammer->Break())
				_OutputOk(outputs,1,"Ok");
			else
				_OutputFail(outputs,2,"NotOk");
			return;
		}
	}

	_OutputFail(outputs,2,"NotOk");
}

BOOL CBgn_ChainedHammerOp::_CheckCanWithdraw(EoChainedHammer *eoHammer)
{
	EoChainedHammer::Stage stage=eoHammer->GetStage();
	if (stage==EoChainedHammer::Stage_Hit)
		return TRUE;
//	return FALSE;
	if (stage==EoChainedHammer::Stage_Throwing)
	{
		LevelPos3D posHammer=eoHammer->GetFramePos3D();

		if (posHammer.getDistanceXZFromSQ(eoHammer->GetSrcPos())>2.5f*2.5f)
		{//离起始位置足够远
			if (posHammer.getDistanceXZFromSQ(eoHammer->GetTargetPos())>1.5f*1.5f)
			{//离目标点足够远
				extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
				CLevelObj *loThreat=LevelUtil_GetAliveLo(_GetLevel(),eoHammer->GetThreatID());
				if (!loThreat)
					return TRUE;
				extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
				if (LevelUtil_CheckDead(loThreat))
					return TRUE;

				LevelPos posThreat=loThreat->GetFramePos();

				LevelPos dir=eoHammer->GetTargetPos().getXZ()-eoHammer->GetSrcPos().getXZ();
				dir.normalize();
				float d=(posThreat-eoHammer->GetSrcPos().getXZ()).dotProduct(dir);
				float distHammer=posHammer.getDistanceXZFrom(eoHammer->GetSrcPos());
				if (d<distHammer-0.5f)
					return TRUE;//Hammer已经在threat身后了,不可能打到它了
// 				float distThreat=posThreat.getDistanceFrom(eoHammer->GetSrcPos().getXZ());
// 				float distOff2=distThreat*distThreat-d*d;
// 				if (distOff2>0.5f*0.5f)
// 					return TRUE;//已偏离Hammer方向太远
			}
		}
	}

	return FALSE;


}
