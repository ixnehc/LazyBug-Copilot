#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckEquip:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckEquip);

	virtual const char *GetTypeName()	{		return "检测装备";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"已装备");
			STUB_OUT(2,"未装备");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_idItem!=RecordID_Invalid)
			FormatString(s,"检测是否装备了%s",assist->GetItemName(_idItem));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckEquip,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,_idItem,RecordID_Invalid);
			GELEM_EDITVAR("装备",GVT_U,GSem(GSem_RecordID,"items"),"要检测的装备");
	END_GOBJ();    

public: //当作protected

	RecordID _idItem;
};


class CBgn_CheckEquip:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckEquip);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
