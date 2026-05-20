#include "stdh.h"

#include "Level.h"

#include "LevelUtil.h"

#include "LoUnit.h"

#include "EoMagicCircuitRailAbsorb.h"

#include "LevelRecords.h"

#include "LevelOSB.h"

#include "LoMagicCircuit.h"

#include "Random/Random.h"
#include "timer/timer.h"



//////////////////////////////////////////////////////////////////////////
//EoMagicCircuitRailAbsorb
BIND_EOPARAM(EoMagicCircuitRailAbsorb,EoParamMagicCircuitRailAbsorb);


void EoMagicCircuitRailAbsorb::_OnPostCreate()
{
	EoParamMagicCircuitRailAbsorb *param=GetParam<EoParamMagicCircuitRailAbsorb>();

	extern BOOL LevelUtil_AddPathToSpline(CUnitMgr * unitmgr, CCubicSpline & spline, LevelPos3D & posSrc, LevelPos3D & posTarget, BOOL bResample);
	LevelPos3D posSrc=_GetInitialPos3D();

	CLoMagicCircuit* loCircuit = (CLoMagicCircuit * )_level->GetUniqueObj(LevelUniqueObj_MagicCircuit);

	if (loCircuit)
	{
		// 找到最近的rail point作为初始目标
		LevelPos3D posTarget = loCircuit->FindClosestRailPoint(posSrc);
		
		// 直接构建到目标的路径
		LevelUtil_AddPathToSpline(_level->GetUnitMgr(), _spline, posSrc, posTarget, TRUE);
		
		//_spline可能与rail点相交,要进行截断
		if (TRUE) 
		{
			// 获取所有rail points
			LopMagicCircuit* circuitParam = loCircuit->GetLop< LopMagicCircuit>();
			const std::vector<i_math::vector3df>& railPoints = circuitParam->rail;

			// 寻找路径上距离最近的rail point
			float minDist = FLT_MAX;
			LevelPos3D closestPoint;
			bool foundIntersection = false;

			// 遍历spline上的所有点，找到与任意rail point最近的点
			float distStep = 0.25f; // 步长
			float totalDist = _spline.GetDistance();

			for (float dist = 0; dist < totalDist; dist += distStep)
			{
				LevelPos3D curPos = _spline.GetPositionByDist(dist);

				// 检查此点与所有rail points的距离
				for (const i_math::vector3df& railPoint : railPoints)
				{
					float curDist = (curPos - railPoint).getLength();
					if (curDist < minDist)
					{
						minDist = curDist;
						closestPoint = curPos;
						foundIntersection = true;
					}
				}
			}

			// 如果找到了交点，切断spline并重新构建
			if (foundIntersection && minDist < 5.0f) // 距离阈值
			{

				// 重新构建spline到交点
				_spline.Reset(FALSE);
				LevelUtil_AddPathToSpline(_level->GetUnitMgr(), _spline, posSrc, closestPoint, TRUE);
			}
		}
	}
}

void EoMagicCircuitRailAbsorb::OnDestroy()
{
	_spline.Reset(FALSE);
}


LevelPos EoMagicCircuitRailAbsorb::GetFramePos()
{
	EoParamMagicCircuitRailAbsorb *param=GetParam<EoParamMagicCircuitRailAbsorb>();

	if (!_spline.IsEmpty())
	{
		float dist = param->CalcChaseDist(ANIMTICK_TO_SECOND(_GetAge()));
		LevelPos3D pos=_spline.GetPositionByDist(dist);
		return pos.getXZ();
	}

	return _xfmInitial.pos.getXZ();
}

void EoMagicCircuitRailAbsorb::_OnWriteFirstSync(CBitPacket* bp, BOOL& bContent, LevelPlayerID idPlayer)
{
	bContent = TRUE;
	_spline.Write(*bp);
}


void EoMagicCircuitRailAbsorb::_OnPostWriteSync()
{
}


void EoMagicCircuitRailAbsorb::_OnUpdate()
{
	EoParamMagicCircuitRailAbsorb *param=GetParam<EoParamMagicCircuitRailAbsorb>();

	float tAge = ANIMTICK_TO_SECOND(_GetAge());

	float distSpline = _spline.GetDistance();
	float durSpline = param->CalcDurationForChaseDist(distSpline);

	if (!_bSignalSent)
	{
		if (tAge > durSpline)
		{
			_level->GetEventMap()->AddSignal(param->idReachSignal, GetFramePos(), 10.0f,GetID());
			_bSignalSent = TRUE;
		}
	}

	if (_bSignalSent)
	{
		if (tAge > param->CalcDurationForChaseDist(_spline.GetDistance()) + 2.0f)
		{
			DeferDestroy();
		}
	}

}

