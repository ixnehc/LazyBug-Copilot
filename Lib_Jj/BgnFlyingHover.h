#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelGesture.h"

struct CBgp_FlyingHover;
class CLevelGesture_FlyingHover:public CLevelGesture_BuildIn
{
public:
	IMPLEMENT_REFCOUNT_C;
	DEFINE_CLASS(CLevelGesture_FlyingHover);
	CLevelGesture_FlyingHover()
	{
		Zero();
	}

	void Zero()
	{
		_owner=NULL;
		_tNextMove=ANIMTICK_INFINITE;
		_bgp=NULL;
		_bFinished=FALSE;
		_idFacingTarget=LevelObjID_Invalid;
	}

	BOOL IsValid()
	{
		return _owner!=NULL;
	}

	void Init(CLevelObj *lo,CBgp_FlyingHover *bgp,LevelPos &posSpecified,LevelObjID idFacingTarget);

	virtual void Destroy();
	virtual void Update(CUnit3D *unit,float dt);
	virtual void Update(CUnit *unit,float dt)	{	}
	virtual BOOL IsFinished()	{		return _bFinished;	}

	void SetSpecifiedPos(LevelPos &pos)	{		_posSpecified=pos;	}

protected:
	AnimTick _GetT();

	void _UpdateCenterPos(AnimTick t);
	void _GenNewPos(AnimTick t);
	BOOL _RecordMasterPos();
	BOOL _RecordThreatPos();

	CLevelObj *_owner;
	AnimTick _tNextMove;
	LevelPos _posCur;
	float _twist;//偏离航线的角度

	AnimTick _dur;
	AnimTick _tStart;

	BOOL _bFinished;

	LevelPos _posInitial;
	LevelPos _posSpecified;
	LevelObjID _idFacingTarget;

	CBgp_FlyingHover *_bgp;

	friend class CBgn_FlyingHover;
};



struct CBgp_FlyingHover:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_FlyingHover);

	enum HoveringCenterMode
	{
		HoveringCenter_Random=0,
		HoveringCenter_Master,
		HoveringCenter_Threat,
		HoveringCenter_CurPos,
		HoveringCenter_SpecifiedPos,

		HoveringCenter_ForceDword=0xffffffff,
	};

	enum FacingMode
	{
		Facing_Default,
		Facing_Threat,
		Facing_SpecifiedTarget,
	};

	virtual const char *GetTypeName()	{		return "盘旋飞行";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_dur>0)
		{
			FormatString(s,"盘旋飞行,持续%.2f(+/-)%.2f秒",
				ANIMTICK_TO_SECOND(_dur),
				ANIMTICK_TO_SECOND(_durVary));
		}
		else
			s="盘旋飞行,永久";
	}

	BEGIN_GOBJ_PURE_UID(CBgp_FlyingHover,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(HoveringCenterMode,_modeCenter,HoveringCenter_Random); GELEM_UID(12);
			GELEM_EDITVAR("中心点产生方式",GVT_S,GSem(GSem_Interger,"随机:0,主人:1,Threat:2,当前位置:3,指定位置:4"),"如何产生盘旋中心点");
		GELEM_VAR_INIT(FacingMode,_modeFacing,Facing_Default); GELEM_UID(13);
			GELEM_EDITVAR("朝向方式",GVT_S,GSem(GSem_Interger,
				"缺省:0"		"|指定朝向对象变量,"
				"Threat:1"	"|指定朝向对象变量,"
				"指定对象:2" ""
				),"朝向方式");
		GELEM_BEHAVIORMEM_OBJID(_nmFacingTarget,"指定朝向对象变量","从哪个变量中取得朝向的对象"); GELEM_UID(15);
		GELEM_VAR_INIT(AnimTick,_gap,ANIMTICK_FROM_SECOND(10.0f));GELEM_UID(1);
			GELEM_EDITVAR("间隔时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"每次更新中心点的间隔时间");
		GELEM_VAR_INIT(AnimTick,_gapVary,ANIMTICK_FROM_SECOND(2.0f));GELEM_UID(2);
			GELEM_EDITVAR("间隔时间上下浮动",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"间隔时间的浮动值");
		GELEM_VAR_INIT(float,_range,10.0f);GELEM_UID(3);
			GELEM_EDITVAR("移动范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"盘旋的中心点变动的范围");
		GELEM_VAR_INIT(float,_rangeHover,10.0f);GELEM_UID(4);
			GELEM_EDITVAR("盘旋范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"盘旋时的范围");
		GELEM_VAR_INIT(float,_verMin,2.0f);GELEM_UID(5);
			GELEM_EDITVAR("飞行最低高度",GVT_F,GSem(GSem_Float,"0,100,0.1"),"盘旋飞行时的最低高度");
		GELEM_VAR_INIT(float,_verMax,4.0f);GELEM_UID(6);
			GELEM_EDITVAR("飞行最高高度",GVT_F,GSem(GSem_Float,"0,100,0.1"),"盘旋飞行时的最高高度");

		GELEM_VAR_INIT(float,_twist,0.5f);GELEM_UID(7);
			GELEM_EDITVAR("水平扭动值",GVT_F,GSem(GSem_Float,"0,1,0.05"),"这个值越大,飞行时可以以越大的角度变换飞行方向");
		GELEM_VAR_INIT(float,_twistVary,0.2f);GELEM_UID(8);
			GELEM_EDITVAR("水平扭动值浮动",GVT_F,GSem(GSem_Float,"0,1,0.05"),"水平扭动值的变化范围");

		GELEM_VAR_INIT(float,_blend,0.2f);GELEM_UID(9);
			GELEM_EDITVAR("惯性抵抗",GVT_F,GSem(GSem_Float,"0,1,0.01"),"这个值越大,飞行时越不受惯性的影响");

		GELEM_VAR_INIT(BOOL,_bSpeedOverride,FALSE);GELEM_UID(15);
			GELEM_EDITVAR("重载速度",GVT_S,GSem(GSem_Boolean,"盘旋速度"),"是否要重载速度");
		GELEM_VAR_INIT(float,_speed,2.0f);GELEM_UID(16);
			GELEM_EDITVAR("盘旋速度",GVT_F,GSem(GSem_Float,"0,100,0.01"),"盘旋速度");

		GELEM_VAR_INIT(AnimTick,_dur,ANIMTICK_FROM_SECOND(10.0f));GELEM_UID(10);
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续盘旋多久,这个时间到后将停止盘旋,如果为0表示永久盘旋");
		GELEM_VAR_INIT(AnimTick,_durVary,ANIMTICK_FROM_SECOND(5.0f));GELEM_UID(11);
			GELEM_EDITVAR("持续时间浮动",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续时间的浮动值");

		GELEM_BEHAVIORMEM_POS(_nmPos,"指定位置变量","从哪个变量中取得指定位置"); GELEM_UID(14);
		GELEM_VAR_INIT(BOOL,_bTrackPosVar,FALSE);
			GELEM_EDITVAR("跟踪位置变量的值",GVT_S,GSem_Boolean,"盘旋过程中持续用位置变量的值更新盘旋中心点");
	END_GOBJ();

	HoveringCenterMode _modeCenter;
	FacingMode _modeFacing;

	AnimTick _gap;
	AnimTick _gapVary;
	float _range;
	float _rangeHover;
	float _verMin;
	float _verMax;
	float _twist;
	float _twistVary;
	float _blend;
	BOOL _bSpeedOverride;
	float _speed;
	AnimTick _dur;
	AnimTick _durVary;

	StringID _nmPos;
	BOOL _bTrackPosVar;
	StringID _nmFacingTarget;

};



//闲逛
class CBgn_FlyingHover:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_FlyingHover);

	CBgn_FlyingHover()
	{
		_ges=NULL;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

	virtual void Destroy();
	virtual void Break(BGNOutputs &outputs);

protected:
	BOOL _Update();


	CLevelGesture_FlyingHover *_ges;

};


