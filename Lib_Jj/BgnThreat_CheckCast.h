#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpThreat_CheckCast:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgpThreat_CheckCast);

	enum Mode
	{
		Mode_Check,
		Mode_UntilSuccess,
		ForceDword,
	};

	virtual const char *GetTypeName()	{		return "检测向Threat施放技能";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Threat;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (idSkill==RecordID_Invalid)
			s="n/a";
		else
		{
			if (mode==Mode_Check)
				FormatString(s,"检测技能[%s]能否在此时此处对Threat施放",assist->GetSkillName(idSkill));
			if (mode==Mode_UntilSuccess)
			{
				if (dur==0)
					FormatString(s,"持续检测技能[%s]能否在此时此处对Threat施放,直到成功",assist->GetSkillName(idSkill));
				else
					FormatString(s,"持续检测技能[%s]能否在此时此处对Threat施放,直到成功,持续%.2f秒",assist->GetSkillName(idSkill),ANIMTICK_TO_SECOND(dur));
			}
			if (bIgnoreFaceCheck)
				s+=",忽略朝向检测";
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_CheckCast,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(DWORD,mode,Mode_Check);
			GELEM_EDITVAR("工作模式",GVT_U,GSem(GSem_Interger,
				"检测一次:0"		"|持续时间,"
				"持续检测直到成功:1"	"|"
				),"工作模式");

		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"持续时间,0代表永远");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("技能",GVT_U,GSem(GSem_RecordID,"skills"),"包含路径的资源");
		GELEM_VAR_INIT(BOOL,bIgnoreFaceCheck,FALSE);
			GELEM_EDITVAR("忽略朝向检测",GVT_S,GSem_Boolean,"忽略朝向检测");
	END_GOBJ();    

public: //当作protected

	Mode mode;
	RecordID idSkill;
	AnimTick dur;
	BOOL bIgnoreFaceCheck;
};

class CBgnThreat_CheckCast:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_CheckCast);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

	BOOL _Check(CBgpThreat_CheckCast*pad);

	AnimTick _tStart;

};
