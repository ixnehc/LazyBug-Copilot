/********************************************************************
	created:	2016/06/11 
	author:		cxi
	
	purpose:	 检查是否可以进行跳跃攻击
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelObj.h"
#include "Level.h"
#include "LevelBehavior.h"

#include "LevelAttrs.h"

#include "BgnCheckJumpAttack.h"

////////////////////////////////////////////////////////////////////////
//CBgn_DetectEquip

BIND_BGN_CLASS(CBgn_CheckJumpAttack,CBgp_CheckJumpAttack);

void CBgn_CheckJumpAttack::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_CheckJumpAttack*pad=_GetPad<CBgp_CheckJumpAttack>();

	CLevel *level=_GetLevel();
	CLevelObj *lo=_GetLo();

	if (level->GetLockers()->CheckJumpAttack())
	{
		LevelObjID idTarget=LevelObjID_Invalid;
		if (pad->nmVar!=StringID_Invalid)
			_GetID(pad->nmVar,BehaviorMemType_ObjID,idTarget);

		if (idTarget!=LevelObjID_Invalid)
		{
			extern CLevelObj *LevelUtil_GetAliveLo(CLevel*lvl,LevelObjID id);
			CLevelObj *loTarget=LevelUtil_GetAliveLo(_GetLevel(),idTarget);
			if (loTarget)
			{
				extern LevelPos LevelUtil_CalcPredictedPos(CLevelObj *loSrc,CLevelObj*loUnit,float dtPredict);
				LevelPos posTarget=LevelUtil_CalcPredictedPos(lo,loTarget,LEVEL_FRAME_INTERVAL*5.0f);
				LevelPos posSrc=_GetLo()->GetFramePos();
// 				if (posTarget.getDistanceSQFrom(loTarget->GetFramePos())<0.1f*0.1f)
				if (TRUE)
				{

					float dist2=posTarget.getDistanceSQFrom(posSrc);
					if (dist2<=pad->rangeMax*pad->rangeMax)
					{
						if (dist2>=pad->rangeMin*pad->rangeMin)
						{
							extern CLevelSkill *LevelUtil_GetCastingSkill(CLevelObj *lo);
							if (!LevelUtil_GetCastingSkill(lo))
							{
								if (!level->GetUnitMgr()->StaticObstacleTest(UnitFindPath_Walkable,posSrc,posTarget))
								{
									_OutputOk(outputs,1,"是");
									return;
								}
							}
						}
					}
				}
			}
		}
	}
	_OutputFail(outputs,2,"否");
}

