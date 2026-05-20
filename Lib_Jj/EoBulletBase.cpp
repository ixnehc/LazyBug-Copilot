
#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoBullet.h"

#include "LevelRecords.h"

#include "LevelOSB.h"


//////////////////////////////////////////////////////////////////////////
//EoBulletBase
void EoBulletBase::_OnPostCreate()
{
	EoParamBulletBase*param=_rec?(EoParamBulletBase*)_rec->param:NULL;
	if (param)
	{
		if (param->modeInitialXfm==EoParamBulletBase::LocalOffsetToHostAim)
		{
			if (CLevelObj *loOwner=_GetOwner())
			{
				LevelPos3D posLocal(param->xLocalOff,param->yLocalOff,param->zLocalOff);
				i_math::matrix43f mat;
				extern void LevelUtil_CalcLoMat(CLevelObj *lo,i_math::matrix43f &mat);
				LevelUtil_CalcLoMat(loOwner,mat);
				mat.transformVect(posLocal,_xfmInitial.pos);
			}
			if (_idHost!=LevelObjID_Invalid)
			{
				CLevelObj *lo=LevelUtil_GetAliveLo(_level,_idHost);
				if (lo)
				{
					LevelPos3D posTarget=lo->GetFramePos3D();
					posTarget.y+=lo->GetAimHeight();
					LevelPos3D dir=posTarget-_xfmInitial.pos;
					dir.normalize();
					_xfmInitial.fromZAxis(_xfmInitial.pos,dir);
				}
			}
		}
	}

	_core=_CreateBullet();
	_t=_GetT();
}

void EoBulletBase::_OnDetroy()
{
	if (_core)
	{
		Safe_Class_Delete(_core->_absorb);
		_DestroyBullet(_core);
	}
	_core=NULL;
}

void EoBulletBase::_WriteHits(CBitPacket *bp,BOOL &bContent)
{
	if ((_hitsToSend.IsEmpty())&&(_hitStaticToSend.IsEmpty()))
		bp->Bit_Write_0();
	else
	{
		bp->Bit_Write_1();
		bp->Bits_Write(_hitsToSend.c,4);
		for (int i=0;i<_hitsToSend.c;i++)
			bp->Data_WriteSimple(_hitsToSend.ids[i]);

		bp->Bits_Write(_hitStaticToSend.tp,2);
		if (!_hitStaticToSend.IsEmpty())
		{
			bp->Data_WriteSimpleR(_core->GetPos());
// 			if (_hitStaticToSend.tp==BulletStaticHit::ShieldAmulet)
// 				bp->Data_WriteSimpleR(_hitStaticToSend.id);
		}

		bContent=TRUE;
	}
}

void EoBulletBase::_WriteAbsorb(CBitPacket *bp,BOOL &bContent)
{
	if (_bNeedSyncAbsorb)
	{
		bp->Bit_Write_1();
		bp->Data_WriteSimple(_core->_absorb->idTarget);
		bContent=TRUE;
	}
	else
		bp->Bit_Write_0();
}



void EoBulletBase::_OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	EoParamBulletBase*param=_rec?(EoParamBulletBase*)_rec->param:NULL;
	if (param)
	{
		if (param->modeInitialXfm==EoParamBulletBase::LocalOffsetToHostAim)
		{
			CLevelObj *loOwner=_GetOwner();
			bp->Data_WriteSimple(loOwner?loOwner->GetID():LevelObjID_Invalid);
			bp->Data_WriteSimple(_idHost);
		}
	}
	_WriteHits(bp,bContent);
	_WriteAbsorb(bp,bContent);
}

void EoBulletBase::_OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)
{
	_WriteHits(bp,bContent);
	_WriteAbsorb(bp,bContent);
}

void EoBulletBase::_OnPostWriteSync()
{
	_hitsToSend.Zero();
	_hitStaticToSend.Zero();
	_bNeedSyncAbsorb=FALSE;
}

BulletAbsorbSetting *EoBulletBase::_FindAbsorbSetting(EoParamBulletBase*param,CLevelObj *loTarget)
{
	if (loTarget->GetType()==LevelObjType_Unit)
	{
		RecordID idUnit=((CLoUnit*)loTarget)->GetRecID();
		for (int i=0;i<param->absorbs.size();i++)
		{
			BulletAbsorbSetting *setting=&param->absorbs[i];
			for (int j=0;j<setting->units.size();j++)
			{
				if (setting->units[j]==idUnit)
					return setting;
			}
		}
	}
	return NULL;
}

void BuildBulletAbsorbSpline(i_math::vector3df &posStart,i_math::vector3df &dir,i_math::vector3df &posEnd,CCubicSpline &spline)
{
	i_math::vector3df dirStart=dir;
	dirStart.normalize();

	LevelPos3D posCorner;
	float distToCorner;
	if (TRUE)
	{
		i_math::vector3df dirTarget=posEnd-posStart;
		dirTarget.normalize();
		float d=dirTarget.dotProduct(dirStart);

		LevelPos3D posCenter=(posStart+posEnd)*0.5f;
		distToCorner=posCenter.getDistanceFrom(posStart)/d;

		posCorner=posStart+dirStart*distToCorner;
	}

	i_math::vector3df dirEnd=(posEnd-posCorner);
	dirEnd.normalize();


	i_math::vector3df velStart=dirStart*distToCorner*1.0f;
	i_math::vector3df velEnd=dirEnd*distToCorner*1.0f;

	const int nSample=6;
	spline.Reset(FALSE);
	for (int i=0;i<nSample;i++)
	{
		float t=((float)i)/(float)(nSample-1);

		extern i_math::vector3df GetPositionOnCubic(const i_math::vector3df &startPos, const i_math::vector3df &startVel, const i_math::vector3df &endPos, const i_math::vector3df &endVel, float time);
		i_math::vector3df pos=GetPositionOnCubic(posStart,velStart,posEnd,velEnd,t);
		spline.AddNode(pos,i_math::quatf());
	}

	spline.BuildRNS();

}

void EoBulletBase::_OnUpdate()
{
	EoParamBulletBase*param=_rec?(EoParamBulletBase*)_rec->param:NULL;

	AnimTick t=_GetT();
	if (t>_t)
	{
		//Detect and apply absorb
		if (param)
		{
			if (_core&&(!_core->_absorb))
			{
				float radiusMax=0;
				for (int i=0;i<param->absorbs.size();i++)
				{
					if (param->absorbs[i].radius>radiusMax)
						radiusMax=param->absorbs[i].radius;
				}

				if (radiusMax)
				{
					LevelPos posCur=_core->GetPos().getXZ();
					LevelPos dirCur=_core->GetDir().getXZ();
					dirCur.normalize();
					DWORD c;
					CLevelObj **buf=_DetectRange(_core->GetPos().getXZ(),radiusMax,c);
					for (int i=0;i<c;i++)
					{
						CLevelObj *lo=buf[i];

						BulletAbsorbSetting *setting=_FindAbsorbSetting(param,lo);
						if (setting)
						{
							LevelPos3D posTarget=lo->GetFramePos3D();
							posTarget.y+=lo->GetAimHeight();

							if (posTarget.getXZ().getDistanceFrom(posCur)<setting->radius)
							{
								i_math::vector2df dir= posTarget.getXZ()-posCur;
								dir.safe_normalize();

								if (acos(dir.dotProduct(dirCur))*i_math::GRAD_PI<setting->angle)
								{
									_core->_absorb=Class_New2(BulletAbsorb);
									_core->_absorb->setting=setting;
									_core->_absorb->distStart=_core->_dist;
									_core->_absorb->idTarget=lo->GetID();
									i_math::vector3df dir=_core->GetDir();
									dir.normalize();
									BuildBulletAbsorbSpline(_core->GetPos(),dir,posTarget,_core->_absorb->spline);

									_bNeedSyncAbsorb=TRUE;

									break;
								}
							}
						}
					}
				}
			}
		}

		if (_core)
		{
			AnimTick dt=t-_t;
			LevelObjHits hits;
			BulletStaticHit hitStatic;
			LevelObjID hitAbsorb=LevelObjID_Invalid;
			_core->Update(ANIMTICK_TO_SECOND(dt),hits,hitStatic,hitAbsorb);
			if (hitAbsorb!=LevelObjID_Invalid)
			{
				CLevelObj *loTarget=LevelUtil_GetAliveLo(_level,_core->_absorb->idTarget);
				if (loTarget)
				{
					DealArg arg;
					arg.link.id=_level->GenOpLinkID();
					arg.dir=_core->GetDir();
					MakeDeals(_core->_absorb->setting->deals,LevelOSB(this),loTarget,arg,NULL);
				}
			}
			for (int i=0;i<hits.c;i++)
			{
				CLevelObj *loHit=LevelUtil_GetAliveLo(_level,hits.ids[i]);
				if (loHit)
				{
					DealArg arg;
					arg.link.id=_level->GenOpLinkID();
					arg.dir=_core->GetDir();

					_MakeDeals(LevelOSB(this),loHit,arg);

					_hitsToSend.Add(hits.ids[i]);
				}
			}
			if (!hitStatic.IsEmpty())
			{
				if (hitStatic.tp==BulletStaticHit::Default)
				{
					if (param->dealsStaticHit.size()>0)
					{
						DealArg arg;
						arg.link.id=_level->GenOpLinkID();
						arg.dir=_core->GetDir();

						MakeDeals(param->dealsStaticHit,LevelOSB(this),_core->GetPos(),arg,NULL);
					}
				}
				if (hitStatic.tp==BulletStaticHit::ShieldAmulet)
				{
					CLevelObj *loHit=LevelUtil_GetAliveLo(_level,hitStatic.id);
					if (loHit)
					{
						LevelOpLink link;
						link.id=_level->GenOpLinkID();
						LevelOp_DmgAbort*op=LevelOSB(this).NewOp<LevelOp_DmgAbort>(link);
						op->v.tp=LevelDmgAbort::ShieldAmulet;
						op->v.idEo=GetID();
						loHit->AddOp(op);
					}
				}
				_hitStaticToSend=hitStatic;
			}
		}
		_t=t;
	}

	if (_tFinish==ANIMTICK_INFINITE)
	{//尚未标记为结束
		BOOL bNeedFinish=TRUE;
		if (_core)
		{
			if (!_core->IsStop())
				bNeedFinish=FALSE;
		}
		if (bNeedFinish)
		{
			DealArg arg;
			arg.link.id=_level->GenOpLinkID();
			arg.link.iSerial=1;
			arg.dir=_core->GetDir();

			MakeDeals(param->dealsReach,LevelOSB(this),_core->GetPos(),arg,NULL);
			_tFinish=_t;//标记为结束
		}
	}

	if (_tFinish!=ANIMTICK_INFINITE)
	{
		if (_t>_tFinish+ANIMTICK_FROM_SECOND(4.0f))
			DeferDestroy();
	}
}

