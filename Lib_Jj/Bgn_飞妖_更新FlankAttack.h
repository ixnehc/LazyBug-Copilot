#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_飞妖_更新FlankAttack:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_飞妖_更新FlankAttack);

	virtual const char *GetTypeName()	{		return "飞妖_更新FlankAttack";	}
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
	}

    BEGIN_GOBJ_PURE_UID(CBgp_飞妖_更新FlankAttack,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续时间");

		GELEM_VAR_INIT(BOOL ,bCW,FALSE);
			GELEM_EDITVAR("环绕方向",GVT_U,GSem(GSem_Interger,"逆时针,顺时针"),"环绕方向");

		GELEM_BEHAVIORMEM_OBJID(varTooth,"飞妖Tooth","飞妖Tooth");
		GELEM_VAR_INIT(float,distMaxFromTooth,12.0f);
			GELEM_EDITVAR("离Tooth最远距离",GVT_F,GSem(GSem_Float,"0.1,100.0,0.05"),"离Tooth最远距离");

		GELEM_BEHAVIORMEM_INTERGER(varNeedEnd,"终止标志变量","终止标志变量");

		GELEM_VAR_INIT(float,radiusMin,3.0f);
			GELEM_EDITVAR("最小环绕半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"最小环绕半径");
		GELEM_VAR_INIT(float,radiusMax,5.0f);
			GELEM_EDITVAR("最大环绕半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"最小环绕半径");
		GELEM_VAR_INIT(float,spdCenterFollow,3.0f);
			GELEM_EDITVAR("中心点移动速度",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"中心点移动速度");

    END_GOBJ();    

public: //当作protected
	BOOL bCW;

	StringID varTooth;
	float distMaxFromTooth;

	StringID varNeedEnd;
	float radiusMin;
	float radiusMax;

	float spdCenterFollow;

	AnimTick dur;

};


class CBgn_飞妖_更新FlankAttack:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_飞妖_更新FlankAttack);

	CBgn_飞妖_更新FlankAttack()
	{
		_dirLast=-10000.0f;
		_swept=0.0f;

		_bTargetPos=FALSE;
		_bPathFace=FALSE;
		_bCenterPos=FALSE;

		_tStart=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;
	virtual void Update(BGNOutputs &outputs) override;

protected:
	LevelFace _swept;
	LevelFace _dirLast;

	BOOL _bPathFace;
	LevelFace _facePath;

	BOOL _bTargetPos;
	LevelPos _posTarget;
	LevelPos _posTargetLast;

	BOOL _bCenterPos;
	LevelPos _posCenter;

	LevelSkillTarget _targetOrg;
	AnimTick _t;
	AnimTick _tStart;


};

