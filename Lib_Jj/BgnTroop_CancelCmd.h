#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "LevelAIContext.h"

struct AttrNodeFloat;
class CBgpTroop_CancelCmd:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpTroop_CancelCmd);

	virtual const char *GetTypeName()	{		return "取消Troop命令";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_IN(1,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"取消Troop %s内的%s的命令",
			GetBVRDesc_StringID(BVR_ARG(_troop),assist),
			LevelTroopRankFlag_GetDesc(BVR_ARG(_flagsRank)));

	}

    BEGIN_GOBJ_PURE_UID(CBgpTroop_CancelCmd,1);
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","组建哪个Troop");
			GELEM_BVR();

		GELEM_VAR_INIT(LevelTroopRankFlags,_flagsRank,LevelTroopRankFlag_All);
			GELEM_EDITVAR("职级",GVT_U,GSem(GSem_Flags,LevelTroopRankFlags_GSemContrains),"对哪些职级发命令");
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(StringID,_troop);
	DEFINE_BVR(LevelTroopRankFlags,_flagsRank);
};


class CBgnTroop_CancelCmd:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_CancelCmd);

	CBgnTroop_CancelCmd()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Destroy();

protected:
	void _UpdateTcc(CLevelTroop *troop);

	void _OccupyTroopControl();
	void _DiscardTroopControl();


};

