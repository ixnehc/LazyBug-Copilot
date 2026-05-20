#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "behaviorgraph/BehaviorCustomConst.h"
#include "behaviorgraph/BehaviorParam.h"

#include "LevelTroops.h"


class CBgpTroop_Join:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpTroop_Join);

	virtual const char *GetTypeName()	{		return "加入Troop";	}
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
		if (_idUnit==StringID_Invalid)
			s="自己";
		else
			FormatString(s,"将[%s]",StrLib_GetStr(_idUnit));
		if (_troop!=StringID_Invalid)
		{
			AppendFmtString(s,"以[%s]为职级加入到[%s]的名为[%s]的Troop",
				LevelTroopRank_GetDesc(_rank),
				StrLib_GetStr(_idTroopOwner),
				GetBVRDesc_StringID(BVR_ARG(_troop),assist));
		}
		else
		{
			AppendFmtString(s,"以[%s]为职级加入到[%s]的Troop",
				LevelTroopRank_GetDesc(_rank),
				StrLib_GetStr(_idTroopOwner));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpTroop_Join,1);

		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","为哪个Troop里的单位添加Buff"); GELEM_VERSION(1);
			GELEM_BVR();
		GELEM_VAR_INIT(LevelTroopRank,_rank,LevelTroopRank_Minion);GELEM_VERSION(2);
			GELEM_EDITVAR("职级",GVT_S,GSem(GSem_Interger,"Leader:1,Minion:2"),"单位的职级");
		GELEM_BEHAVIORMEM_OBJID(_idTroopOwner,"Troop所有者","Troop所有者");GELEM_VERSION(3);
		GELEM_BEHAVIORMEM_OBJID(_idUnit,"单位ID","单位ID");GELEM_VERSION(4);

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(StringID,_troop);
	LevelTroopRank _rank;

	StringID _idTroopOwner;
	StringID _idUnit;


};


class CBgnTroop_Join:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_Join);

	CBgnTroop_Join()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:


};


