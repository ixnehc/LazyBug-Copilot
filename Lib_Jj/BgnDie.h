#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgp_Die:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Die);

	virtual const char *GetTypeName()	{		return "死亡";	}
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
		if (_idDeathBuff!=RecordID_Invalid)
			FormatString(s,"死亡(%s)",assist->GetBuffName(_idDeathBuff));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Die,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,_idDeathBuff,RecordID_Invalid);
			GELEM_EDITVAR("死亡Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"死亡Buff");
    END_GOBJ();    

public: //当作protected

	RecordID _idDeathBuff;
};


class CBgn_Die:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Die);

	CBgn_Die()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
