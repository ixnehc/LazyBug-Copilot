/********************************************************************
	created:	2020/8/21 
	author:		cxi
	
	*******************************************************************/
#include "stdh.h"
#include "commondefines/general_stl.h"

#include "Level.h"
#include "LevelBehavior.h"

#include "LevelRecords.h"

#include "LevelUtil.h"


#include "Bgn_地狱触手_侦测干扰.h"

#include "LevelObj.h"
#include "LevelBGs.h"

#include "LevelUnitArg.h"
#include "LoUnit.h"
#include "LevelSensor.h"

#include "Buff_TongueFly.h"
#include "LoCentipede.h"

static BOOL dlgt__地狱触手_侦测干扰(CLevelObj *lo,float dist2)
{
	if (lo->GetType()!=LevelObjType_Unit)
		return FALSE;

	RecordID id=((CLoUnit*)lo)->GetRecID();
	CLoCentipede*loCentipede=(CLoCentipede*)lo->GetLevel()->Get1stCentipede();
	if (loCentipede)
	{
		if (id==loCentipede->GetNodeUnitID())
			return TRUE;
	}

	return FALSE;

}

BOOL func_地狱触手_侦测干扰(CLevelObj *loSrc,LevelPos pos,float radius)
{
	LevelObjMapEnumCallBack dlgt;
	dlgt.bind(&dlgt__地狱触手_侦测干扰);

	LevelUtilDetectParam param;

	param.loSrc=loSrc;
	param.pos=pos;
	param.rangeMin=0.0f;
	param.rangeMax=radius;
	LevelDetectTargetFlag flags=((LevelDetectTargetFlag)(LevelDetectTarget_Ally|LevelDetectTarget_Native|LevelDetectTarget_Neutral|
		LevelDetectTarget_Unit|LevelDetectTarget_Ground));
	param.flags=&flags;
	param.nFlags=1;
	LevelObjRequire requires=LevelObjRequire_Attackable;
	param.requires=&requires;
	param.nRequires=1;
	param.weights.Zero();
	param.bTouching=TRUE;

	CLevelObj *loDetect=LevelUtil_DetectFirst(param,dlgt);
	if(loDetect)
		return TRUE;

	if (TRUE)
	{
		extern Buff_TongueFly *FindTongueFlyBuff(CLevel *level);
		Buff_TongueFly *buff=FindTongueFlyBuff(loSrc->GetLevel());
		if (buff)
		{
			LevelPos buf[128];
			DWORD c=buff->GetTongueNodesPos(buf,ARRAY_SIZE(buf));

			float radiusDetect=radius+buff->GetTongueNodeRadius();

			for (int i=0;i<c;i++)
			{
				if (pos.getDistanceFrom(buf[i])<radiusDetect)
				{
					return TRUE;
				}
			}
		}
	}

	return FALSE;

}

////////////////////////////////////////////////////////////////////////
//CBgn_地狱触手_侦测干扰
BIND_BGN_CLASS(CBgn_地狱触手_侦测干扰,CBgp_地狱触手_侦测干扰);

void CBgn_地狱触手_侦测干扰::Start(DWORD iStb,BGNOutputs &outputs)
{
	CBgp_地狱触手_侦测干扰*pad=_GetPad<CBgp_地狱触手_侦测干扰>();
	CLevelObj *lo=_GetLo();
	CLevel *level=_GetLevel();

	LevelBehaviorContext *ctx=_GetCtx();

	if (func_地狱触手_侦测干扰(lo,lo->GetFramePos(),pad->_radius+lo->GetRadius_()))
	{
		_OutputOk(outputs,1,"成功");
		return;
	}

	_OutputFail(outputs,2,"失败");

	return;
}
