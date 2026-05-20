#pragma once

#include "EoBulletBase.h"
#include "EoBullet.h"

#include "gds/GObjUID.h"

#include "behaviorgraph/BehaviorParam.h"

#include "spline/CubicSpline.h"

#include "LevelDeal.h"


#define CLASSUID_ChainedHammer 52

#define GRAB_DUR (0.2f)

struct EoParamChainedHammer:public EoParamBullet
{
	DEFINE_EOPARAM_CLASS(EoParamChainedHammer);

	enum Mode
	{
		Mode_None,
		Mode_Throw,
		Mode_Swing,

		Mode_ForceDword=0xffffffff,
	};

	EoParamChainedHammer()
	{
		GConstructor();
		_vsSwingRadius.ResetFloat(0.0f);
		_vsSwingRadius.AddFloat(0.5f,5.0f);
		_vsSwingRadius.AddFloat(1.0f,0.0f);
	}

	~EoParamChainedHammer()
	{
		GDestructor();
	}
	

	BEGIN_GOBJ_UID(EoParamChainedHammer,1);
		DERIVE_GOBJ(EoParamChainedHammer,EoParamBullet);

		GELEM_VAR_INIT(unsigned __int64,_chain,0);
			GELEM_EDITVAR("锁链Proto",GVT_Bx8,GSem_ProtoPath,"锁链的Proto");

		GELEM_VAR_INIT(Mode,_mode,Mode_Throw);
			GELEM_EDITVAR("工作模式",GVT_S,GSem(GSem_Interger,
				"投掷:1"		"|横扫时间&横扫半径,"
				"横扫:2"	"|锤子停止旋转&替身ID&替身BehaviorGraph变量&拔出时间"
				),"奖励类型");

		GELEM_VAR_INIT(float,_angleStart,0.0f);
			GELEM_EDITVAR("锤子初始旋转",GVT_F,GSem(GSem_Float,"0.0,360.0,0.05"),"锤子初始旋转");
		GELEM_VAR_INIT(float,_angleStop,60.0f);
			GELEM_EDITVAR("锤子停止旋转",GVT_F,GSem(GSem_Float,"0.0,360.0,0.05"),"锤子停止旋转");
		GELEM_VAR_INIT(float,_spdRot,360.0f);
			GELEM_EDITVAR("锤子转速",GVT_F,GSem(GSem_Float,"0.0,7200.0,0.05"),"锤子转速");
		GELEM_BEHAVIORMEM_OBJID(_nmBhvVar,"BehaviorGraph变量","保存在哪个BehaviorGraph变量中");
		GELEM_VAR_INIT(RecordID,_idImposter,RecordID_Invalid);
			GELEM_EDITVAR("替身ID",GVT_U,GSem(GSem_RecordID,"units"),"替身");
		GELEM_BEHAVIORMEM_OBJID(_nmBhvVarImposter,"替身BehaviorGraph变量","替身保存在哪个BehaviorGraph变量中");

		GELEM_VAR_INIT(float,_spdWithdraw,6.0f);
			GELEM_EDITVAR("收回速度",GVT_F,GSem(GSem_Float,"0.1,100.0,0.05"),"收回速度");
		GELEM_VAR_INIT(AnimTick,_durPullOut,ANIMTICK_FROM_SECOND(0.6f));
			GELEM_EDITVAR("拔出时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"拔出时间时间");
		GELEM_OBJVECTOR(DealEntry,_dealsBroken);
			GELEM_EDITOBJ("打断结算列表","打断结算列表");

		GELEM_VAR_INIT( StringID,_targetWithdraw,StringID_Invalid);
			GELEM_EDITVAR( "收回位点", GVT_U, GSem(GSem_StringID,"Dummy名称"), "挂接的Dummy名称" );
		GELEM_VAR_INIT( StringID,_eventHideWpn,StringID_Invalid);	
			GELEM_EDITVAR( "隐藏武器骨架事件", GVT_U, GSem(GSem_StringID,"骨架事件"), "隐藏武器骨架事件" );
		GELEM_VAR_INIT( StringID,_eventShowWpn,StringID_Invalid);	
			GELEM_EDITVAR( "显示武器骨架事件", GVT_U, GSem(GSem_StringID,"骨架事件"), "显示武器骨架事件" );

		GELEM_VAR_INIT(AnimTick,_durSwing,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("横扫时间",GVT_U,GSem(GSem_AnimTick,"0.1,100,0.1"),"横扫时间");
		GELEM_OBJVAR( ValueSet, _vsSwingRadius);
			GELEM_EDITOBJ_EX("横扫半径","横扫半径曲线",GSem( GSem_Unknown, "0,0,1,20" ));

	END_GOBJ();

	Mode _mode;

	unsigned __int64 _chain;

	float _angleStart;
	float _spdRot;

	StringID _targetWithdraw;

	StringID _eventHideWpn;
	StringID _eventShowWpn;

	float _spdWithdraw;

	//投掷模式
	float _angleStop;
	RecordID _idImposter;
	StringID _nmBhvVar;
	StringID _nmBhvVarImposter;
	AnimTick _durPullOut;
	std::vector<DealEntry> _dealsBroken;//打断时对owner的deal

	//横扫模式
	AnimTick _durSwing;
	ValueSet _vsSwingRadius;
};

class EoChainedHammer;
class CChainedHammer:public CBulletBase
{
public:
	DEFINE_CLASS(CChainedHammer);

	CChainedHammer()
	{
		_param=NULL;
		_t=0.0f;
		_speed=0.0f;
	}

	~CChainedHammer()
	{
	}

	void Init(EoChainedHammer *owner,LevelPos3D &src,i_math::vector3df&dir,EoParamChainedHammer *param);

	float GetT()	{		return _t;	}

	virtual i_math::vector3df&GetDir()	override{		return _dir;	}
	float GetSpeed()	{		return _speed;	}
	void GetSwingFace(LevelFace &faceInitial,LevelFace &face)
	{
		faceInitial=_faceSwingInitial;
		face=_faceSwing;
	}

	LevelObjID _DetectHit_ShieldAmulet(i_math::line3df &line) override;

	LevelObjID _DetectHit(i_math::line3df &line) override;
	void _DetectHits(i_math::line3df &line,LevelObjHits &hits,CLevelObjHistory &history) override;

	static void BuildSwingSpline(LevelPos3D &pos,LevelFace faceInitial,LevelFace faceOwner,ValueSet &vsRadius,CCubicSpline &spline);


protected:

	virtual BOOL _NeedStop()
	{
		return FALSE;
	}
	virtual void _UpdateStep(AnimSecond dt,LevelPos3D &dir,float &dDist);
	virtual BOOL _CanHit(CLevelObj *lo)
	{
		return TRUE;
	}

	EoParamChainedHammer *_param;
	float _t;
	LevelPos3D _posLocal;

	//投掷
	i_math::vector3df _dir;
	float _speed;

	//横扫
	LevelFace _faceSwingInitial;
	LevelFace _faceSwing;
	CCubicSpline _splineSwing;
};

class EoChainedHammer:public EoBulletBase
{
public:
	EoChainedHammer()
	{
		_stage=Stage_None;
		_tStageStart=0;
		_idThreat=LevelObjID_Invalid;
		_idOwner=LevelObjID_Invalid;
		_durWithdraw=0;
	}
	DEFINE_LEVELOBJ_CLASS(EoChainedHammer,CLASSUID_ChainedHammer);

	enum Stage
	{
		Stage_None,
		Stage_Throwing,
		Stage_Stuck,
		Stage_Hit,
		Stage_Withdrawing,
		Stage_Withdrawn,
		Stage_Pulling,
		Stage_Detached,
		Stage_Grabbing,
	};

	virtual const char *GetShowName()	{		return "锁链锤";	}


	BOOL Withdraw();
	BOOL Grab();
	BOOL Pull();
	BOOL Break();
	BOOL IsWithdrawn()	{		return Stage_Withdrawn==_stage;	}
	BOOL IsStuck()	{		return Stage_Stuck==_stage;	}
	BOOL IsDetached()	{		return Stage_Detached==_stage;	}

	Stage GetStage()	{		return _stage;	}
	LevelPos3D GetTargetPos()	{		return _posTarget;	}
	LevelPos3D GetSrcPos()	{		return _posSrc;	}
	LevelObjID GetThreatID()	{		return _idThreat;	}

	virtual LevelPos GetFramePos() override;
	virtual LevelPos3D GetFramePos3D() override;


protected:

	virtual void _OnPostCreate() override;
	virtual void _OnDetroy() override;
	virtual void _OnUpdate() override;

	void _OnWriteFirstSync(CBitPacket *bp,BOOL &bContent,LevelPlayerID idPlayer);

	virtual CBulletBase *_CreateBullet();
	virtual void _DestroyBullet(CBulletBase *bullet);

	void _AddOp(LevelOp_ChainedHammer::Op op,AnimTick dur,LevelOpLink *link=NULL);

	void _SetStage(Stage stage);

	LevelObjID _idThreat;
	LevelObjID _idOwner;
	LevelObjID _idImposter;
	LevelPos3D _posSrc;
	LevelPos3D _posTarget;
	Stage _stage;
	AnimTick _tStageStart;
	AnimTick _durWithdraw;

	CCubicSpline _splineSwing;

};
