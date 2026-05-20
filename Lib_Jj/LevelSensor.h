#pragma once

#include "LevelDefines.h"

#include "strlib/strlibdefines.h"

#include "bitset/bitset.h"

#include "LevelDetectTargetFlags.h"
#include "LevelDetectWeights.h"

#include "LevelObjMap.h"

#include "LevelObj.h"

class CLevelObj;

struct LevelSensorParam
{
	LevelSensorParam()
	{
		range=10.0f;
		bUseOwnerSight=TRUE;
		bDisableWhenCastingSkill=TRUE;
	}
	float range;
	std::vector<LevelDetectTargetFlag> flagsDetect;
	std::vector<LevelObjRequire> requires;
	LevelDetectWeights weightsDetect;
	BOOL bUseOwnerSight;
	BOOL bDisableWhenCastingSkill;

};

struct LevelUtilDetectParam;
struct TroopCombatContext;
class CLevelSensor
{
public:
	DEFINE_CLASS(CLevelSensor);
	CLevelSensor()
	{
		Zero();
	}
	void Zero()
	{
		_lo=NULL;
		_owner=NULL;
		_param=NULL;
		_threat=NULL;

		_fail=NULL;
		_tFail=0;

		_tLastDetect=0;

		_tpSight=DetectSightType_Me;

		_bThreatOverriden=FALSE;
	}

	enum DetectMethod
	{
		Detect_None=0,
		Detect_Sight,//在视野范围内重新搜索
		Detect_Closer,//在比_threat更近的范围内重新搜索
	};


	void Create(CLevelObj *lo,LevelSensorParam *param);
	void Destroy();

	LevelObjID GetThreatId()	{		return _threat?_threat->GetID():LevelObjID_Invalid;	}
	CLevelObj *GetThreat()	{		return _threat;	}

	void PushActive(BOOL bActive)	{		_actives.push(bActive);	}
	void PopActive()	{		_actives.pop();	}
	BOOL IsActive()	{		return _actives.cur();	}

	void Update();

	void ForceUpdate();

	void BeginOverrideThreat();
	void OverrideThreat(CLevelObj *loThreat,AnimTick t);
	void EndOverrideThreat();

protected:
	void _FillDetectParam(LevelUtilDetectParam &param,CLevelObj *lo);
	CLevelObj *_DetectBest(DetectMethod method,DetectSightType tpSight);
	TroopCombatContext *_GetTcc();
	BOOL _CheckOwnerSight(CLevelObj *lo,float dist2);
	LevelObjMapEnumCallBack _GetDetectDlgt();
	void _RecordThreat(CLevelObj *loThreat);
	float _GetMaxDetectRange();
	void _UpdateFail();
	void _UpdateDetectSightType();
	void _UpdateDetect(DetectMethod method);

	BitsetStack<1,TRUE> _actives;


	CLevelObj *_lo;
	CLevelObj *_owner;
	LevelSensorParam *_param;

	BOOL _bThreatOverriden;
	CLevelObj *_threat;
	CLevelObj *_fail;
	AnimTick _tFail;


	DetectSightType _tpSight;

	AnimTick _tLastDetect;//上一次Detect的时间

};


