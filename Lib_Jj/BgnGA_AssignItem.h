#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_AssignItem:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_AssignItem);

	virtual const char *GetTypeName()	{		return "授予道具";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"将道具[%s]赋予玩家",GetBVRDesc_ItemID(BVR_ARG(idItem),assist));
	}

    BEGIN_GOBJ_PURE(CBgpGA_AssignItem,1);

		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);GELEM_BVR();
			GELEM_EDITVAR("Item",GVT_U,GSem(GSem_RecordID,"items"),"Item");

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(RecordID,idItem);
};


class CBgnGA_AssignItem:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_AssignItem);

	CBgnGA_AssignItem()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

