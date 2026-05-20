#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_Accompany_Obsolete:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Accompany_Obsolete);

	virtual const char *GetTypeName()	{		return "伴随(obsolete)";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (bContinuous)
			FormatString(s,"持续伴随锁定的玩家");
		else
			FormatString(s,"伴随锁定的玩家(尝试一次)");
		if (modeInnerMove==1)
			s+=",靠后";
		if (modeInnerMove==2)
			s+=",靠前";
		if (idSkill!=RecordID_Invalid)
			AppendFmtString(s,"\n固定位置施放技能:%s",assist->GetSkillName(idSkill));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Accompany_Obsolete,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(BOOL,bContinuous,TRUE);
			GELEM_EDITVAR("持续伴随",GVT_S,GSem_Boolean,"是否持续伴随,还是只尝试一次伴随");
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("固定位置施放技能",GVT_U,GSem(GSem_RecordID,"skills"),"伴随过程中停下时释放的技能(固定位置施放)");
		GELEM_VAR_INIT(int,modeInnerMove,2);
			GELEM_EDITVAR("内部移动模式",GVT_S,GSem(GSem_Interger,"不移动,靠后,靠前"),"内部移动模式");
    END_GOBJ();    

public: //当作protected
	RecordID idSkill;

	BOOL bContinuous;
	int modeInnerMove;
};

struct AttrNodeBase;
class CBgn_Accompany_Obsolete:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Accompany_Obsolete);

	CBgn_Accompany_Obsolete()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Destroy();
	virtual void Break(BGNOutputs &outputs);


protected:
};
