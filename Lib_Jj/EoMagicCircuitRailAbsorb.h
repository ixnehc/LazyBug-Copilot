#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "spline/CubicSpline.h"


#define CLASSUID_MagicCircuitRailAbsorb 77

extern BOOL LevelUtil_AddPathToSpline(CUnitMgr* unitmgr, CCubicSpline& spline, LevelPos3D& posSrc, LevelPos3D& posTarget, BOOL bResample);

struct EoParamMagicCircuitRailAbsorb:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamMagicCircuitRailAbsorb);

	BEGIN_GOBJ_PURE(EoParamMagicCircuitRailAbsorb,1);

		GELEM_VAR_INIT(float,speedChase,3.0f);
			GELEM_EDITVAR("Chase速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"Chase速度");
		GELEM_VAR_INIT(float,accelChase,3.0f);
			GELEM_EDITVAR("Chase加速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"Chase加速度");
		GELEM_VAR_INIT(float,speedChaseMax,8.0f);
			GELEM_EDITVAR("Chase最大速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"Chase最大速度");
		GELEM_VAR_INIT(StringID,idReachSignal,StringID_Invalid);
			GELEM_EDITVAR("信号名称", GVT_U, GSem(GSem_StringID, "信号名称"), "到达后发送的信号名称");

	END_GOBJ();

	float CalcChaseDist(float t)
	{
		float dist=0.0f;
		float dur=(speedChaseMax-speedChase)/accelChase;
		if (t<dur)
			dist=speedChase*t+0.5f*accelChase*t*t;
		else
		{
			dist=speedChase*dur+0.5f*accelChase*dur*dur+
				speedChaseMax*(t-dur);
		}
		return dist;
	}

	float CalcDurationForChaseDist(float dist)
	{
		// 计算从初始速度到最大速度所需的时间
		float dur = (speedChaseMax - speedChase) / accelChase;

		// 加速阶段能达到的距离
		float distInAccelPhase = speedChase * dur + 0.5f * accelChase * dur * dur;

		// 如果给定的距离小于在加速阶段内能达到的距离
		if (dist <= distInAccelPhase)
		{
			// 在加速阶段，解二次方程：speedChase * t + 0.5 * accelChase * t^2 = dist
			// 使用求根公式：t = (-b + sqrt(b^2 - 4ac)) / 2a
			// 其中 a = 0.5 * accelChase, b = speedChase, c = -dist
			float a = 0.5f * accelChase;
			float b = speedChase;
			float c = -dist;

			// 由于物理意义，我们只需要正根
			float t = (-b + sqrt(b * b - 4 * a * c)) / (2 * a);
			return t;
		}
		else
		{
			// 加速阶段时间 + 匀速阶段时间
			float t = dur + (dist - distInAccelPhase) / speedChaseMax;
			return t;
		}
	}

	float speedChase;
	float accelChase;
	float speedChaseMax;
	StringID idReachSignal;

};

class EoMagicCircuitRailAbsorb:public CLoEffectObj
{
public:
	EoMagicCircuitRailAbsorb()
	{
		_bSignalSent = FALSE;
	}
	DEFINE_LEVELOBJ_CLASS(EoMagicCircuitRailAbsorb,CLASSUID_MagicCircuitRailAbsorb);


	virtual const char *GetShowName()	{		return "MagicCircuitRailAbsorb";	}
	virtual LevelPos GetFramePos() override;

protected:

	void _OnPostCreate()override;
	void OnDestroy()override;

	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	void _OnPostWriteSync() override;

	void _OnUpdate()override;
	virtual BOOL _NeedOps()	{		return FALSE;	}

	CCubicSpline _spline;

	BOOL _bSignalSent;

};
