#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_AssignVita:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_AssignVita);

	virtual const char *GetTypeName()	{		return "授予Vita";	}
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
		FormatString(s,"从[%d]个生命虫中随机挑选一些给玩家\n荣耀值需求范围[%d,%d]",nTotal,hnrMin,hnrMax);
	}

    BEGIN_GOBJ_PURE_UID(CBgpGA_AssignVita,1);

		GELEM_VAR_INIT(int,nTotal,10);
			GELEM_EDITVAR("总数",GVT_S,GSem_Interger,"总数");
		GELEM_VAR_INIT(int,hnrMin,10);
			GELEM_EDITVAR("最小荣耀需求",GVT_S,GSem_Interger,"最小荣耀需求");
		GELEM_VAR_INIT(int,hnrMax,10);
			GELEM_EDITVAR("最大荣耀需求",GVT_S,GSem_Interger,"最大荣耀需求");
		GELEM_VAR_INIT(LevelObjID,idPlayerUnit,LevelObjID_Invalid);
			GELEM_EDITVAR("玩家对象ID",GVT_U,GSem_ObjID,"玩家对象ID");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	int nTotal;
	int hnrMin;
	int hnrMax;
	DEFINE_BVR(LevelObjID,idPlayerUnit);
};


class CBgnGA_AssignVita:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_AssignVita);

	CBgnGA_AssignVita()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

