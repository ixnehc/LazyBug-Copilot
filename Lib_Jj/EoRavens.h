#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "spline/CubicSpline.h"

#include "EoUtumAttack.h"


#define CLASSUID_Ravens 70


struct EoParamRavens:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamRavens);

	EoParamRavens()
	{
		GConstructor();
		vsVer.ResetFloat(0.0f);
		vsVer.AddFloat(0.33f,0.3f);
		vsVer.AddFloat(0.66f,0.3f);
		vsVer.AddFloat(1.0f,0.0f);
	}
	~EoParamRavens()
	{
		GDestructor();
	}

	BEGIN_GOBJ(EoParamRavens,1);

		GELEM_VAR_INIT(int,countMin,2);
			GELEM_EDITVAR("最小个数",GVT_S,GSem_Interger,"最小个数");
		GELEM_VAR_INIT(int,countMax,3);
			GELEM_EDITVAR("最大个数",GVT_S,GSem_Interger,"最大个数");
		GELEM_VAR_INIT(float,radiusMin,15.0f);
			GELEM_EDITVAR("落点最小范围",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"落点最小范围");
		GELEM_VAR_INIT(float,radiusMax,25.0f);
			GELEM_EDITVAR("落点最大范围",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"落点最大范围");
		GELEM_VAR_INIT(float,speed,5.0f);
			GELEM_EDITVAR("飞行速度",GVT_F,GSem(GSem_Float,"0.01,100,0.1"),"飞行速度");
		GELEM_VAR_INIT(float,distGuide,0.0f);
			GELEM_EDITVAR("导引距离",GVT_F,GSem(GSem_Float,"0.00,100,0.1"),"导引距离");

		GELEM_OBJVAR( ValueSet, vsVer);
			GELEM_EDITOBJ_EX( "高度变化", "随水平距离变化的高度变化", GSem( GSem_Unknown, "0,0,1,2") );

		GELEM_VAR_INIT(AnimTick,durTakeOff,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("起飞动画时长",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"起飞动画时长");
		GELEM_VAR_INIT(AnimTick,durLand,ANIMTICK_FROM_SECOND(1.0f));
			GELEM_EDITVAR("着陆动画时长",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"着陆动画时长");
		GELEM_VAR_INIT(AnimTick,durFlyingCycle,ANIMTICK_FROM_SECOND(0.7f));
			GELEM_EDITVAR("飞行动画周期时长",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"飞行动画周期时长");
		GELEM_VAR_INIT( StringID,nmTuner,StringID_Invalid);	
			GELEM_EDITVAR( "动画控制Tuner名称", GVT_U, GSem(GSem_StringID,"动画树Tuner名称"), "动画控制Tuner名称" );

		GELEM_OBJVECTOR(DealEntry,summon); 
			GELEM_EDITOBJ("真身召唤","结算");
		GELEM_OBJVECTOR(DealEntry,summonImposter); 
			GELEM_EDITOBJ("假身召唤","结算");

	END_GOBJ();

	int countMin;
	int countMax;
	float radiusMin;
	float radiusMax;

	float distGuide;

	float speed;

	ValueSet vsVer;

	AnimTick durTakeOff;
	AnimTick durLand;
	AnimTick durFlyingCycle;
	StringID nmTuner;

	std::vector<DealEntry> summon;
	std::vector<DealEntry> summonImposter;

};

class EoRavens:public CLoEffectObj
{
public:
	EoRavens()
	{
		_idxGenuine=-1;
		_hpOwner=-1;
		_troop=NULL;
	}
	DEFINE_LEVELOBJ_CLASS(EoRavens,CLASSUID_Ravens);


	virtual const char *GetShowName()	{		return "乌鸦飞行";	}
	virtual LevelPos GetFramePos() override;

	void SetTroop(CLevelTroop *troop)override	{		_troop=troop;	}
	CLevelTroop *GetTroop()override	{		return _troop;	}

protected:

	struct RavenPath
	{
		LevelPos posTarget;
		CCubicSpline spline;
		BOOL bDealt;
	};


	void _OnPostCreate()override;
	void OnDestroy()override;


	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer)override;

	void _OnUpdate()override;
	virtual BOOL _NeedOps()	{		return FALSE;	}

	void _Build();
	void _Build(RavenPath &path);

	CLevelTroop *_troop;

	std::vector<RavenPath> _pathes;
	int _idxGenuine;
	int _hpOwner;

};
