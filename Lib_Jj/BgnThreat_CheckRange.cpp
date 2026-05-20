/********************************************************************
	created:	2016/09/15 
	author:		cxi
	
	purpose:	 检测Threat范围
*********************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "BgnThreat_CheckRange.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelSkillDriver.h"

#include "LevelUtil.h"

#include "Log/LogDump.h"


////////////////////////////////////////////////////////////////////////
//CBgnThreat_CheckRange
BIND_BGN_CLASS(CBgnThreat_CheckRange,CBgpThreat_CheckRange);

void CBgnThreat_CheckRange::Start(DWORD iStb,BGNOutputs &outputs)
{
	LevelBehaviorContext *ctx=_GetCtx();
	CBgpThreat_CheckRange*pad=_GetPad<CBgpThreat_CheckRange>();

	LevelPos posTarget;
	if (!_GetLevelSkillTarget_Pos(pad->_target,posTarget))
	{
		_OutputFail(outputs,2,"否");
		return;
	}

	CLevelObj *target=_GetLevelSkillTarget_Obj(pad->_target);

	CLevelObj *lo=_GetLo();

	LevelFace faceMe=lo->GetFrameFace();
	LevelPos posMe=lo->GetFramePos();

	BOOL bInRange=FALSE;
	float dist=posTarget.getDistanceFrom(posMe);
	if (pad->_bConsiderUnitRadius)
	{
		dist-=lo->GetRadius_();
		if (target)
			dist-=target->GetRadius_();
	}
	if (dist<0.0f)
		dist=0.0f;
	if ((dist>=pad->_rngRadius.low)&&(dist<=pad->_rngRadius.hi))
	{
		float face=atan2f(posTarget.y-posMe.y,posTarget.x-posMe.x);
		face=i_math::normalize_radian(face-faceMe)*i_math::GRAD_PI;

		face=-face;

		if (pad->_rngFace.low<=pad->_rngFace.hi)
		{
			if ((face>=pad->_rngFace.low)&&(face<=pad->_rngFace.hi))
				bInRange=TRUE;
		}
		else
		{
			if (!((face>=pad->_rngFace.hi)&&(face<=pad->_rngFace.low)))
				bInRange=TRUE;
		}
	}

	if(bInRange)
		_OutputOk(outputs,1,"是");
	else
		_OutputFail(outputs,2,"否");
}


