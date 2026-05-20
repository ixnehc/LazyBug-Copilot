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

#include "BgnThreat_FindFleePos.h"

#include "LevelObj.h"
#include "LevelObjMove.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"
#include "LevelTroops.h"

#include "LevelUtil.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_FindFleePos
BIND_BGN_CLASS(CBgnThreat_FindFleePos,CBgpThreat_FindFleePos);

void CBgnThreat_FindFleePos::Destroy()
{
}



void CBgnThreat_FindFleePos::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpThreat_FindFleePos*pad=_GetPad<CBgpThreat_FindFleePos>();
	CLevelObj *target=_GetThreat();
	CLevelObj *src=ctx->lo;
	UnitFindPathType tpFindPath=UnitFindPath_Walkable;

	if (ctx->level)
	{
		CUnitMgrNavMesh *unitmgr=ctx->level->GetUnitMgr();
		if (unitmgr)
		{
			if (src&&target)
			{
				if (src->IsAlive()&&target->IsAlive())
				{
					LevelPos posSrc=src->GetFramePos();

					if (unitmgr->ToClosestWalkable(tpFindPath,posSrc))
					{
						LevelPos dir=posSrc-target->GetFramePos();
						float dist=dir.getLength();
						float distToGo=pad->_dist-dist;
						float distToGoMin=1.0f;

						if (dist<0.01f)
							dir.set(0.0f,1.0f);
						else
							dir.normalize();

						int signAvoid=0;
						if (pad->_varSignAvoid!=StringID_Invalid)
						{
							short v;
							if (_GetNumber(pad->_varSignAvoid,v))
								signAvoid=v;
						}

						float euler=dir.getEuler();
						float step=10.0f*i_math::GRAD_PI2;
						int nStep=(int)(i_math::Pi*2.0f/step);
						float sign=CSysRandom::Roll(0.5f)?1.0f:-1.0f;

						LevelPos posTarget,posHit;
						BOOL bFound=FALSE;

						for (int i=0;i<nStep;i++)
						{
							if (signAvoid==0)
							{
								euler=euler+step*sign*(float)i;
								signAvoid=sign>0.0f?1:-1;
								sign*=-1.0f;
							}
							else
								euler=euler+step*(float)signAvoid;

							dir.setEuler(euler);

							dir*=distToGo;
							posTarget=target->GetFramePos()+dir;

							//限制到center范围内
							if (pad->_radiusToOwner>0.0f)
							{
								extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
								CLevelObj *loOwner=LevelUtil_GetOwnerLo(src);
								if (loOwner)
								{
									LevelPos posCenter=loOwner->GetFramePos();
									if (posTarget.getDistanceSQFrom(posCenter)>pad->_radiusToOwner*pad->_radiusToOwner)
									{
										LevelPos dir=posTarget-posCenter;
										dir.normalize();
										dir*=pad->_radiusToOwner;
										posTarget=posCenter+dir;
									}
								}
							}

							if (unitmgr->StaticRayCast(tpFindPath,posSrc,posTarget,posHit))
							{
								if (posSrc.getDistanceSQFrom(posHit)<distToGoMin*distToGoMin)
									continue;//这个方向上卡住了,继续尝试
								posTarget=posHit;
							}

							bFound=TRUE;
							break;
						}

						if (!bFound)
							signAvoid=0;

						if (pad->_varSignAvoid!=StringID_Invalid)
							_SetNumber(pad->_varSignAvoid,(short)signAvoid);

						if (bFound)
						{
							_SetPos(pad->_varPos,posTarget);
							_OutputOk(outputs,1,"成功");
							return;
						}
					}
				}
			}
		}
	}
	_OutputFail(outputs,2,"失败");
}




