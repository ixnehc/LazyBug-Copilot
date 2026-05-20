#pragma once

#include "class/class.h"
#include "gds/GObjEx.h"

#include "records/records.h"


#include "LevelDefines.h"


#include "LoEffectObj.h"

#include "valueset/valueset.h"

#include "math/circle.h"


class CTimedCircles
{
public:
	CTimedCircles()
	{
		Zero();
	}
	~CTimedCircles()
	{
		Clear();
	}

	struct TimedCircle
	{
		AnimTick tStart;
		AnimTick tCache;//circle里的radius计算的时间
		i_math::circlef circle;
	};
	
	void Zero()
	{
		_vsRadiusScale=NULL;
		_radius=0.0f;
		_radiusMax=0.0f;
		_tStart=0;
		_gapMax=0.0f;
	}
	void Init(float radius,ValueSet *vsRadius,float gapMax)
	{
		_radius=radius;
		_vsRadiusScale=vsRadius;
		_radiusMax=_vsRadiusScale->GetMaxFloat();
		_gapMax=gapMax;
	}
	void Clear()
	{
		Reset(0);
		Zero();
	}
	void Reset(AnimTick tStart)
	{
		_circles.clear();
		_tStart=tStart;
	}

	BOOL IsEmpty()	{		return _circles.size()<=0;	}

	void Discard(AnimTick t);

	BOOL Add(i_math::vector2df &pos,AnimTick t);//注意,t必须大于等于最后加入的circle的时间

	BOOL IsIn(i_math::vector2df &pos,AnimTick t);

	i_math::rectf GetBoundRect(AnimTick t);

protected:
	void _Add(i_math::vector2df &pos,AnimTick tLocal)
	{
		TimedCircle tc;
		tc.tStart=tLocal;
		tc.tCache=ANIMTICK_INFINITE;
		tc.circle.center=pos;
		_circles.push_back(tc);
	}


	AnimTick _tStart;
	std::deque<TimedCircle> _circles;
	float _radius;
	ValueSet *_vsRadiusScale;
	float _radiusMax;
	float _gapMax;//两个circle之间的最大距离


};


#define CLASSUID_Stripe 28

struct EoParamStripe:public LevelEoParam
{
	DEFINE_EOPARAM_CLASS(EoParamStripe);
	EoParamStripe()
	{
		GConstructor();
		scaleRadius.ResetFloat(1.0f);
	}
	~EoParamStripe()
	{
		GDestructor();
	}

	BEGIN_GOBJ(EoParamStripe,1);

		GELEM_VAR_INIT(float,radius,1.0f);GELEM_VERSION(2);
			GELEM_EDITVAR("影响范围",GVT_F,GSem(GSem_Float,"0.1,100.0,0.05"),"影响范围");

		GELEM_OBJVAR( ValueSet,scaleRadius);
			GELEM_EDITOBJ_EX( "影响范围缩放曲线", "影响范围随时间变化",GSem( GSem_Unknown, "0,0,-1,1") );

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"持续时间,如果为0,则只在一开始产生伤害");
		GELEM_VAR_INIT(float,speed,1.0f);
			GELEM_EDITVAR("前进速度",GVT_F,GSem(GSem_Float,"0.1,100.0,0.05"),"前进速度");
		GELEM_VAR_INIT(AnimTick,cycle,ANIMTICK_FROM_SECOND(0.2f));
			GELEM_EDITVAR("作用周期",GVT_U,GSem(GSem_AnimTick,"0,2000,0.1"),"作用周期,隔多长时间作用一次");

		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位");

		GELEM_VARVECTOR_INIT(LevelObjRequire,requires,LevelObjRequire_Attackable);
			GELEM_EDITVAR("作用对象的特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"作用对象的特定需求");

	END_GOBJ();

	float radius;
	ValueSet scaleRadius;
	float speed;
	AnimTick dur;
	AnimTick cycle;

	std::vector<LevelDetectTargetFlag> flagsDetect;
	std::vector<LevelObjRequire> requires;
};



class EoStripe:public CLoEffectObj
{
public:
	EoStripe()
	{
		_bEnd=FALSE;
		_tLast=0;
		_remain=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoStripe,CLASSUID_Stripe);

	virtual const char *GetShowName()	{		return "条带影响";	}

	virtual LevelPos GetFramePos()	{		return _pos;	}

protected:
	virtual void _OnPostCreate() override;
	virtual void _OnUpdate() override;
	virtual void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	AnimTick _tLast;

	KeySet _ks;

	LevelPos _pos;

	CTimedCircles _circles;
	BOOL _bEnd;

	LevelPos _dir;

	AnimTick _remain;

};
