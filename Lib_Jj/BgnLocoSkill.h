#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

struct LocoSkillStages
{
    BEGIN_GOBJ_PURE(LocoSkillStages,1);
		GELEM_VAR_INIT(StringID,cycleLStomp,StringID_Invalid);GELEM_UID(1);
			GELEM_EDITVAR("CycleLeftStomp",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");
		GELEM_VAR_INIT(StringID,cycleRStomp,StringID_Invalid);GELEM_UID(2);
			GELEM_EDITVAR("CycleRightStomp",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");
		GELEM_VAR_INIT(StringID,stopLStomp,StringID_Invalid);GELEM_UID(3);
			GELEM_EDITVAR("StopLeftStomp",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");
		GELEM_VAR_INIT(StringID,stopRStomp,StringID_Invalid);GELEM_UID(4);
			GELEM_EDITVAR("StopRightStomp",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");
		GELEM_VAR_INIT(StringID,startFwd,StringID_Invalid);GELEM_UID(5);
			GELEM_EDITVAR("StartForward",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");
		GELEM_VAR_INIT(StringID,startL90,StringID_Invalid);GELEM_UID(6);
			GELEM_EDITVAR("StartLeft90",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");
		GELEM_VAR_INIT(StringID,startR90,StringID_Invalid);GELEM_UID(7);
			GELEM_EDITVAR("StartRight90",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");
		GELEM_VAR_INIT(StringID,startL180,StringID_Invalid);GELEM_UID(8);
			GELEM_EDITVAR("StartLeft180",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");
		GELEM_VAR_INIT(StringID,startR180,StringID_Invalid);GELEM_UID(9);
			GELEM_EDITVAR("StartRight180",GVT_U,GSem(GSem_StringID,"技能阶段"),"阶段名称");
	END_GOBJ();    

	StringID cycleLStomp;//Left Stomp
	StringID cycleRStomp;//Right Stomp
	StringID stopLStomp;
	StringID stopRStomp;
	StringID startFwd;
	StringID startL90;
	StringID startR90;
	StringID startL180;
	StringID startR180;
};

struct LocoSkillSetting
{
	LocoSkillStages stages;

	BEGIN_GOBJ_PURE(LocoSkillSetting,1);

		GELEM_OBJ(LocoSkillStages,stages);
			GELEM_EDITOBJ("Stages","Stages");

	END_GOBJ();    


};

struct BMO_LocoSkillState:public CBehaviorMemObj
{
	DECLARE_CLASS(BMO_LocoSkillState);
	BEGIN_GOBJ_PURE(BMO_LocoSkillState,1);

		GELEM_VAR_INIT(LocoSkillSetting *,setting,NULL);			GELEM_UID(1);
		GELEM_VAR_INIT(StringID ,stageCur,StringID_Invalid);			GELEM_UID(2);
		GELEM_VAR(LevelPos,posTarget);			GELEM_UID(3);
		GELEM_VAR_INIT(BYTE,status,None);			GELEM_UID(4);
		GELEM_VAR_INIT(BOOL,bStartToLeft,FALSE);			GELEM_UID(5);

	END_GOBJ();

	enum Status
	{
		None,
		Start,
		CycleLStomp,
		CycleRStomp,
		Stop,
	};

	void Reset(LocoSkillSetting *setting_)
	{
		setting=setting_;
		status=0;
		stageCur=StringID_Invalid;
		bStartToLeft=NULL;
		posTarget.set(0.0f,0.0f);
		bRequestStop=FALSE;
	}

	LocoSkillSetting *setting;
	BYTE status;
	StringID stageCur;
	BOOL bStartToLeft;
	LevelPos posTarget;
	BOOL bRequestStop;
};


class CBgp_LocoSkillSetup:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_LocoSkillSetup);

	virtual const char *GetTypeName()	{		return "LocoSkill-设置";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (varState!=StringID_Invalid)
			FormatString(s,"在[%s]中初始化LocoSkill",StrLib_GetStr(varState));
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_LocoSkillSetup,487,1);
		GELEM_BGP_BASE();
		GELEM_OBJ(LocoSkillSetting,setting);
			GELEM_EDITOBJ("设置","设置");

		GELEM_VAR_INIT( StringID,varState,StringID_Invalid);
			GELEM_EDITVAR( "LocoSkill状态变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "LocoSkill状态变量");

	END_GOBJ();    

public: //当作protected
	LocoSkillSetting setting;

	StringID varState;
};


class CBgn_LocoSkillSetup:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_LocoSkillSetup);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



class CBgp_LocoSkillSwitchStage:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_LocoSkillSwitchStage);

	virtual const char *GetTypeName()	{		return "LocoSkill-切换Stage";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (varState!=StringID_Invalid)
			FormatString(s,"为[%s]切换LocoSkill的Stage",StrLib_GetStr(varState));
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_LocoSkillSwitchStage,488,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,varState,StringID_Invalid);
			GELEM_EDITVAR( "LocoSkill状态变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "LocoSkill状态变量");
	END_GOBJ();    

public: //当作protected

	StringID varState;
};


class CBgn_LocoSkillSwitchStage:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_LocoSkillSwitchStage);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

class CBgp_LocoSkillStop:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_LocoSkillStop);

	virtual const char *GetTypeName()	{		return "LocoSkill-停止";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (varState!=StringID_Invalid)
			FormatString(s,"对[%s]停止LocoSkill",StrLib_GetStr(varState));
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_LocoSkillStop,489,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,varState,StringID_Invalid);
			GELEM_EDITVAR( "LocoSkill状态变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "LocoSkill状态变量");

	END_GOBJ();    

public: //当作protected

	StringID varState;
};


class CBgn_LocoSkillStop:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_LocoSkillStop);

	void Start(DWORD iStb,BGNOutputs &outputs) override;
	void Update(BGNOutputs &outputs) override;

protected:

};
