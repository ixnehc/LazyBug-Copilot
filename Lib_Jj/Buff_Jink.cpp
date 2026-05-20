/********************************************************************
	created:	2012/01/11
	file base:	Buff_Jink
	author:		cxi
	
	purpose:	击退的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LevelRecordBuff.h"


#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelDeal.h"


#include "Buff_Jink.h"
#include "Buff_KD.h"
#include "Buff_Dead.h"

//////////////////////////////////////////////////////////////////////////
//CBuff_Jink

BIND_BUFFPARAM(Buff_Jink,BuffParam_Jink,BuffArg_Jink);

LevelBuffMask Buff_Jink::GetForbiddingBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_KD)->GetUID());
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Dead)->GetUID());

	return mask;

}


LevelBuffMask Buff_Jink::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Jink)->GetUID());

	return mask;
}

void Buff_Jink::_OnCreate(LevelBuffArg *arg0)
{
	BuffArg_Jink *arg=(BuffArg_Jink *)arg0;
	BuffParam_Jink *param=(BuffParam_Jink *)_param;

	CLevelObj *owner=_GetOwner();

	if (TRUE)
	{
		_dur=param->durHide+param->durShow;
		float dist=owner->GetFramePos().getDistanceFrom(arg->pos);
		_dur+=ANIMTICK_FROM_SECOND(dist/param->spd);
	}


	CLevelObjPauser *pauser=owner->GetPauser();
	if (pauser)
	{
		_dur+=pauser->GetDelay();
		_idTeleport=owner->GenTeleportID();
		_idBroken=pauser->Teleport(arg->pos,arg->face,_idTeleport);
	}

	_strike=arg->strike;
	_pos=arg->pos;
	_strike=arg->strike;

	_tFinish=_dur;

}

void Buff_Jink::_OnDestroy()
{
}

void Buff_Jink::_OnUpdate(AnimTick dt)
{
	BuffParam_Jink *param=(BuffParam_Jink *)_param;
	if (!_bShowDealed)
	{
		if (_tAge+param->durShow>=_tFinish)
		{
			DealArg arg;
			arg.grd=1;
			arg.link.id=_GetLevel()->GenOpLinkID();
			LevelPos3D pos3D;
			extern LevelPos3D LevelUtil_GetGroundHeight(CLevel *lvl,float x,float y,BOOL bHiReso);
			pos3D=LevelUtil_GetGroundHeight(_GetLevel(),_pos.x,_pos.y,TRUE);
			MakeDeals(param->dealsShow,LevelOSB(this),pos3D,arg,NULL);

			_bShowDealed=TRUE;
		}
	}
}



void Buff_Jink::_WriteData(CBitPacket *bp)
{
	bp->Data_WriteSimple(_idBroken);
	bp->Data_WriteSimple(_idTeleport);
	bp->Data_WriteSimple(_pos);
	bp->Data_WriteSimple(_face);
	_strike.Save(bp);

}
