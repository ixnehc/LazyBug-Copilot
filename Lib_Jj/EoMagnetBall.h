#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#define CLASSUID_MagnetBall 30

struct EoParamMagnetBall:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamMagnetBall);

	BEGIN_GOBJ_PURE(EoParamMagnetBall,1);

		GELEM_VAR_INIT(float,radius,0.2f);
			GELEM_EDITVAR("半径",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"半径");
		GELEM_VAR_INIT(float,ht,1.0f);
			GELEM_EDITVAR("高度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"离地高度");
		GELEM_VAR_INIT(float,htVary,0.0f);
			GELEM_EDITVAR("高度浮动",GVT_F,GSem(GSem_Float,"0.00,100,0.01"),"离地高度浮动值");
		GELEM_VAR_INIT(float,speedHor,5.0f);
			GELEM_EDITVAR("水平速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"速度");
		GELEM_VAR_INIT(float,speedHorVary,0.0f);
			GELEM_EDITVAR("水平速度浮动",GVT_F,GSem(GSem_Float,"0.00,100,0.01"),"水平速度浮动值");
		GELEM_VAR_INIT(float,rotMax,180.0f);
			GELEM_EDITVAR("最大转速",GVT_F,GSem(GSem_Float,"0.01,3600,0.1"),"度/秒");
		GELEM_VAR_INIT(float,rotMaxDec,20.0f);
			GELEM_EDITVAR("最大转速衰减",GVT_F,GSem(GSem_Float,"0.01,3600,0.1"),"度/秒平方");
		GELEM_VAR_INIT(float,speedVerInitial,0.0f);
			GELEM_EDITVAR("垂直初速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"速度");
		GELEM_VAR_INIT(float,speedVerInitialVary,0.0f);
			GELEM_EDITVAR("垂直初速度浮动",GVT_F,GSem(GSem_Float,"0.0,100,0.01"),"垂直初速度浮动");
		GELEM_VAR_INIT(float,g,10.0f);
			GELEM_EDITVAR("重力加速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"重力加速度");
		GELEM_VAR_INIT(DWORD,nBumps,2);
			GELEM_EDITVAR("弹跳次数",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"弹跳次数");
		GELEM_VAR_INIT(float,dampHorBump,1.0f);
			GELEM_EDITVAR("水平速度弹跳衰减",GVT_F,GSem(GSem_Float,"0.0,1,0.01"),"弹跳系数");
		GELEM_VAR_INIT(float,rateBump,0.3f);
			GELEM_EDITVAR("弹跳系数",GVT_F,GSem(GSem_Float,"0.0,1,0.01"),"弹跳系数");
		GELEM_VAR_INIT(BOOL,bTrace,TRUE);
			GELEM_EDITVAR("跟踪目标",GVT_S,GSem_Boolean,"是否跟踪目标");
// 		GELEM_VAR_INIT(AnimTick,tStartTrace,ANIMTICK_FROM_SECOND(0.0f));
// 			GELEM_EDITVAR("起始跟踪时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"多久后开始跟踪");
		GELEM_VAR_INIT(AnimTick,delayExplodeDmg,ANIMTICK_FROM_SECOND(0.1f));
			GELEM_EDITVAR("爆炸伤害延迟",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"Explode后多久开始产生伤害");
		GELEM_VAR_INIT(float,radiusExplode,1.0f);
			GELEM_EDITVAR("爆炸范围",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"爆炸范围");
		GELEM_VAR_INIT(AnimTick,durAutoExplode,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("自动爆炸延迟",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"0表示不会自动爆炸");
	END_GOBJ();

	float radius;
	float fall;
	float ht;
	float htVary;
	float speedHor;
	float speedHorVary;
	float speedVerInitial;
	float speedVerInitialVary;
	float rotMax;
	float rotMaxDec;
	float g;
	int nBumps;
	float dampHorBump;//弹跳水平速度衰减
	float rateBump;//弹跳系数
	BOOL bTrace;
// 	AnimTick tStartTrace;//多久后开始跟踪
	float radiusExplode;
	AnimTick delayExplodeDmg;
	AnimTick durAutoExplode;
};

class CMagnetBall
{
public:
	CMagnetBall()
	{
		_level=NULL;
		_param=NULL;
		_bTargetPos=FALSE;
	}
	struct State
	{
		State()
		{
			t=0;
			face=0.0f;
			rotMax=0.0f;
			spdHor=0.0f;
			spdVer=0.0f;
			nBumps=0;
			bSupport=FALSE;
			bHitStatic=FALSE;
		}
		AnimTick t;
		LevelPos3D pos;
		BOOL bSupport;
		float spdHor;
		float spdVer;
		float face;
		float rotMax;
		int nBumps;
		BOOL bHitStatic;
	};
	void Init(CLevel *level,EoParamMagnetBall *param,LevelPos &pos,float face,AnimTick t);
	void SetTargetPos(LevelPos &posTarget)
	{
		_posTarget=posTarget;
		_bTargetPos=TRUE;
	}
	void Update(AnimTick t);
	State &GetState()
	{
		return _state;
	}
protected:

	CLevel *_level;

	EoParamMagnetBall *_param;
	BOOL _bTargetPos;
	LevelPos _posTarget;

	State _state;
};


class EoMagnetBall:public CLoEffectObj
{
public:
	EoMagnetBall()
	{
		_idTarget=LevelObjID_Invalid;
		_bExplode=FALSE;
		_bDamage=FALSE;
		_tExplode=0;
		_tStart=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoMagnetBall,CLASSUID_MagnetBall);

	virtual const char *GetShowName()	{		return "磁力球";	}
	virtual LevelPos GetFramePos()	{		return _core.GetState().pos.getXZ();	}

protected:

	virtual void _OnPostCreate();


	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);
	void _OnWriteSyncH(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	void _OnUpdate();
	virtual BOOL _NeedOps()	{		return TRUE;	}

	void _WriteState(CBitPacket *bp);

	AnimTick _tStart;

	CMagnetBall _core;

	LevelObjID _idTarget;

	BOOL _bExplode;
	BOOL _bDamage;
	AnimTick _tExplode;


};
