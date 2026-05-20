/********************************************************************
	created:	2012/01/17
	file base:	Buff_Dead
	author:		cxi
	
	purpose:	死亡的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"
#include "LevelUtil.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelObjPauser.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"


#include "Buff_Dead.h"

#include "Buff_KB.h"
#include "Buff_KD.h"
#include "Buff_Dizzy.h"

#include "Random/Random.h"

//////////////////////////////////////////////////////////////////////////
//BuffParam_Dead
int BuffParam_Dead::FindDyingDesc(float htMe,LevelFace faceMe,LevelFace faceStrike)
{
	float ht=htMe;

	std::vector<int> indices;
	CSysRandom::GenRandomIndices(indices,dyings.size());

	float gapMin=10000000.0f;
	float gapHtMin=10000000.0f;
	int idxDesc=-1;
	for (int i=0;i<indices.size();i++)
	{
		DyingDesc &desc=dyings[indices[i]];
		LevelFace face=faceMe;
		LevelFaceApplyYaw(face,desc.yaw*i_math::GRAD_PI2);

		float gap=i_math::get_radian_dist(face,faceStrike);
		float gapHt=fabsf(ht-desc.ht);
		if (gapHt<gapHtMin-0.001f)
		{
			idxDesc=indices[i];
			gapMin=gap;
			gapHtMin=gapHt;
		}
		else
		{
			if (gapHt<gapHtMin+0.001f)
			{
				if (gap<gapMin)
				{
					idxDesc=indices[i];
					gapMin=gap;
					gapHtMin=gapHt;
				}
			}
		}
	}

	return idxDesc;
}


////////////////////////////////////////////////////////////////////////
//BuffData_Dead
void BuffData_Dead::Save(CBitPacket *bp)
{
	bp->Data_WriteSimple(idBroken);
	strike.Save(bp);
	bp->Bits_Write(tpObliterate,3);
	bp->Data_WriteSimple(idxDesc);
}

void BuffData_Dead::Load(CBitPacket *bp)
{
	bp->Data_ReadSimple(idBroken);
	strike.Load(bp);
	tpObliterate=(LevelObliterateType)bp->Bits_Read(3);
	bp->Data_ReadSimple(idxDesc);
}


//////////////////////////////////////////////////////////////////////////
//CBuff_Dead
BIND_BUFFPARAM(Buff_Dead,BuffParam_Dead,BuffArg_Dead);

LevelBuffMask Buff_Dead::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask |= ((LevelBuffMask)1) << (Class_Ptr2(Buff_Dead)->GetUID());
	mask |= ((LevelBuffMask)1) << (Class_Ptr2(Buff_KB)->GetUID());
	mask |= ((LevelBuffMask)1) << (Class_Ptr2(Buff_KD)->GetUID());
	mask |= ((LevelBuffMask)1) << (Class_Ptr2(Buff_Dizzy)->GetUID());
	mask |= ((LevelBuffMask)1) << (Class_Ptr2(Buff_Stun)->GetUID());

	return mask;
}

CLevelBuff::ConflictResult Buff_Dead::CheckConflict(CLevelBuff *buffExist)
{
	if (buffExist->GetRec())
	{
		if (buffExist->GetRec()->bDestroyOnDie)
			return Conflict_Replace;
	}
	return CLevelBuff::CheckConflict(buffExist);
}


void Buff_Dead::_OnCreate(LevelBuffArg *arg0)
{
	BuffParam_Dead *param=(BuffParam_Dead *)_param;
	BuffArg_Dead *arg=(BuffArg_Dead *)arg0;

	CLevelObj *owner=_GetOwner();
	LevelPos3D src=owner->GetFramePos3D();
	LevelFace face=owner->GetFrameFace();
	float ht;
	if (TRUE)
	{
		LevelPos3D posGround=LevelUtil_GetWalkableGroundHeight(_GetLevel(),src.x,src.z,TRUE);
		ht=src.y-posGround.y;
		if (ht<0.0f)
			ht=0.0f;
	}

	LevelFace faceStrike=CSysRandom::RandRange(0.0f,i_math::Pi*2.0f);
	if (arg->strike)
		faceStrike=arg->strike->GetFace();

	_data.idxDesc=param->FindDyingDesc(ht,face,faceStrike);
	if (arg->strike)
		_data.strike=*arg->strike;
	if (arg->argObliterate)
		_data.tpObliterate=(*arg->argObliterate).tp;

	RecordID idSkill=RecordID_Invalid;
	if (_data.tpObliterate==LevelObliterate_None)
	{
		if (_data.idxDesc>=0)
			idSkill=param->dyings[_data.idxDesc].idSkill;
	}

	CLevelObjPauser *pauser=owner->GetPauser();
	if (pauser)
	{
		_dur=ANIMTICK_INFINITE;

		if (idSkill==RecordID_Invalid)
			_data.idBroken=pauser->Pause();
		else
			_data.idBroken=pauser->PauseNoDelay();
	}

// 	assert(arg->strike);


	//Obliterate
	if (_data.tpObliterate!=LevelObliterate_None)
	{
		DealArg argDeal;
		argDeal.link=arg->link;
		argDeal.dir.setXZ(_data.strike.GetDir());
		argDeal.grd=1;
		argDeal.amount=1;
		argDeal.argObliterate=arg->argObliterate;

		for (int i=0;i<param->dealsObliterate.size();i++)
		{
			ObliterateDealEntry &entry=param->dealsObliterate[i];
			if (entry.tp!=_data.tpObliterate)
				continue;
			if (entry.deal->IsNull())
				continue;

			if (arg->osbSrc)
				entry.deal->Make(*arg->osbSrc,GetOwner(),argDeal,NULL);
			else
				entry.deal->Make(LevelOSB(this),GetOwner(),argDeal,NULL);
		}
	}

	if (!param->deal->IsNull())
		_bNeedDeal=TRUE;
	else
		_bNeedDeal=FALSE;

	_UpdateCreateEo();

	if (idSkill!=RecordID_Invalid)
	{
		_bInSkill=TRUE;
		_buffs->MarkFlagsDirty();

		CLevelSkillDriver *driver=owner->GetSkillDriver();
		if (driver)
		{
			LevelSkillTarget target;
			LevelPos dirStrike=LevelFaceToDir(faceStrike);
			LevelPos posTarget=src.getXZ()-dirStrike*1.0f;
			target.SetPos(posTarget);

			driver->StartCast(LevelSkillType(idSkill),target,1,NULL,&arg->link);
		}

	}

}

void Buff_Dead::_WriteData(CBitPacket *bp)
{
	_data.Save(bp);
}

void Buff_Dead::_UpdateCreateEo()
{
	if (_bNeedDeal)
	{
		BuffParam_Dead *param=(BuffParam_Dead *)_param;
		if (_tAge>=param->delayDeal)
		{
			if (param->deal)
			{
				if (GetOwner())
				{
					DealArg arg;
					arg.grd=1;
					arg.link.id=_GetLevel()->GenOpLinkID();
					param->deal->Make(LevelOSB(this),GetOwner()->GetFramePos3D(),arg,NULL);
				}
			}
			_bNeedDeal=FALSE;
		}
	}
}


void Buff_Dead::_OnUpdate(AnimTick dt)
{
	_UpdateCreateEo();

	if (_bInSkill)
	{
		CLevelObj *owner=_GetOwner();
		CLevelSkillDriver *driver=owner->GetSkillDriver();
		if (driver)
		{
			if (!driver->IsSkillCasting())
				_bInSkill=FALSE;
		}
		else
			_bInSkill=FALSE;
		if (!_bInSkill)
			_buffs->MarkFlagsDirty();
	}

}

void Buff_Dead::_OnDestroy()
{
}
