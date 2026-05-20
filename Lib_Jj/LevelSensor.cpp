
#include "stdh.h"
#include "Level.h"

#include "LevelUtil.h"

#include "LevelSensor.h"
#include "LevelTroops.h"

#include "LevelObjMove.h"

#include "LevelEventSrc.h"

#include "LevelAIContext.h"

#include "datapacket/BitPacket.h"


//////////////////////////////////////////////////////////////////////////
//CLevelSensor
#define OWNER_SIGHT 20.0f

void CLevelSensor::Create(CLevelObj *lo,LevelSensorParam *param)
{
	_lo=lo;
	_param=param;
}

void CLevelSensor::Destroy()
{
	_RecordThreat(NULL);
	SAFE_RELEASE(_fail);
	Zero();
}


void CLevelSensor::_FillDetectParam(LevelUtilDetectParam &param,CLevelObj *lo)
{
	param.loSrc=lo;
	param.pos=lo->GetFramePos();
	param.rangeMin=0.0f;
	param.rangeMax=_param->range;
	param.flags=&_param->flagsDetect[0];
	param.nFlags=_param->flagsDetect.size();
	param.requires=&_param->requires[0];
	param.nRequires=_param->requires.size();
	param.weights.CopyFrom(_param->weightsDetect);
// 	param.weights.OverrideFrom(pad->weights);

	param.bTouching=TRUE;
}

TroopCombatContext *CLevelSensor::_GetTcc()
{
	CLevelTroop *troop=_lo->GetTroop();
	if (troop)
		return troop->GetCombatContext();

	return NULL;
}

BOOL CLevelSensor::_CheckOwnerSight(CLevelObj *lo,float dist2)
{
	if (!_owner)
		return TRUE;

	if (_owner->GetFramePos().getDistanceSQFrom(lo->GetFramePos())>=OWNER_SIGHT*OWNER_SIGHT)
		return FALSE;
	return TRUE;
}



LevelObjMapEnumCallBack CLevelSensor::_GetDetectDlgt()
{
	if (_owner)
	{
		LevelObjMapEnumCallBack dlgt;
		dlgt.bind(this,&CLevelSensor::_CheckOwnerSight);
		return dlgt;
	}
	return NULL;
}


extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt,std::vector<CLevelObj *>&candidates);
extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
CLevelObj *CLevelSensor::_DetectBest(CLevelSensor::DetectMethod method,DetectSightType tpSight)
{
	if ((method==Detect_Closer)&&(!_threat))//当前没有目标,切换为在视野内侦测
		method=Detect_Sight;

	if (method==Detect_Sight)
	{
		if ((tpSight==DetectSightType_Me)||(tpSight==DetectSightType_Owner))
		{
			CLevelObj *loSrc=_lo;
			extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
			if (tpSight!=DetectSightType_Me)
			{
				loSrc=LevelUtil_GetOwnerLo(_lo);
				if (!loSrc)
					loSrc=_lo;
			}

			extern CLevelObj *LevelUtil_DetectBest(LevelUtilDetectParam &param,LevelObjMapEnumCallBack dlgt);
			LevelUtilDetectParam param;
			_FillDetectParam(param,loSrc);
			param.fail=_fail;
			param.recent=_threat;
			if (tpSight==DetectSightType_Owner)
				param.rangeMax=OWNER_SIGHT;
			return LevelUtil_DetectBest(param,NULL);
		}
		if (tpSight==DetectSightType_Troop)
		{
			TroopCombatContext *tcc=_GetTcc();
			if (!tcc)
				return NULL;

			LevelUtilDetectParam param;
			_FillDetectParam(param,_lo);
			param.rangeMax=10000000.0f;//very big value
			return LevelUtil_DetectBest(param,NULL,tcc->detects);
		}
	}

	if (method==Detect_Closer)
	{
		if ((tpSight==DetectSightType_Me)||(tpSight==DetectSightType_Owner))
		{
			CLevelObj *loSrc=_lo;
			extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
			if (tpSight==DetectSightType_Owner)
				_owner=loSrc=LevelUtil_GetOwnerLo(_lo);

			float radius=_threat->GetFramePos().getDistanceFrom(loSrc->GetFramePos());

			LevelUtilDetectParam param;
			_FillDetectParam(param,loSrc);
			param.rangeMax=radius;
			param.fail=_fail;
			param.recent=_threat;
			CLevelObj *loDetected=LevelUtil_DetectBest(param,_GetDetectDlgt());

			_owner=NULL;
			return loDetected;
		}

		if (tpSight==DetectSightType_Troop)
		{
			float radius=_threat->GetFramePos().getDistanceFrom(_lo->GetFramePos());

			TroopCombatContext *tcc=_GetTcc();
			if (!tcc)
				return NULL;

			LevelUtilDetectParam param;
			_FillDetectParam(param,_lo);
			param.rangeMax=radius;
			return LevelUtil_DetectBest(param,NULL,tcc->detects);
		}
	}

	return NULL;

}

void CLevelSensor::_RecordThreat(CLevelObj *loThreat)
{
	if (_threat)
	{
		LevelAIContext *ctx=_threat->GetAIContext();
		if (ctx)
			ctx->RemoveThreating(_lo->GetID());
	}

	SAFE_REPLACE(_threat,loThreat);

	if (loThreat)
	{
		LevelAIContext *ctx=_threat->ObtainAIContext();
		if (ctx)
			ctx->AddThreating(_lo->GetID());
	}

}

float CLevelSensor::_GetMaxDetectRange()
{
	switch(_tpSight)
	{
		case DetectSightType_Me:
			return _param->range;
		case DetectSightType_Owner:
			return OWNER_SIGHT;
		case DetectSightType_Troop:
			return 10000000.0f;//very big value
	}
	return 0.0f;
}

void CLevelSensor::BeginOverrideThreat()
{
	_bThreatOverriden=TRUE;
}

void CLevelSensor::OverrideThreat(CLevelObj *loThreat,AnimTick t)
{
	if (_bThreatOverriden)
	{
		_RecordThreat(loThreat);
		_tLastDetect=t;
	}
}

void CLevelSensor::EndOverrideThreat()
{
	_bThreatOverriden=FALSE;
}

void CLevelSensor::_UpdateFail()
{
	VerifyLevelObjAlive(_fail);
	if (_fail)
	{
		AnimTick durForgetFail=ANIMTICK_FROM_SECOND(2.0f);
		if (_lo->GetT()>_tFail+durForgetFail)
		{
			SAFE_RELEASE(_fail);
		}
	}
}

void CLevelSensor::_UpdateDetectSightType()
{
	_tpSight=DetectSightType_Me;

	if (_GetTcc())
		_tpSight=DetectSightType_Troop;
	else
	{
		if (_param->bUseOwnerSight)
		{
			extern CLevelObj *LevelUtil_GetOwnerLo(CLevelObj *lo);
			if (LevelUtil_GetOwnerLo(_lo))
				_tpSight=DetectSightType_Owner;
		}
	}
}

void CLevelSensor::_UpdateDetect(DetectMethod method)
{
	CLevelObj *loDetect=_DetectBest(method,_tpSight);
	_tLastDetect=_lo->GetT();

	if (loDetect)
	{
		if (loDetect==_threat)
			loDetect=NULL;
	}

	if (loDetect)
		_RecordThreat(loDetect);
}


void CLevelSensor::Update()
{
	if (!IsActive())
		return;

	AnimTick t=_lo->GetT();
	CLevelEventSrc *dmgsrc=_lo->GetEventSrc();

	//判断是不是要取消_fail
	_UpdateFail();

	if (_bThreatOverriden)
		return;

	CLevelSkillDriver *driver=_lo->GetSkillDriver();
	if (!driver)
		return;

	_UpdateDetectSightType();

	VerifyLevelObjAlive(_threat);

	//判断是否需要重新寻找攻击目标
	if (TRUE)
	{
		DetectMethod methodReDetect=Detect_None;
		if (!driver->IsWorking())
		{//技能已经结束了
			methodReDetect=Detect_Sight;
			if (driver->IsFailReach())
			{
				if (_threat)
				{
					if (driver->GetFailTarget()==_threat->GetID())
					{
						SAFE_REPLACE(_fail,_threat);
						_tFail=t;
					}
				}
			}
			_RecordThreat(NULL);
		}
		else
		{
			//技能正在进行中(正在跟踪目标,或者正在释放技能)
			BOOL bNeedUpdate=TRUE;
			if (_param->bDisableWhenCastingSkill&&driver->IsSkillCasting()&&_threat)//如果当前没有threat,会强制更新一下
				bNeedUpdate=FALSE;

			if (bNeedUpdate)
			{
				//看看threat是否离得太远
				if (_threat)
				{
					BOOL bTooFar=FALSE;
					float rangeMax=_GetMaxDetectRange();
					float rangeMaxExt=rangeMax*1.4f;
					if (_tpSight==DetectSightType_Me)
					{
						float dist2=_threat->GetFramePos().getDistanceSQFrom(_lo->GetFramePos());
						if (dist2>rangeMaxExt*rangeMaxExt)
							bTooFar=TRUE;
					}
					if (_tpSight==DetectSightType_Owner)
					{
						CLevelObj *owner=LevelUtil_GetOwnerLo(_lo);
						if (owner)
						{
							float dist2=_threat->GetFramePos().getDistanceSQFrom(owner->GetFramePos());
							if (dist2>rangeMaxExt*rangeMaxExt)
								bTooFar=TRUE;
						}
					}
					if (bTooFar)
					{
						//放弃这个Threat
						driver->StopMove();
						_RecordThreat(NULL);
					}
				}

				if (_threat)
				{
					if (dmgsrc)
					{
						//受击后立即更新
						if (dmgsrc->Exist(LET_Damage,t-ANIMTICK_FROM_SECOND(0.2f)))
						{
							methodReDetect=Detect_Sight;
						}
					}

					if (methodReDetect==Detect_None)
					{
						AnimTick durUpdate=ANIMTICK_FROM_SECOND(0.5f);

						if (t>_tLastDetect+durUpdate)
						{//隔一段时间更新
							methodReDetect=Detect_Closer;
						}
					}
				}
				else
					methodReDetect=Detect_Sight;//找范围内的
			}
		}
		if (methodReDetect!=Detect_None)
			_UpdateDetect(methodReDetect);
	}
}

void CLevelSensor::ForceUpdate()
{
	_UpdateFail();
	_UpdateDetectSightType();

	VerifyLevelObjAlive(_threat);

	_UpdateDetect(Detect_Sight);
}

