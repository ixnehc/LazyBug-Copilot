/********************************************************************
	created:	2016/09/14 
	author:		cxi
	
	purpose:	 攻击Threat
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"
#include "Random/Random.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnThreat_FindAttackPos.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"
#include "LevelTroops.h"

#include "LevelUtil.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_FindAttackPos
BIND_BGN_CLASS(CBgnThreat_FindAttackPos,CBgpThreat_FindAttackPos);

void CBgnThreat_FindAttackPos::Destroy()
{
}



void CBgnThreat_FindAttackPos::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpThreat_FindAttackPos*pad=_GetPad<CBgpThreat_FindAttackPos>();
	CLevelObj *target=_GetThreat();
	CLevelObj *src=ctx->lo;
	UnitFindPathType tpFindPath=UnitFindPath_Walkable;

	if (target&&src)
	{
		if (src->IsAlive()&&target->IsAlive())
		{
			if (ctx->level)
			{
				LevelPos posTarget=target->GetFramePos();
				LevelPos posSrc=src->GetFramePos();
				LevelPos dirBase=posTarget-posSrc;
				LevelFace faceBase=LevelFaceFromDir(dirBase);

				for (int i=0;i<pad->_nTry;i++)
				{
					LevelPos pos;
					if (pad->_mode==0)
					{
						float dist=CSysRandom::RandRange(pad->_radiusMin,pad->_radiusMax);
						float faceOff=CSysRandom::RandRange(-pad->_fov/2.0f,pad->_fov/2.0f);
						faceOff*=i_math::GRAD_PI2;

						LevelPos dir=LevelFaceToDir(faceBase+faceOff);
						pos=posSrc+dir*dist;

						if (TRUE)
						{
							float distToThreat2=pos.getDistanceSQFrom(posTarget);
							if (distToThreat2<pad->_distMinToThreat*pad->_distMinToThreat)
							{
								//离目标太近
								if (distToThreat2>0.0001f)
								{
									//反推到最小距离以外
									if (TRUE)
									{
										LevelPos dirProj=pos-posTarget;
										dirProj.safe_normalize();
										dirProj*=pad->_distMinToThreat;
										pos=posTarget+dirProj;
									}

									float dist2=pos.getDistanceSQFrom(posSrc);
									if (dist2<pad->_radiusMin*pad->_radiusMin)
										continue;//失败
									if (dist2>pad->_radiusMax*pad->_radiusMax)
										continue;//失败

									//注意我们这里没有对反推点作FOV的检验
								}
								else
									continue;//失败
							}
							else
							{
								if (distToThreat2>pad->_distMaxToThreat*pad->_distMaxToThreat)
								{
									//离目标太远

									//拉到最大距离以内
									if (TRUE)
									{
										LevelPos dirProj=pos-posTarget;
										dirProj.safe_normalize();
										dirProj*=pad->_distMaxToThreat;
										pos=posTarget+dirProj;
									}

									float dist2=pos.getDistanceSQFrom(posSrc);
									if (dist2<pad->_radiusMin*pad->_radiusMin)
										continue;//失败
									if (dist2>pad->_radiusMax*pad->_radiusMax)
										continue;//失败

									//注意我们这里没有作FOV的检验
								}
							}
						}
					}
					else
					{
						float dist=CSysRandom::RandRange(pad->_distMinToThreat,pad->_distMaxToThreat);
						float faceOff=CSysRandom::RandRange(-pad->_fovThreat/2.0f,pad->_fovThreat/2.0f);
						faceOff*=i_math::GRAD_PI2;

						LevelPos dir=LevelFaceToDir(faceBase+faceOff+i_math::Pi);
						pos=posTarget+dir*dist;

					}

					CUnitMgrNavMesh *unitmgr=ctx->level->GetUnitMgr();
					if (!unitmgr)
						continue;//失败

					if (!unitmgr->IsWalkable(UnitFindPath_Walkable,pos))
						continue;//失败

					if (pad->_bUnobstructedToThreat)
					{
						if (unitmgr->StaticObstacleTest(UnitFindPath_Walkable,pos,posTarget))
							continue;
					}
					if (pad->_bUnobstructedToMe)
					{
						if (unitmgr->StaticObstacleTest(UnitFindPath_Walkable,pos,posSrc))
							continue;
					}

					if (pad->_radiusWalkable>=0.0f)
					{
						if (pad->_radiusWalkable==0.0f)
						{
							if (!unitmgr->IsWalkable(UnitFindPath_Walkable,pos))
								continue;
						}
						else
						{
							if (!unitmgr->CheckWalkableArea(UnitFindPath_Walkable,pos,pad->_radiusWalkable))
								continue;
						}
					}

					_SetPos(pad->_varPos,pos);
					_OutputOk(outputs,1,"成功");
					return;
				}
			}
		}
	}
	_OutputFail(outputs,2,"失败");
}




