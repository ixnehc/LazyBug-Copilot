/********************************************************************
	created:	2022/06/26
	author:		cxi
	
	purpose:	FireFly的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelSkillDriver.h"

#include "LoUnit.h"

#include "LevelRecordBuff.h"

#include "LevelRecordUnit.h"

#include "EoEnv.h"

#include "LevelUtil.h"


#include "Buff_FireFly.h"



//////////////////////////////////////////////////////////////////////////
//CBuff_FireFly
BIND_BUFFPARAM(Buff_FireFly,BuffParam_FireFly,BuffArg_FireFly);


LevelBuffMask Buff_FireFly::GetForbiddingBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_FireFly)->GetUID());

	return mask;
}

void Buff_FireFly::_OnCreate(LevelBuffArg *arg)
{
	BuffParam_FireFly *param=(BuffParam_FireFly*)_param;

	CLevelObj *owner=_GetOwner();

	KeySet_Define(&_ksFlee,KT_Floatx2);

}

void Buff_FireFly::LoadTeleport(CLevelBuff *buffOrg0)
{
	Buff_FireFly* buffOrg=(Buff_FireFly*)buffOrg0;

	_ksFlee.CopyFrom(buffOrg->_ksFlee);
	_ksFlee.Clean();

}


BOOL Buff_FireFly::GetCurFleePos(LevelPos &posFlee)
{
	if (!_bFleeing)
		return FALSE;

	AnimTick tCur=_GetOwner()->GetT();
	Key_2f k;
	_ksFlee.CalcKey<Key_2f>(tCur,&k);
	posFlee=k.v;

	return TRUE;
}

void Buff_FireFly::LeaveTorch(CLevelObj *loPlayer)
{
	if (!_bInTorch)
		return;

	LevelPos posPlayer=loPlayer->GetFramePos();

	//选一条guide离开
	int iGuide=-1;
	float distMinToGuide=1000000.0f;
	for (int i=0;i<_guideTorch->size();i++)
	{
		FireFlyGuide &guide=(*_guideTorch)[i];
		if (guide.mats.size()>0)
		{
			LevelPos3D posGuide=guide.mats[guide.mats.size()-1].getTranslation();
			_matTorch.transformVect(posGuide,posGuide);
			float dist=posPlayer.getDistanceFrom(posGuide.getXZ());
			if (dist<distMinToGuide)
			{
				iGuide=i;
				distMinToGuide=dist;
			}
		}
	}

	if (iGuide>=0)
	{
		FireFlyGuide &guide=(*_guideTorch)[iGuide];

		LevelOp_FireFly *op=NewOp<LevelOp_FireFly>(LevelOpLink());
		op->op=LevelOp_FireFly::LeaveTorch;
		op->idPlayer=loPlayer->GetID();
		op->nPos=guide.mats.size();

		for (int i=0;i<op->nPos;i++)
		{
			LevelPos3D pos=guide.mats[i].getTranslation();
			_matTorch.transformVect(pos,pos);
			op->Pos3DBuf()[i]=pos;
		}

		_GetOwner()->AddOp(op);

		_bInTorch=FALSE;
		_guideTorch=NULL;
	}


}


void Buff_FireFly::_OnUpdate(AnimTick dt)
{
	BuffParam_FireFly *param=(BuffParam_FireFly*)_param;

	AnimTick tCur=_GetOwner()->GetT();

	if (_bInTorch)
	{
		if (_GetOwner())
		{
			extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
			CLevelObj *loPlayer=LevelUtil_GetOwnerLo(_GetOwner());
			if (loPlayer)
				LeaveTorch(loPlayer);
		}
	}

	if (_bFleeing)
	{
		if (_GetOwner())
		{
			AnimTick tFleeStart=_ksFlee.GetKey(0)->t;

			if (tCur>=tFleeStart+param->durFleeAge)
			{
				_GetOwner()->DeferDestroy();
				return;
			}

			extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
			CLevelObj *loPlayer=LevelUtil_GetOwnerLo(_GetOwner());
			if (loPlayer)
			{
				LevelPos posPlayer=loPlayer->GetFramePos();

				if (tCur>=tFleeStart+param->durFleeUnrescure)
				{
					Key_2f k;
					_ksFlee.CalcKey<Key_2f>(tCur,&k);
					LevelPos pos=k.v;

					if (posPlayer.getDistanceFrom(pos)<2.5f)
					{
						_bFleeing=FALSE;
						_ksFlee.Clean();
						LevelOp_FireFly *op=NewOp<LevelOp_FireFly>(LevelOpLink());

						op->op=LevelOp_FireFly::StopFlee;
						_GetOwner()->AddOp(op);
					}
				}
			}
		}

	}

	if (_bEnteringTorch)
	{
		if (_GetOwner())
		{
			if (tCur>=_tEnterTorch+ANIMTICK_FROM_SECOND(1.0f))
			{
				_GetOwner()->DeferDestroy();
				return;
			}
		}
	}


}

void Buff_FireFly::_OnDestroy()
{

}


void Buff_FireFly::_WriteData(CBitPacket *bp)
{
	bp->Bit_Write(_bInTorch);
}

void Buff_FireFly::HandleEvent(LevelEvent &e0)
{
	BuffParam_FireFly *param=(BuffParam_FireFly*)_param;

	if ((!_bFleeing)&&(!_bEnteringTorch))
	{
		if (e0.GetType()==LET_OwnerDamage)
		{
			CLevelObj *loOwner=_GetOwner();
			if (loOwner)
			{
				extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
				CLevelObj *loPlayer=LevelUtil_GetOwnerLo(loOwner);
				if (loPlayer)
				{

					LevelPos posPlayer=loPlayer->GetFramePos();
					LevelPos posFlee;

					if (TRUE)
					{
						_eoEnvWorking=NULL;
						CLevelObj *eoEnv=_GetLevel()->GetEoEnv();
						if (eoEnv)
						{
							if (eoEnv->GetClass()->IsSameWith(Class_Ptr2(EoEnv)))
								_eoEnvWorking=(EoEnv*)eoEnv;
						}
					}

					FindNearbyPosCallBack dlgtFindNearbyPos;
					FindFleePosCallBack dlgtFindFleePos;
					if (_eoEnvWorking)
					{
						dlgtFindNearbyPos.bind(this,&Buff_FireFly::_CheckInArea);
						dlgtFindFleePos.bind(this,&Buff_FireFly::_CheckInArea);
					}
					if (LevelUtil_FindNearbyPos(_GetLevel(),posPlayer,10.0f,TRUE,TRUE,15,posFlee,dlgtFindNearbyPos))
					{
						_bFleeing=TRUE;
						AnimTick tStart=_GetLevel()->GetT_();

						DWORD nFleeSeg=10;//修改这个值要确保不超过LevelOp_FireFly::bufPos的容量
						float distFleeSeg=3.0f;

						_ksFlee.SetKeyCount(nFleeSeg+2);

						if (TRUE)
						{
							Key_2f *k=(Key_2f *)_ksFlee.GetKey(0);
							k->t=tStart;
							k->v=posPlayer;
						}

						if (TRUE)
						{
							Key_2f *k=(Key_2f *)_ksFlee.GetKey(1);
							float dur=posFlee.getDistanceFrom(posPlayer)/((param->speedFleeStart+param->speedFlee)/2.0f);
							k->t=tStart+ANIMTICK_FROM_SECOND(dur);
							k->v=posFlee;
						}

						int signAvoid=0;
						for (int i=0;i<nFleeSeg;i++)
						{
							float distKeep=posFlee.getDistanceFrom(posPlayer)+distFleeSeg;
							LevelPos posNew;
							if (LevelUtil_FindFleePos(_GetLevel(),posFlee,posPlayer,distKeep,signAvoid,NULL,0.0f,posNew,dlgtFindFleePos))
							{
								Key_2f *k=(Key_2f *)_ksFlee.GetKey(i+2);
								Key_2f *kPrev=(Key_2f *)_ksFlee.GetKey(i+1);
								float dur=posNew.getDistanceFrom(posFlee)/param->speedFlee;
								k->t=kPrev->t+ANIMTICK_FROM_SECOND(dur);
								k->v=posNew;
								posFlee=posNew;
							}
							else
							{
								_ksFlee.SetKeyCount(i+2);
								break;
							}
						}

						LeOwnerDamage &e=(LeOwnerDamage &)e0;
						LevelOp_FireFly *op=NewOp<LevelOp_FireFly>(e.link);

						op->op=LevelOp_FireFly::StartFlee;
						for (int i=1;i<_ksFlee.GetKeyCount();i++)
						{
							Key_2f *k=(Key_2f *)_ksFlee.GetKey(i);
							op->PosBuf()[i-1]=k->v;
						}
						op->nPos=_ksFlee.GetKeyCount()-1;

						_GetOwner()->AddOp(op);
					}
				}
			}
		}
	}
}

BOOL Buff_FireFly::EnterTorch(LevelObjID idTorch,std::vector<FireFlyGuide>&guides,i_math::matrix43f &matBase)
{
	if (_bFleeing)
		return FALSE;
	if (_bEnteringTorch)
		return FALSE;

	std::vector<i_math::matrix43f>*guideClosest=NULL;
	if (TRUE)
	{
		LevelPos posOwner=GetOwner()->GetFramePos();

		//选一条guide进入
		float distMinToGuide=1000000.0f;
		for (int i=0;i<guides.size();i++)
		{
			FireFlyGuide &guide=guides[i];
			if (guide.mats.size()>0)
			{
				LevelPos3D posGuide=guide.mats[0].getTranslation();
				matBase.transformVect(posGuide,posGuide);
				float dist=posOwner.getDistanceFrom(posGuide.getXZ());
				if (dist<distMinToGuide)
				{
					guideClosest=&guide.mats;
					distMinToGuide=dist;
				}
			}
		}
	}

	if (!guideClosest)
		return FALSE;

	LevelOp_FireFly *op=NewOp<LevelOp_FireFly>(LevelOpLink());

	op->op=LevelOp_FireFly::EnterTorch;
	for (int i=0;i<guideClosest->size();i++)
	{
		op->Pos3DBuf()[i]=(*guideClosest)[i].getTranslation();
		matBase.transformVect(op->Pos3DBuf()[i],op->Pos3DBuf()[i]);
	}
	op->nPos=guideClosest->size();
	op->idTorch=idTorch;

	_GetOwner()->AddOp(op);

	_bEnteringTorch=TRUE;
	_tEnterTorch=GetOwner()->GetT();


	return TRUE;
}

BOOL Buff_FireFly::_CheckInArea(LevelPos &pos)
{
	if (!_eoEnvWorking)
		return TRUE;
	BccArea &area=_eoEnvWorking->GetArea();
	if (area.IsEmpty())
		return TRUE;
	return area.CheckIn(pos);
}
