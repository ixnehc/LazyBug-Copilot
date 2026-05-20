/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"
#include "LevelBehavior.h"
#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelTroops.h"
#include "BgnTroop_MoveAlong.h"

#include "LevelSkillDriver.h"
#include "LoUnit.h"



#include "LevelUtil.h"


////////////////////////////////////////////////////////////////////////
//CBgnTroop_MoveAlong
BIND_BGN_CLASS(CBgnTroop_MoveAlong,CBgpTroop_MoveAlong);

extern BOOL LevelUtil_CheckDead(CLevelObj *lo);
extern BOOL LevelUtil_IsMoving_(CLevelObj *lo);
extern BOOL LevelUtil_IsMovingOrRotating(CLevelObj *lo);

void CBgnTroop_MoveAlong::_StartMove(Entry &e,BccRoute *bcc,AnimTick t)
{
	if (LevelUtil_CheckDead(e.lo))
		return;

	if (TRUE)
	{
		LevelPos pos=e.lo->GetFramePos();

		int idxClosest=-1;
		float distMin2=10000000000.0f;
		for (int i=0;i<bcc->sphereset.size();i++)
		{
			float dist2=(pos.x-bcc->sphereset[i].center.x)*(pos.x-bcc->sphereset[i].center.x)+
				(pos.y-bcc->sphereset[i].center.z)*(pos.y-bcc->sphereset[i].center.z);

			if (dist2<distMin2)
			{
				distMin2=dist2;
				idxClosest=i;
			}
		}

		e.idxNode=idxClosest;
	}

	if (TRUE)
	{
		LevelPos posTarget;
		posTarget.x=bcc->sphereset[e.idxNode].center.x;
		posTarget.y=bcc->sphereset[e.idxNode].center.z;
		extern BOOL LevelUtil_MoveTo(CLevelObj *lo,LevelPos &pos,float rangeFollow);
		LevelUtil_MoveTo(e.lo,posTarget,-1.0f);
	}

	e.tStartMove=t;

}

void CBgnTroop_MoveAlong::_OccupyTroopControl()
{
	CBgpTroop_MoveAlong*pad=_GetPad<CBgpTroop_MoveAlong>();
	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (troop)
	{
		troop->SetCmdToAllUnits(LevelAIContext::GetStdCmd_Controlled());
	}
}

void CBgnTroop_MoveAlong::_DiscardTroopControl()
{
	CBgpTroop_MoveAlong*pad=_GetPad<CBgpTroop_MoveAlong>();
	CLevelTroop *troop=_GetTroop(pad->_troop);
	if (troop)
	{
		troop->DiscardCmdFromAllUnits(LevelAIContext::GetStdCmd_Controlled());
	}
}

void CBgnTroop_MoveAlong::Destroy()
{
	//清除_entries
	for (int i=0;i<_entries.size();i++)
	{
		SAFE_DESTROY(_entries[i].anSpeed);
		SAFE_RELEASE(_entries[i].lo);
	}
	_entries.clear();

	_DiscardTroopControl();
}



void CBgnTroop_MoveAlong::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgpTroop_MoveAlong*pad=_GetPad<CBgpTroop_MoveAlong>();
	CLevel *level=_GetLevel();
	LevelBehaviorContext *ctx=_GetCtx();

	AnimTick t=_GetT();

	BccRoute *bcc=&pad->_route;
	if (bcc)
	{
		if (bcc->sphereset.size()>0)
		{
			bcc->UpdateDistsToGo();

			CLevelTroop *troop=_GetTroop(pad->_troop);
			if (troop)
			{
				//接管所有troop单位的控制权
				_OccupyTroopControl();

				DWORD nFrames=troop->GetFrameCount();
				_entries.reserve(nFrames);
				for (int i=0;i<nFrames;i++)
				{
					LevelTroopFrame *frm=troop->GetFrame(i);
					CLevelObj *lo=level->GetIDs()->LoFromID(frm->idUnit);
					if (lo)
					{
						if (lo->IsAlive())
						{
							Entry e;
							e.lo=lo;
							SAFE_ADDREF(e.lo);

							_StartMove(e,bcc,t);

							_entries.push_back(e);
						}
					}
				}
				return;
			}
		}
	}

	_OutputFail(outputs,2,"失败");
}

BOOL CBgnTroop_MoveAlong::_CanControl(CBgnTroop_MoveAlong::Entry &e)
{
	if (!e.lo)
		return FALSE;
	if (!e.lo->IsAlive())
		return FALSE;

	if (LevelUtil_CheckDead(e.lo))
		return FALSE;

	if (e.lo->GetAICmd()!=LevelAIContext::GetStdCmd_Controlled())
		return FALSE;

	return TRUE;
}


void CBgnTroop_MoveAlong::Update(BGNOutputs &outputs)
{
	CBgpTroop_MoveAlong*pad=_GetPad<CBgpTroop_MoveAlong>();

	AnimTick t=_GetT();
	BccRoute *bcc=&pad->_route;

	BOOL bAdjustSpeed=TRUE;//是否需要进行速度调整
	if (bAdjustSpeed)
		bcc->UpdateDistsToGo();

	BOOL bAnyControlling=FALSE;
	BOOL bAllReached=TRUE;
	float sumDistToGo=0.0f;
	int nDistToGo=0;
	for (int i=0;i<_entries.size();i++)
	{
		Entry &e=_entries[i];

		e.distToGo=-1.0f;

		if (!_CanControl(e))
		{
			SAFE_DESTROY(e.anSpeed);
			continue;
		}

		if (bAdjustSpeed)
		{
			if (LevelUtil_IsMoving_(e.lo))
			{
				CUnit *unit=e.lo->GetUnit();
				if (unit)
				{
					e.distToGo=unit->GetDistToGo();
					if (e.distToGo>=0.0f)
						e.distToGo+=bcc->distsToGo[e.idxNode];
				}
			}
			if (e.distToGo>=0.0f)
			{
				sumDistToGo+=e.distToGo;
				nDistToGo++;
			}
			else
			{
				if (e.bReached)
				{
					sumDistToGo+=0.0f;
					nDistToGo++;
				}
			}
		}

		if (e.bReached)
			continue;


		bAnyControlling=TRUE;

		LevelPos pos=e.lo->GetFramePos();

		//检测是否走到当前的目的地
		BOOL bReached=FALSE;
		if (TRUE)
		{
			float dist2=(pos.x-bcc->sphereset[e.idxNode].center.x)*(pos.x-bcc->sphereset[e.idxNode].center.x)+
				(pos.y-bcc->sphereset[e.idxNode].center.z)*(pos.y-bcc->sphereset[e.idxNode].center.z);
			if (dist2<bcc->sphereset[e.idxNode].radius*bcc->sphereset[e.idxNode].radius)
				bReached=TRUE;
		}

		if (bReached)
		{
			e.idxNode++;//already in range,go to the next route node

			if (e.idxNode<bcc->sphereset.size())
			{
				bAllReached=FALSE;
				LevelPos posTarget;
				posTarget.x=bcc->sphereset[e.idxNode].center.x;
				posTarget.y=bcc->sphereset[e.idxNode].center.z;
				extern BOOL LevelUtil_MoveTo(CLevelObj *lo,LevelPos &pos,float rangeFollow);
				LevelUtil_MoveTo(e.lo,posTarget,-1.0f);
			}
			else
				e.bReached=TRUE;
		}
		else
		{
			bAllReached=FALSE;

			if (t>e.tStartMove+ANIMTICK_FROM_SECOND(1.0f))
			{//开始移动后一段时间后检测
				if (!LevelUtil_IsMovingOrRotating(e.lo))
				{//不知道什么原因停止移动了

					//重新开始
					_StartMove(e,bcc,t);
				}
			}
		}
	}

	if (!bAnyControlling)
	{
		_OutputFail(outputs,2,"失败");
		return;
	}
	if (bAllReached)
	{
		_OutputOk(outputs,1,"成功");
		return;
	}

	//根据distToGo调整速度,使整个队伍不至于拉得太开
	if (bAdjustSpeed)
	{
		if (nDistToGo>0)
		{
			float distAvg=sumDistToGo/(float)nDistToGo;

			for (int i=0;i<_entries.size();i++)
			{
				Entry &e=_entries[i];

				if (e.distToGo<0.0f)
					continue;

				CLevelObjMove *move=e.lo->GetMove();
				if (!move)
					continue;

				float speedOrg=move->GetOrgSpeed();

				BOOL bAcc=e.anSpeed?TRUE:FALSE;//是否正在加速
				BOOL bNeedAcc=FALSE;
				float tDelta=(e.distToGo-distAvg)/speedOrg;//落后时间
				if (!bAcc)
				{
					if (tDelta>0.2f)
						bNeedAcc=TRUE;
				}
				else
				{
					if (tDelta>-0.2f)
						bNeedAcc=TRUE;
				}

				if (bNeedAcc)
				{
					float speedIntend=speedOrg*1.5f;
					if (!e.anSpeed)
					{
						SpeedMod *mod=move->ObtainSpeedMod();
						if (mod)
						{
							e.anSpeed=(AttrNodeFloat*)mod->speed.Add(speedIntend,200);//very high priority
						}
					}
				}
				else
				{
					SAFE_DESTROY(e.anSpeed);
				}
			}
		}

	}


}
