/********************************************************************
	created:	2012/03/15
	file base:	Buff_ReviveBirth
	author:		cxi
	
	purpose:	出生的Buff
*********************************************************************/
#include "stdh.h"

#include "Level.h"

#include "LevelRecordBuff.h"

#include "Buff_Dormant.h"

#include "LevelAttrs.h"

#include "LevelObjMove.h"


#include "Random/Random.h"



//////////////////////////////////////////////////////////////////////////
//CBuff_Birth

BIND_BUFFPARAM(Buff_Dormant,BuffParam_Dormant,BuffArg_Dormant);

LevelBuffMask Buff_Dormant::GetReplaceBuffs()
{
	LevelBuffMask mask=0;
	mask|=((LevelBuffMask)1)<<(Class_Ptr2(Buff_Dormant)->GetUID());

	return mask;
}

void Buff_Dormant::_OnCreate(LevelBuffArg *arg0)
{
	BuffParam_Dormant *param=(BuffParam_Dormant *)_param;
	BuffArg_Dormant *arg=(BuffArg_Dormant *)arg0;
	CLevelObj *owner=_GetOwner();

	_dur=ANIMTICK_INFINITE;

	_sitesRevive=arg->sitesRevive;
	_nReviveSites=(WORD)arg->nReviveSites;

	if (param->forms.size())
		_iForm=CSysRandom::RandRangeInt<int>(0,param->forms.size());
}

void Buff_Dormant::_OnDestroy()
{

}

void ScatterDirs(LevelPos &src,LevelPos &aim,float rangeHalf,LevelPos *dirs,DWORD c)
{
	if (c==1)
	{
		dirs[0]=aim-src;
		dirs[0].normalize();
		return;
	}

	LevelPos dir;
	dir=aim-src;

	float range=rangeHalf;

	float rad=atan2f(dir.y,dir.x);

	float step=range*2.0f/(float)(c-1);

	float radFrom=rad-range;

	for (int i=0;i<c;i++)
	{
		float r=radFrom+step*(float)i;
		dirs[i].x=cosf(r);
		dirs[i].y=sinf(r);
	}

}


void Buff_Dormant::_OnUpdate(AnimTick t)
{
	BuffParam_Dormant *param=(BuffParam_Dormant *)_param;
	if(!_bRevived)
	{
		if (_bNeedRevive)
		{
			if (param->forms.size()>0)
			{
				DormantForm *form=&param->forms[_iForm];
				CLevel *level=_GetLevel();
				CLevelObj *owner=_GetOwner();
				LevelPos posMe=owner->GetFramePos();
				CLevelObj *loReviveTarget=level->GetIDs()->LoFromID(_idReviveTarget);
				BOOL bFind=FALSE;
				LevelPos posRevive;
				if (loReviveTarget)
				{
					LevelPos posTarget=loReviveTarget->GetFramePos();
					if (_nReviveSites>0)
					{//使用设定的位点
						int iReviveSite=0;
						if (TRUE)//找离目标最近的位点
						{
							float dist2Min=1000000000.0f;
							LevelPos pos;
							for (int i=0;i<_nReviveSites;i++)
							{
								i_math::vector3df *pos3D=_sitesRevive[i].getTranslationP();
								pos.set(pos3D->x,pos3D->z);

								float dist2=pos.getDistanceSQFrom(posTarget);
								if (dist2<dist2Min)
								{
									dist2Min=dist2;
									iReviveSite=i;
								}
							}
						}
						i_math::matrix43f &mat=_sitesRevive[iReviveSite];
						posRevive.set(mat.getTranslationP()->x,mat.getTranslationP()->z);
						bFind=TRUE;
					}
					else
					{//自动寻找一个位点

						i_math::vector2df dir=posTarget-posMe;
						float dist=dir.getLength();

						if (dist<=0.01f)
						{
							posRevive=posTarget;
							bFind=TRUE;
						}
						else
						{
							LevelPos dirs[4];
							ScatterDirs(posMe,posTarget,i_math::Pi/2.0f,dirs,ARRAY_SIZE(dirs));

							CLevel *level=_GetLevel();
							GameTileMap *gtm=level->GetGtm();

							LevelPos pos;
							float step=0.2f;
							float d=2.0f;
							while(!bFind)
							{
								d+=step;
								for (int i=0;i<ARRAY_SIZE(dirs);i++)
								{
									pos=posMe+dirs[i]*d;
									if (gtm->IsWalkable(pos.x,pos.y))
									{
										if (!level->GetUnitMgr()->StaticObstacleTest(UnitFindPath_Walkable,pos,posTarget))
										{
											posRevive=pos;
											bFind=TRUE;
											break;
										}
									}
								}
								if (d>dist)
									break;
							}
						}
					}
				}
				else
				{
					posRevive=owner->GetFramePos();
					bFind=TRUE;
				}

				if (bFind)
				{
					LevelTeleportID idTeleport=LevelTeleportID_Invalid;

					if (loReviveTarget)
					{
						posRevive=LevelPosFromInt(LevelPosToInt(posRevive));

						CLevelObjMove *move=owner->GetMove();
						if (move)
						{
							idTeleport=owner->GenTeleportID();
							LevelPos posOld=move->GetFramePos();
							LevelPos dir=posRevive-posOld;

							move->Teleport(idTeleport,posRevive,atan2f(dir.y,dir.x));
						}
					}

					_bRevived=1;
					_buffs->MarkFlagsDirty();

					_dur=form->durRevive;

					//修改Dur的Op
					if (TRUE)
					{
						LevelOp_ModBuffDur*op=NewOp<LevelOp_ModBuffDur>();
						op->durNew=_dur;
						owner->AddOp(op);
					}

					//苏醒的Op
					if (TRUE)
					{
						LevelOp_Revive *op=NewOp<LevelOp_Revive>();
						op->posRevive=posRevive;
						op->idTeleport=idTeleport;
						owner->AddOp(op);
					}
				}
			}
		}
	}
}


void Buff_Dormant::_WriteData(CBitPacket *dp)
{
	dp->Data_NextChar()=_iForm;
	dp->Data_NextByte()=_bRevived;
}
