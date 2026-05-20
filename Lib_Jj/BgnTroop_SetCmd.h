#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "LevelAIContext.h"

struct AttrNodeFloat;
class CBgpTroop_SetCmd:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpTroop_SetCmd);

	virtual const char *GetTypeName()	{		return "设置Troop命令";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_idCmd==StringID_Invalid)
			FormatString(s,"取消Troop %s内的%s的命令",GetBVRDesc_StringID(BVR_ARG(_troop),assist),LevelTroopRankFlag_GetDesc(BVR_ARG(_flagsRank)));
		else
			FormatString(s,"向Troop %s内的%s设置命令:%s",
				GetBVRDesc_StringID(BVR_ARG(_troop),assist),
				LevelTroopRankFlag_GetDesc(BVR_ARG(_flagsRank)),
				StrLib_GetStr(_idCmd));

	}

    BEGIN_GOBJ_PURE_UID2(CBgpTroop_SetCmd,494,1);
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","组建哪个Troop");
			GELEM_BVR();

		GELEM_VAR_INIT(LevelTroopRankFlags,_flagsRank,LevelTroopRankFlag_All);
			GELEM_EDITVAR("职级",GVT_U,GSem(GSem_Flags,LevelTroopRankFlags_GSemContrains),"对哪些职级发命令");
			GELEM_BVR();

		GELEM_VAR_INIT(StringID,_idCmd,StringID_Invalid);
			GELEM_EDITVAR("命令",GVT_U,GSem(GSem_StringID,"AI命令"),"命令");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(StringID,_troop);
	DEFINE_BVR(LevelTroopRankFlags,_flagsRank);

	StringID _idCmd;

};


class CBgnTroop_SetCmd:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_SetCmd);

	CBgnTroop_SetCmd()
	{
	}

	void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:

};

