#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



struct CBgp_FlyUp:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_FlyUp);

	virtual const char *GetTypeName()	{		return "起飞";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_idSkill!=RecordID_Invalid)
			FormatString(s,"起飞,使用[%s]\n过程持续%.2f秒",assist->GetSkillName(_idSkill),ANIMTICK_TO_SECOND(_dur));
	}


	BEGIN_GOBJ_PURE_UID(CBgp_FlyUp,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);
			GELEM_EDITVAR("起飞使用的技能",GVT_U,GSem(GSem_RecordID,"skills"),"使用的技能");
		GELEM_VAR_INIT(AnimTick,_dur,ANIMTICK_FROM_SECOND(10.0f));
			GELEM_EDITVAR("起飞时间",GVT_U,GSem(GSem_AnimTick,"0.0f,100,0.1"),"起飞过程持续多久");
	END_GOBJ();

	RecordID _idSkill;
	AnimTick _dur;

};

class CBgn_FlyUp:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_FlyUp);

	CBgn_FlyUp()
	{
		_tStart=0;
		_tStartFly=ANIMTICK_INFINITE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);


protected:

	AnimTick _tStart;
	AnimTick _tStartFly;

};

