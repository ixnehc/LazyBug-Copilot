/********************************************************************
	created:	2012/10/22 
	author:		cxi
	
	purpose:	骑行的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"

#include "LevelEvents.h"

#include "Buff_Mount.h"


//////////////////////////////////////////////////////////////////////////
//CBuff_Dead
BIND_BUFFPARAM(Buff_Mount,BuffParam_Mount,BuffArg_Mount);

LevelBuffMask Buff_Mount::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
// 	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Mount)->GetUID());
// 	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KB)->GetUID());
// 	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KD)->GetUID());

	return mask;
}

void Buff_Mount::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_Mount*arg=(BuffArg_Mount*)arg0;

	CLevelObj *owner=_GetOwner();

	CLevelObj*obj=owner->GetLevel()->GetIDs()->LoFromID(arg->idTarget);
	if (obj)
	{
		CLevelObjPauser *pauser=owner->GetPauser();
		if (pauser)
		{
			_dur=ANIMTICK_INFINITE;

			_data.idTarget=arg->idTarget;
			_data.idTeleport=owner->GenTeleportID();
			extern float LevelUtil_GetMountHeight(CLevelObj *lo);
			float htMount=LevelUtil_GetMountHeight(obj);
			_data.idBroken=pauser->TeleportMount(obj->GetID(),htMount,_data.idTeleport);
			_data.bCanceled=FALSE;
			_data.posCancel.set(0,0);
		}
	}

	if (arg->bNeedEnter)
	{
		LevelRecordBuff *rec=_rec;
		if (rec)
		{
			BuffParam_Mount *param=rec->GetParam<BuffParam_Mount>();
			if (param)
				_durEnter=param->durEnter;
		}

		_SetFlag(BuffFlag_Mount|BuffFlag_GhostCollide|BuffFlag_Pausing);//暂停技能使用
	}
	else
	{
		_durEnter=0;
		_SetFlag(BuffFlag_Mount);
	}

}


void Buff_Mount::_OnUpdate(AnimTick t)
{
	if (_data.bCanceled)
		_SetFlag(BuffFlag_Mount|BuffFlag_Pausing);
	else
	{
		if (_tAge>=_durEnter)
			_SetFlag(BuffFlag_Mount);
	}
}



void Buff_Mount::_OnDestroy()
{
}


void Buff_Mount::_WriteData(CBitPacket *dp)
{
	dp->Data_WriteSimpleR(_data);
}

void Buff_Mount::_CancelMount(LevelOpLink &link)
{
	CLevelObj *owner=_GetOwner();
	if (!owner)
		return;

	CLevelObjPauser *pauser=owner->GetPauser();
	if (!pauser)
		return;

	LevelPos pos;//找一个落下的位置
	if (TRUE)
	{
		extern LevelPos LevelUtil_FindPosAround(LevelPos &center,float radius0,CLevel *level,DWORD nTry);
		CLevelObj *loMount=_GetLevel()->GetIDs()->LoFromID(_data.idTarget);
		BOOL bFound=FALSE;
		if (loMount)
		{
			pos=LevelUtil_FindPosAround(loMount->GetFramePos(),loMount->GetRadius_()+owner->GetRadius_(),_GetLevel(),4);
			bFound=TRUE;
		}
		if (!bFound)
			pos=owner->GetFramePos();
	}

	//掉落到地面位置上
	LevelTeleportID idTeleport=owner->GenTeleportID();
	extern float LevelUtil_GenRandomFace();
	LevelSkillID idBroken=pauser->TeleportGround(pos,LevelUtil_GenRandomFace(),idTeleport);//暂时用一个随机的朝向

	//通知客户端取消mound
	if (TRUE)
	{
		LevelOp_CancelMount*op=NewOp<LevelOp_CancelMount>();
		op->GetDesc().link=link;
		op->pos=pos;
		op->idTeleport=idTeleport;
		op->idBroken=idBroken;

		owner->AddOp(op);
	}

	//延迟结束自己
	LevelRecordBuff *rec=_rec;
	if (rec)
	{
		BuffParam_Mount *param=rec->GetParam<BuffParam_Mount>();
		if (param)
			_dur=param->durExit;
	}

	//更新自己的状态
	_data.idTeleport=idTeleport;
	_data.idBroken=idBroken;
	_data.bCanceled=1;
	_data.posCancel=pos;

}


void Buff_Mount::HandleEvent(LevelEvent &e0)
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
				_CancelMount(e.link);
			}
		}
	}
}
