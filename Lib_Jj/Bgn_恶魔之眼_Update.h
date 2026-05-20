#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"

struct Param_DevilEye
{
	float radiusSense;
	float radiusSight;
	float fov;
	float radiusOutlook;
	AnimTick durOutlook;
	AnimTick durMinSwitchFaceCD;
	AnimTick durMaxSwitchFaceCD;
	float angleMinSwitchFace;
	float angleMaxSwitchFace;
	float spdIncAlert;
	float spdDecAlert;

	BEGIN_GOBJ_PURE(Param_DevilEye,1);

		GELEM_VAR_INIT(float,radiusSense,5.0f);
			GELEM_EDITVAR("感知范围",GVT_F,GSem(GSem_Float,"0.1,30.0,0.05"),"感知范围");

		GELEM_VAR_INIT(float,radiusSight,10.0f);
			GELEM_EDITVAR("视野范围",GVT_F,GSem(GSem_Float,"0.1,30.0,0.05"),"视野范围");

		GELEM_VAR_INIT(float,fov,90.0f);
			GELEM_EDITVAR("视野角度",GVT_F,GSem(GSem_Float,"0.1,180.0,0.05"),"视野角度");

		GELEM_VAR_INIT(float,radiusOutlook,15.0f);
			GELEM_EDITVAR("了望范围",GVT_F,GSem(GSem_Float,"0.1,30.0,0.05"),"了望范围");

		GELEM_VAR_INIT(AnimTick,durOutlook,ANIMTICK_FROM_SECOND(5.0f));
			GELEM_EDITVAR("了望时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"了望时间");

		GELEM_VAR_INIT(AnimTick,durMinSwitchFaceCD,ANIMTICK_FROM_SECOND(3.0f));
			GELEM_EDITVAR("切换朝向最小CD",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"切换朝向最小CD");
		GELEM_VAR_INIT(AnimTick,durMaxSwitchFaceCD,ANIMTICK_FROM_SECOND(6.0f));
			GELEM_EDITVAR("切换朝向最大CD",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"切换朝向最大CD");
		GELEM_VAR_INIT(float,angleMinSwitchFace,30.0f);
			GELEM_EDITVAR("切换朝向最小角度",GVT_F,GSem(GSem_Float,"0.1,180.0,0.05"),"切换朝向最小角度");
		GELEM_VAR_INIT(float,angleMaxSwitchFace,75.0f);
			GELEM_EDITVAR("切换朝向最大角度",GVT_F,GSem(GSem_Float,"0.1,180.0,0.05"),"切换朝向最大角度");

		GELEM_VAR_INIT(float,spdIncAlert,0.2f);
			GELEM_EDITVAR("警戒值升高速度",GVT_F,GSem(GSem_Float,"0.01,10.0f,0.05"),"警戒值累加速度");
		GELEM_VAR_INIT(float,spdDecAlert,0.1f);
			GELEM_EDITVAR("警戒值降低速度",GVT_F,GSem(GSem_Float,"0.01,10.0f,0.05"),"警戒值降低速度");

	END_GOBJ();

};

//FL代表FaceLimit
struct Param_DevilEye_FL
{
	BEGIN_GOBJ_PURE(Param_DevilEye_FL,1);

		GELEM_VAR_INIT(BOOL,bEnable,FALSE);
			GELEM_EDITVAR("Enable",GVT_S,GSem(GSem_Interger,
				"TRUE:1"		","
				"FALSE:0"	"|范围&目标点"
				),"是否有效");
		GELEM_VAR_INIT(float,range,180.0f);
			GELEM_EDITVAR("范围",GVT_F,GSem(GSem_Float,"0.1,180.0,0.05"),"范围");
		GELEM_VARVECTOR(i_math::vector3df,posTarget); 
			GELEM_EDITVAR("目标点",GVT_Fx3,GSem(GSem_Unknown,"MatSet"),"目标点");

	END_GOBJ();

	BOOL bEnable;
	float range;
	std::vector<i_math::vector3df> posTarget;

};

struct BMO_DevilEyeStatus:public CBehaviorMemObj
{
	DECLARE_CLASS(BMO_DevilEyeStatus);
	BEGIN_GOBJ_PURE(BMO_DevilEyeStatus,1);

		GELEM_VAR_INIT(AnimTick,tTarget,0);
			GELEM_UID(1);
		GELEM_VAR_INIT(LevelFace,faceTarget,0.0f);
			GELEM_UID(2);
		GELEM_VAR_INIT(LevelObjID,idTarget,LevelObjID_Invalid);
			GELEM_UID(3);
		GELEM_VAR_INIT(DWORD,ver,0);
			GELEM_UID(4);

	END_GOBJ();

	AnimTick tTarget;
	LevelFace faceTarget;
	LevelObjID idTarget;
	DWORD ver;
};




class CBgp_DevilEye_Update:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_DevilEye_Update);

	virtual const char *GetTypeName()	{		return "恶魔之眼";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (varStatus!=StringID_Invalid)
			FormatString(s,"更新恶魔之眼的状态保存在[%s]中",StrLib_GetStr(varStatus));

	}

    BEGIN_GOBJ_PURE_UID2(CBgp_DevilEye_Update,425,1);
		GELEM_BGP_BASE();

		GELEM_OBJ(Param_DevilEye,param);
			GELEM_EDITOBJ("恶魔之眼参数","恶魔之眼参数");
			GELEM_BVR();

		GELEM_OBJ(Param_DevilEye_FL,paramFL);
			GELEM_EDITOBJ("朝向限制参数","朝向限制参数");
			GELEM_BVR();

		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位");
			GELEM_BVR();
		GELEM_VARVECTOR_INIT(LevelObjRequire,requires,LevelObjRequire_Attackable);
			GELEM_EDITVAR("特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"有哪些特定的需求");
		GELEM_OBJ(LevelDetectWeights,weights);
			GELEM_EDITOBJ("侦测权重","侦测权重");
			GELEM_BVR();


		GELEM_VAR_INIT( StringID,varStatus,StringID_Invalid);
			GELEM_EDITVAR( "恶魔之眼状态保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "恶魔之眼状态保存在哪个变量里");

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(Param_DevilEye,param);
	DEFINE_BVR(Param_DevilEye_FL,paramFL);
	DEFINE_BVR(std::vector<LevelDetectTargetFlag>,flagsDetect);
	std::vector<LevelObjRequire> requires;
	DEFINE_BVR(LevelDetectWeights,weights);

	StringID varStatus;

};

class RollAwardsResult;
class CBgn_DevilEye_Update:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_DevilEye_Update);

	CBgn_DevilEye_Update()
	{
		_state=State_None;
		_tStateStart=0;
		_tLast=ANIMTICK_INFINITE;
		_tNextSwitchFacing=ANIMTICK_INFINITE;
		_vAlert=0.0f;

		_bFaceLimit=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;
	virtual void Update(BGNOutputs &outputs) override;
	virtual void Break(BGNOutputs &outputs) override;
	virtual void Destroy()override;

public:
	enum State
	{
		State_None,
		State_LookAround,
		State_Focusing,
		State_Focused,
		State_OutlookFocusing,
		State_OutlookFocused,
	};

	class CFacing
	{
	public:
		CFacing()
		{
			Reset(0.0f,0,0);
			_ver=0;
		}
		void Reset(LevelFace face,AnimTick t,DWORD ver)
		{
			_faceTarget=_faceSrc=face;
			_tSrc=t;
			_durRotate=0;
			_idTarget=LevelObjID_Invalid;
			_ver=ver;
		}
		BOOL IsReached(AnimTick t)
		{
			return t>=_tSrc+_durRotate;
		}
		LevelFace GetCur(CLevelObj *loOwner)
		{
			AnimTick t=loOwner->GetT();

			LevelFace faceTarget=_faceTarget;

			if (_idTarget!=LevelObjID_Invalid)
			{
				extern LevelFace LevelUtil_CalcTargetFacing(CLevelObj *lo,LevelObjID idTarget);
				faceTarget=LevelUtil_CalcTargetFacing(loOwner,_idTarget);
			}

			if (_durRotate<=0)
				return faceTarget;
			AnimTick dt=ANIMTICK_SAFE_MINUS(t,_tSrc);
			if (dt>_durRotate)
				dt=_durRotate;
			return LevelFaceLerp(_faceSrc,faceTarget,((float)dt)/(float)_durRotate);
		}
		void SetTarget(CLevelObj *loOwner,LevelFace faceTarget,float spdRot)
		{
			AnimTick tCur=loOwner->GetT();
			LevelFace faceCur=GetCur(loOwner);
			_tSrc=tCur;
			_faceSrc=faceCur;

			float gap=i_math::get_radian_dist(_faceSrc,faceTarget);
			_durRotate=ANIMTICK_FROM_SECOND(gap/spdRot);
			_faceTarget=faceTarget;
			_idTarget=LevelObjID_Invalid;

			_ver++;
		}

		void SetTarget(CLevelObj *loOwner,LevelObjID idTarget,float spdRot)
		{
			extern LevelFace LevelUtil_CalcTargetFacing(CLevelObj *lo,LevelObjID idTarget);
			LevelFace faceTarget=LevelUtil_CalcTargetFacing(loOwner,idTarget);

			SetTarget(loOwner,faceTarget,spdRot);
			_idTarget=idTarget;
		}


	public:
		LevelObjID _idTarget;
		LevelFace _faceTarget;
		AnimTick _durRotate;
		LevelFace _faceSrc;
		AnimTick _tSrc;
		DWORD _ver;
	};

	BOOL _CheckDetected(CLevelObj *lo,float dist2);
	LevelFace _faceCur;//仅在_CheckDetected中使用

	LevelObjID _DetectClosest(BOOL &bOutlook);

	BOOL _Update_LookAround(AnimTick tCur,AnimTick dt);
	BOOL _Update_Focusing(AnimTick tCur,AnimTick dt);
	BOOL _Update_OutlookFocusing(AnimTick tCur,AnimTick dt);
	BOOL _Update_OutlookFocused(AnimTick tCur,AnimTick dt);

	void _SetState(State state);
	State _state;
	AnimTick _tStateStart;

	void _IncAlert(AnimTick dt);
	void _DecAlert(AnimTick dt);
	float _vAlert;//取值范围0..1,只在State_LookAround和State_OutlookFocused状态下会增加或减少

	BOOL _CheckValidFocusingTarget(LevelObjID idTarget);

	BOOL _CheckFaceLimit(LevelFace face);

	void _GenSwitchFacingTime();
	AnimTick _tNextSwitchFacing;//只在State_LookAround时有效

	AnimTick _tLast;

	CFacing _facing;

	//For face limit
	BOOL _bFaceLimit;
	LevelFace _faceLimitBase;
	LevelFaceYaw _yawLimit;

};

