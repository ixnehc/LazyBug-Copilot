#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelDeal.h"

#include "records/recordsdefine.h"

class CBgp_Jink:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Jink);

	virtual const char *GetTypeName()	{		return "快闪";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Buff;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if ((_idBuff!=RecordID_Invalid)&&(_varPos!=StringID_Invalid))
			FormatString(s,"快闪(%s),至目的地[%s]",assist->GetBuffName(_idBuff),assist->GetStr(_varPos));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Jink,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_POS(_varPos,"位置变量","从哪个变量里取得移动目的")
		GELEM_VAR_INIT(RecordID,_idBuff,RecordID_Invalid);
			GELEM_EDITVAR("快闪Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"快闪Buff");
		GELEM_OBJVECTOR(DealEntry,_dealsOnPos);
			GELEM_EDITOBJ("原地结算列表","原地结算列表");
    END_GOBJ();    

public: //当作protected
	StringID _varPos;

	RecordID _idBuff;
	std::vector<DealEntry> _dealsOnPos;
};


class CBgn_Jink:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Jink);

	CBgn_Jink()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
