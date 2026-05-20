#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelDeal.h"

#include "records/recordsdefine.h"

class CBgp_Possess:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Possess);

	virtual const char *GetTypeName()	{		return "附体";	}
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
		if ((_idBuff!=RecordID_Invalid)&&(_varTarget!=StringID_Invalid))
			FormatString(s,"附体(%s),至目标[%s]",assist->GetBuffName(_idBuff),assist->GetStr(_varTarget));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Possess,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_OBJID(_varTarget,"对象变量","目标对象变量")
		GELEM_VAR_INIT(RecordID,_idBuff,RecordID_Invalid);
			GELEM_EDITVAR("附体Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"附体Buff");
    END_GOBJ();    

public: //当作protected
	StringID _varTarget;

	RecordID _idBuff;
};


class CBgn_Possess:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Possess);

	CBgn_Possess()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
