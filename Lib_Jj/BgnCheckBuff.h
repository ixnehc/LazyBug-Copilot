#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckBuff:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckBuff);

	virtual const char *GetTypeName()	{		return "检测Buff";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"有Buff");
			STUB_OUT(2,"没有Buff");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Buff;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist);

    BEGIN_GOBJ_PURE_UID(CBgp_CheckBuff,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_OBJID(_nmLo,"游戏对象变量","检测哪个游戏对象")
		GELEM_VAR_INIT(RecordID,_idBuff,RecordID_Invalid);GELEM_BVR();
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"要检测的Buff");
		GELEM_VAR_INIT(BOOL,_bCheckParamClass,FALSE);
			GELEM_EDITVAR("检测Buff类型",GVT_S,GSem_Boolean,"是否检测Buff类型");
	END_GOBJ();    

public: //当作protected
	StringID _nmLo;
	DEFINE_BVR(RecordID,_idBuff);
	BOOL _bCheckParamClass;
};


class CBgn_CheckBuff:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckBuff);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
