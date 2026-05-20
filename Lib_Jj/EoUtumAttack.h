#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "spline/CubicSpline.h"



class CPathSpline
{
public:
	void Reset()
	{
		_spline.Reset(FALSE);
		_pthConfirmed.clear();
	}
	void AddInitialPos(i_math::vector2df &pos)
	{
		i_math::vector3df pos3D;
		pos3D.setXZ(pos);
		AddInitialPos(pos3D);
	}
	void AddInitialPos(i_math::vector3df &pos)
	{
		_pthConfirmed.push_back(pos);
	}

	BOOL Build(CUnitMgr *unitmgr,i_math::vector2df &posTarget)
	{
		i_math::vector3df pos3D;
		pos3D.setXZ(posTarget);
		return Build(unitmgr,pos3D);
	}

	BOOL Construct(CUnitMgr *unitmgr,i_math::vector3df &posTarget)
	{
		_spline.Reset(FALSE);
		for (int i=0;i<_pthConfirmed.size()-1;i++)
			_spline.AddNode(_pthConfirmed[i],i_math::quatf());

		extern BOOL LevelUtil_AddPathToSpline(CUnitMgr *unitmgr, CCubicSpline &spline,LevelPos3D &posSrc,LevelPos3D &posTarget,BOOL bResample);
		return LevelUtil_AddPathToSpline(unitmgr,_spline,_pthConfirmed[_pthConfirmed.size()-1],posTarget,TRUE);
	}

	BOOL Build(CUnitMgr *unitmgr,i_math::vector3df &posTarget)
	{
		BOOL bRet=Construct(unitmgr,posTarget);
		_spline.BuildSNS();
		return bRet;

		// 		for (int i=0;i<_spline.GetNodeCount();i++)
		// 		{
		// 			LevelPos3D pos=_spline.GetNode(i)->position;
		// 			g_gs.dbgdraw->DrawSphere(pos,0.04f,0xffffffff);
		// 			if (i+1<_spline.GetNodeCount())
		// 			{
		// 				LevelPos3D pos2=_spline.GetNode(i+1)->position;
		// 				g_gs.dbgdraw->DrawLine(pos,pos2,0xffffffff);
		// 			}
		// 		}

	}

	float GetDistance()
	{
		return _spline.GetDistance();
	}

	void Sample(float dist,i_math::vector3df &pos,i_math::vector3df &vel)
	{
		float time=i_math::clamp_f(dist/_spline.GetDistance(),0.0f,1.0f);
		pos=_spline.GetPosition(time);
		vel=_spline.GetVelocity(time);
	}

	void Sample(float dist,i_math::vector3df &pos)
	{
		float time=i_math::clamp_f(dist/_spline.GetDistance(),0.0f,1.0f);
		pos=_spline.GetPosition(time);
	}

	void Sample(float dist,i_math::vector2df &pos)
	{
		i_math::vector3df pos3D;
		Sample(dist,pos3D);
		pos=pos3D.getXZ();
	}

	void Confirm(float dist)
	{
		float time=i_math::clamp_f(dist/_spline.GetDistance(),0.0f,1.0f);

		int idx=_spline.GetCurNode(time);
		if (idx+1<_spline.GetNodeCount())
			idx++;
		int start=_pthConfirmed.size();
		for (int i=start;i<=idx;i++)
			_pthConfirmed.push_back(_spline.GetNode(i)->position);
	}


public:
	CCubicSpline _spline;
	std::vector<i_math::vector3df> _pthConfirmed;
};


#define CLASSUID_UtumAttack 57

#define UTUMATTACK_RADIUS_CHASE_ANCHOR (1.f)


struct EoParamUtumAttack:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamUtumAttack);

	BEGIN_GOBJ_PURE(EoParamUtumAttack,1);

		GELEM_VAR_INIT(float,radiusTakeOff,4.0f);
			GELEM_EDITVAR("TakeOff半径",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"TakeOff半径");
		GELEM_VAR_INIT(float,radiusTakeOffVary,1.0f);
			GELEM_EDITVAR("TakeOff半径浮动",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"TakeOff半径浮动");
		GELEM_VAR_INIT(float,htTakeOff,2.50f);
			GELEM_EDITVAR("TakeOff高度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"TakeOff高度");
		GELEM_VAR_INIT(float,htTakeOffVary,0.5f);
			GELEM_EDITVAR("TakeOff高度浮动",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"TakeOff高度浮动");
		GELEM_VAR_INIT(float,speedTakeOff,6.0f);
			GELEM_EDITVAR("TakeOff速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"TakeOff速度");
		GELEM_VAR_INIT(float,radiusChase,5.0f);
			GELEM_EDITVAR("Chase半径",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"Chase半径");
		GELEM_VAR_INIT(float,speedChase,3.0f);
			GELEM_EDITVAR("Chase速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"Chase速度");
		GELEM_VAR_INIT(float,accelChase,3.0f);
			GELEM_EDITVAR("Chase加速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"Chase加速度");
		GELEM_VAR_INIT(float,speedChaseMax,8.0f);
			GELEM_EDITVAR("Chase最大速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"Chase最大速度");
		GELEM_VAR_INIT(float,durChase,4.0f);
			GELEM_EDITVAR("Chase最长时间",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"Chase最长时间");

		GELEM_VAR_INIT(float,rangeAttack,5.0f);
			GELEM_EDITVAR("攻击范围",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"攻击范围");


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

	float radiusTakeOff;
	float radiusTakeOffVary;
	float htTakeOff;
	float htTakeOffVary;
	float speedTakeOff;
	float radiusChase;
	float speedChase;
	float accelChase;
	float speedChaseMax;
	float durChase;
	float rangeAttack;

};

class EoUtumAttack:public CLoEffectObj
{
public:
	EoUtumAttack()
	{
		_bSyncDirty=FALSE;
	}
	DEFINE_LEVELOBJ_CLASS(EoUtumAttack,CLASSUID_UtumAttack);

	struct State
	{
		State()
		{
			stage=None;
		}

		enum Stage
		{
			None,
			TakeOff,
			Chase,
			Return,
		};
		Stage stage;
		AnimTick tStageStart;

		LevelPos3D posTakeOffSrc;
		LevelPos3D posTakeOffTarget;
		float durTakeOff;

		LevelObjID idChaseTarget;
		LevelPos posChaseTargetCache;
		LevelPos posChaseSrc;
		LevelFace faceChaseSrc;
		LevelFace faceChaseAnchor;

		BOOL bDamaged;

	};

	virtual const char *GetShowName()	{		return "Utum攻击";	}
	virtual LevelPos GetFramePos() override;

	BOOL HasDamaged()	{		return _state.stage==State::Return&&_state.bDamaged;	}

protected:

	void _OnPostCreate()override;
	void OnDestroy()override;


	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	void _OnWriteSyncL(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;
	void _OnPostWriteSync() override;

	void _OnUpdate()override;
	virtual BOOL _NeedOps()	{		return FALSE;	}

	void _WriteState(CBitPacket *bp);

	BOOL _GetTakeOffTargetPos(LevelFace face,LevelPos &posTarget);
	BOOL _TakeOff();
	void _Chase(LevelPos &posSrc,LevelPos &dirSrc,CLevelObj *loTarget);
	void _Return(BOOL bDamaged);
	float _CalcChaseDist();

	State _state;
	BOOL _bSyncDirty;

	CPathSpline _spline;

};
