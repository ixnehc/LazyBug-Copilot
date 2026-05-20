#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"



class CBgp_CheckItem:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_CheckItem);

	virtual const char *GetTypeName()	{		return "检查道具";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"有");
			STUB_OUT(2,"没有");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (!bCheckMemory)
			FormatString(s,"检查玩家是否有[%s]道具",GetBVRDesc_ItemID(BVR_ARG(idItem),assist));
		else
			FormatString(s,"检查玩家是否当前拥有或者曾经拥有过[%s]道具",GetBVRDesc_ItemID(BVR_ARG(idItem),assist));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckItem,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);
			GELEM_EDITVAR("道具",GVT_U,GSem(GSem_RecordID,"items"),"道具");
			GELEM_BVR();

		GELEM_VAR_INIT(LevelObjID,idPlayerUnit,LevelObjID_Invalid);
			GELEM_EDITVAR("玩家对象ID", GVT_U, GSem_ObjID, "" );
			GELEM_BVR();

		GELEM_VAR_INIT(BOOL,bCheckMemory,FALSE);
			GELEM_EDITVAR("检测是否曾经拥有过",GVT_S,GSem_Boolean,"检测是否曾经拥有过");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(RecordID,idItem);
	DEFINE_BVR(LevelObjID,idPlayerUnit);
	BOOL bCheckMemory;
};


class CBgn_CheckItem:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckItem);

	CBgn_CheckItem()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

