#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_ModBaseAttr:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_ModBaseAttr);

	virtual const char *GetTypeName()	{		return "修改基本属性";	}
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
		if (tp==0)
			FormatString(s,"TalkPlayer的Str+=%s",GetBVRDesc_Int(BVR_ARG(nMod),assist));
		if (tp==1)
			FormatString(s,"TalkPlayer的Magic+=%s",GetBVRDesc_Int(BVR_ARG(nMod),assist));
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_ModBaseAttr,474,1);
		GELEM_VAR_INIT(DWORD,tp,0);
			GELEM_EDITVAR("Mod类型",GVT_U,GSem(GSem_Interger,"修改Str,修改Magic"),"Mod类型");
		GELEM_VAR_INIT(int,nMod,0);
			GELEM_EDITVAR("修改值",GVT_S,GSem_Interger,"修改值");
			GELEM_BVR();
END_GOBJ();    

public: //当作protected
	int tp;
	DEFINE_BVR(int,nMod);

};


class CBgnGA_ModBaseAttr:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_ModBaseAttr);

	CBgnGA_ModBaseAttr()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

