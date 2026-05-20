/********************************************************************
	created:	2013/5/29 
	author:		cxi
	
	purpose:	GA功能:创建道具
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"
#include "LevelRecordItem.h"

#include "LevelUtil.h"

#include "LevelOSB.h"

#include "Bgn_小恶魔_寻找闪避跳跃点.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LoGeneralAgent.h"


#include "Log/LogDump.h"

#include "ranges/ranges.h"


////////////////////////////////////////////////////////////////////////
//CBgn_小恶魔_寻找闪避跳跃点
BIND_BGN_CLASS(CBgn_小恶魔_寻找闪避跳跃点,CBgp_小恶魔_寻找闪避跳跃点);

static BOOL StaticHitTest(LevelPos &posMe,LevelFace face,CUnitMgrNavMesh *unitmgr,CBgp_小恶魔_寻找闪避跳跃点*pad,LevelPos &posTarget)
{
	LevelPos dir=LevelFaceToDir(face);
	posTarget=posMe+dir*pad->radius;

	LevelPos posHit;
	if (unitmgr->StaticRayCast(UnitFindPath_Walkable,posMe,posTarget,posHit))
	{
		if (posMe.getDistanceSQFrom(posHit)>pad->radiusMin*pad->radiusMin)
			return TRUE;
	}
	return FALSE;
}

void CBgn_小恶魔_寻找闪避跳跃点::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_小恶魔_寻找闪避跳跃点*pad=_GetPad<CBgp_小恶魔_寻找闪避跳跃点>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();
	CUnitMgrNavMesh *unitmgr= level->GetUnitMgr();

	LevelBehaviorContext *ctx=_GetCtx();


	LevelUtilDetectParam param;
	param.loSrc=lo;
	param.pos=lo->GetFramePos();
	param.toIgnores=NULL;
	param.nIgnores=0;
	param.flags=&pad->flagsDetect[0];
	param.nFlags=pad->flagsDetect.size();
	param.requires=&pad->requires[0];
	param.nRequires=pad->requires.size();
	param.rangeMin=0.0f;
	param.rangeMax=pad->rangeDetect;
	param.weights.AddFlag(LevelDetectWeights_Dist);
	param.weights.wtDist=100.0f;
	param.bTouching=TRUE;

	DWORD c=0;
	CLevelObj **nbs=LevelUtil_Detect(param,NULL,c);

	LevelPos posMe=lo->GetFramePos();
	float radiusMe=lo->GetRadius_();
	level->GetDbgDraw().DrawCircle(posMe,radiusMe,ColorAlpha(0x0000ff,0xff),2.0f);
	CCircumRanges cr;
	for (int i=0;i<c;i++)
	{
		CLevelObj *loNb=nbs[i];
		LevelPos dir=loNb->GetFramePos()-posMe;
		float dist=(float)dir.getLength();

		float rSum=radiusMe+loNb->GetRadius_();

		LevelFace face=LevelFaceFromDir(dir);

		float scatter;
		if (rSum>dist)
			scatter=i_math::Pi/2.0f;
		else
			scatter=asin(rSum/dist);

		cr.CRanges::AddRange(face-scatter,face+scatter);

		if (TRUE)
		{
			level->GetDbgDraw().DrawCircle(loNb->GetFramePos(),loNb->GetRadius_(),ColorAlpha(0x00ff00,0xff),2.0f);
			level->GetDbgDraw().DrawLine(posMe,posMe+LevelFaceToDir(face-scatter),ColorAlpha(0xff00ff,0xff),2.0f);
			level->GetDbgDraw().DrawLine(posMe,posMe+LevelFaceToDir(face+scatter),ColorAlpha(0xff00ff,0xff),2.0f);
		}
	}

	if (cr.GetCoverRate()>=0.99f)
	{
		_OutputFail(outputs,2,"失败");
		return;
	}

	cr.Invert();

	if (pad->varHammer!=StringID_Invalid)
	{
		CLevelBehavior *bhv=lo->GetBehaviorAI();
		if (bhv)
		{
			CBehaviorMem *mem=bhv->GetMem(0);
			if (mem)
			{
				LevelObjID idHammer;
				if (mem->GetID(pad->varHammer,BehaviorMemType_ObjID,idHammer))
				{
					CLevelObj *loHammer=LevelUtil_GetAliveLo(level,idHammer);
					if (loHammer)
					{
						LevelPos posHammer=loHammer->GetFramePos();
						LevelFace faceHammer=LevelFaceFromDir(posHammer-posMe);

						LevelFace face;
						if (cr.FindClosestInRange(faceHammer,face))
						{
							LevelPos posTarget;
							if (!StaticHitTest(posMe,face,unitmgr,pad,posTarget))
							{
								level->GetDbgDraw().DrawLine(posMe,posTarget,ColorAlpha(0x00ff00,0xff),2.0f);

								_SetPos(pad->varPos,posTarget);
								_OutputOk(outputs,1,"成功");
								return;
							}
						}
					}
				}
			}
		}
	}

	for (int i=0;i<4;i++)
	{
		LevelFace face=cr.Rand();
		LevelPos posTarget;
		if (StaticHitTest(posMe,face,unitmgr,pad,posTarget))
			continue;

		level->GetDbgDraw().DrawLine(posMe,posTarget,ColorAlpha(0x00ff00,0xff),2.0f);

		_SetPos(pad->varPos,posTarget);
		_OutputOk(outputs,1,"成功");

		return;
	}

	_OutputFail(outputs,2,"失败");
	return;
}
