#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgp_Equip:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Equip);

	virtual const char *GetTypeName()	{		return "装备道具";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_idItem!=RecordID_Invalid)
		{
			if (!_bUnEquip)
				FormatString(s,"装备道具:%s",assist->GetItemName(_idItem));
			else
				FormatString(s,"卸下道具:%s",assist->GetItemName(_idItem));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_Equip,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,_idItem,RecordID_Invalid);
			GELEM_EDITVAR("道具",GVT_U,GSem(GSem_RecordID,"items"),"相关道具");
		GELEM_VAR_INIT(int,_bUnEquip,0);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,"装备道具,卸下道具"),"装备还是卸下道具");
    END_GOBJ();    

public: //当作protected

	RecordID _idItem;
	BOOL _bUnEquip;
};


class CBgn_Equip:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Equip);

	CBgn_Equip()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
