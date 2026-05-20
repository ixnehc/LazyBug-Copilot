/********************************************************************
	created:	2012/11/24 
	author:		cxi
	
	purpose:	 对话相关的BGN
*********************************************************************/
#include "stdh.h"

#include "LevelBGs.h"

#include "LevelObjMap.h"
#include "LevelBehavior.h"
#include "LevelObj.h"
#include "LevelObjMove.h"
#include "Level.h"
#include "BgnFlee.h"

#include "LevelUtil.h"

#include "Random/Random.h"


////////////////////////////////////////////////////////////////////////
//CBgn_Flee
BIND_BGN_CLASS(CBgn_Flee,CBgp_Flee);

void CBgn_Flee::Destroy()
{
	SAFE_DESTROY(_anSpeed);
}


void CBgn_Flee::_UpdateFlee(AnimTick t)
{
	if (_bFinished)
		return;
	CBgp_Flee*pad=_GetPad<CBgp_Flee>();
	if (pad->dur!=0)
	{
		if (t>_tStart+pad->dur)
		{
			_bFinished=TRUE;
			return;
		}
	}

	if (_nCDFrames>0)
	{
		_nCDFrames--;
		return;
	}

// 	if (t>_tResetAvoidSign)
// 	{
// 		_signAvoid=0;
// 		_tResetAvoidSign=t+ANIMTICK_FROM_SECOND(CSysRandom::RandRange(3.0f,6.0f));
// 	}

	CLevelObj *lo=_GetLo();
	CLevelObj *loDetect=NULL;
	if (lo)
	{
		CLevelObjMap *om=lo->GetLevel()->GetObjMap();
		LevelObjRequire requires[]={LevelObjRequire_Attackable};

		LevelUtilDetectParam param;
		param.loSrc=lo;
		param.flags=&pad->flagsDetect[0];
		param.nFlags=pad->flagsDetect.size();
		param.requires=requires;
		param.nRequires=ARRAY_SIZE(requires);
		param.rangeMin=0.0f;
		param.rangeMax=pad->radius;
		param.pos=lo->GetFramePos();
		param.weights.AddFlag(LevelDetectWeights_Dist);
		param.weights.wtDist=100.0f;
		param.weights.distRef=pad->radius;

		extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
		loDetect=LevelUtil_DetectBest(param,NULL);
	}

	if (loDetect)
	{
		extern BOOL LevelUtil_Flee(CLevelObj * lo, CLevelObj * loEscapeFrom, float distKeep, int& signAvoid, CLevelObj * loCenter, float radius);
		if (pad->radiusFollow<=0.0f)
			LevelUtil_Flee(lo,loDetect,pad->radius+1.0f,_signAvoid,NULL,0);
		else
			LevelUtil_Flee(lo,loDetect,pad->radius+1.0f,_signAvoid,_GetLockLo(),pad->radiusFollow);

		_nCDFrames=1;//每隔1帧更新一次
	}
	else
	{
		if (pad->bQuitWhenOutOfRange)
			_bFinished=TRUE;
	}

}

void CBgn_Flee::Start(DWORD iStb,BGNOutputs &outputs)
{
	_tStart=_GetT();

	CBgp_Flee*pad=_GetPad<CBgp_Flee>();

	if (pad->speed>0.0f)
	{
		CLevelObj *lo=_GetLo();
		if (lo)
		{
			CLevelObjMove *move=lo->GetMove();
			if (move)
			{
				SpeedMod *mod=move->ObtainSpeedMod();
				if (mod)
					_anSpeed=(AttrNodeFloat *)mod->speed.Add(pad->speed,200);
			}
		}
	}


	_UpdateFlee(_tStart);
}

void CBgn_Flee::Update(BGNOutputs &outputs)
{
	_UpdateFlee(_GetT());
	if (_bFinished)
	{
		_OutputOk(outputs,1,"结束");
	}
}

