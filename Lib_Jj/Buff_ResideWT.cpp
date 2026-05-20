/********************************************************************
	created:	2012/10/22 
	author:		cxi
	
	purpose:	驻扎进岗楼的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"

#include "LevelObjResidable.h"

#include "LevelEvents.h"

#include "LoWatchTower.h"

#include "Buff_ResideHole.h"//for BuffArg_Reside

#include "Buff_ResideWT.h"


//////////////////////////////////////////////////////////////////////////
//CBuff_Dead
BIND_BUFFPARAM(Buff_ResideWT,BuffParam_ResideWT,BuffArg_Reside);

LevelBuffMask Buff_ResideWT::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
// 	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_ResideWT)->GetUID());
// 	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KB)->GetUID());
// 	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KD)->GetUID());

	return mask;
}

void Buff_ResideWT::_OnCreate(LevelBuffArg *param0)
{
	BuffArg_Reside *param=(BuffArg_Reside*)param0;

	CLevelObj *owner=_GetOwner();

	CLoWatchTower*obj=owner->GetLevel()->GetIDs()->LoFromID<CLoWatchTower>(param->idTarget);
	if (obj)
	{
		LevelPos3D pos;
		CLevelObjResidable *residable=obj->GetResidable();

		//跳转到目的位点上去
		if (residable)
		{
			_token=residable->Occupy(owner->GetID());
			pos=residable->GetSeatPos(_token);
		}

		CLevelObjPauser *pauser=owner->GetPauser();
		if (pauser)
		{
			_dur=ANIMTICK_INFINITE;

			_data.idTarget=param->idTarget;
			_data.idTeleport=owner->GenTeleportID();
			_data.idBroken=pauser->TeleportResided(pos,_data.idTeleport);//Reside到target的位点上
			_data.pos.set(pos.x,pos.z);
			//记录进入位点,这个位点用于出来时确定位置
			obj->FindEntry(owner->GetFramePos(),_posEntry);
			_data.bCanceled=FALSE;

			_SetFlag(BuffFlag_Reside|BuffFlag_GhostCollide|BuffFlag_Pausing);//暂停技能使用
		}
	}

	LevelRecordBuff *rec=_rec;
	if (rec)
	{
		BuffParam_ResideWT *param=rec->GetParam<BuffParam_ResideWT>();
		if (param)
			_durEnter=param->durEnter;
	}
}

void Buff_ResideWT::_DiscardSeatToken()
{
	if (_token!=LevelObjSeatToken_Invalid)
	{
		CLevelObj *loTarget=_GetOwner()->GetLevel()->GetIDs()->LoFromID(_data.idTarget);
		if (loTarget)
		{
			CLevelObjResidable *residable=loTarget->GetResidable();
			if (residable)
				residable->Discard(_token);
		}
		_token=LevelObjSeatToken_Invalid;
	}
}

void Buff_ResideWT::_OnUpdate(AnimTick t)
{
	if (_data.bCanceled)
		_SetFlag(BuffFlag_Reside|BuffFlag_Pausing);
	else
	{
		if (_tAge>=_durEnter)
			_SetFlag(BuffFlag_Reside);
	}
}



void Buff_ResideWT::_OnDestroy()
{
	_DiscardSeatToken();
}


void Buff_ResideWT::_WriteData(CBitPacket *dp)
{
	dp->Data_WriteSimpleR(_data);
}

void Buff_ResideWT::_CancelReside(LevelOpLink &link)
{
	CLevelObj *owner=_GetOwner();
	if (!owner)
		return;

	CLevelObjPauser *pauser=owner->GetPauser();
	if (!pauser)
		return;

	//掉落到入口点的地面位置上
	LevelTeleportID idTeleport=owner->GenTeleportID();
	extern float LevelUtil_GenRandomFace();
	LevelSkillID idBroken=pauser->TeleportGround(_posEntry,LevelUtil_GenRandomFace(),idTeleport);//暂时随机产生一个朝向

	//通知客户端取消reside
	if (TRUE)
	{
		LevelOp_CancelReside *op=NewOp<LevelOp_CancelReside>();
		op->GetDesc().link=link;
		op->pos=_posEntry;
		op->idTeleport=idTeleport;
		op->idBroken=idBroken;

		owner->AddOp(op);
	}

	//延迟结束自己
	LevelRecordBuff *rec=_rec;
	if (rec)
	{
		BuffParam_ResideWT *param=rec->GetParam<BuffParam_ResideWT>();
		if (param)
			_dur=param->durExit;
	}

	//记录下自己的状态
	_data.idTeleport=idTeleport;
	_data.idBroken=idBroken;
	_data.bCanceled=1;
	_data.pos=_posEntry;

}


void Buff_ResideWT::HandleEvent(LevelEvent &e0)
{
	__super::HandleEvent(e0);
	if (e0.bHandled)
		return;

	if (e0.GetType()==LET_Kill)
	{
		LeKill &e=(LeKill &)e0;
		if (e.loTarget)
		{
			if (e.loTarget->GetID()==_data.idTarget)
			{//Reside的对象物体被杀死了
				_CancelReside(e.link);
			}
		}
	}
}
