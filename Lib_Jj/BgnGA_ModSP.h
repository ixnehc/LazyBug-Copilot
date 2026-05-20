#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_ModSP:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_ModSP);

	virtual const char *GetTypeName()	{		return "修改SP";	}
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
		if (!bMaxSP)
			FormatString(s,"TalkPlayer的SP+=%s",GetBVRDesc_Int(BVR_ARG(nMod),assist));
		else
			FormatString(s,"TalkPlayer的MaxSP+=%s",GetBVRDesc_Int(BVR_ARG(nMod),assist));
	}

    BEGIN_GOBJ_PURE_UID(CBgpGA_ModSP,1);
		GELEM_VAR_INIT(int,nMod,0);
			GELEM_EDITVAR("修改值",GVT_S,GSem_Interger,"修改值");
			GELEM_BVR();
		GELEM_VAR_INIT(BOOL,bMaxSP,FALSE);
			GELEM_EDITVAR("修改SP最大值",GVT_S,GSem_Boolean,"修改SP最大值");
	END_GOBJ();    

public: //当作protected
	DEFINE_BVR(int,nMod);
	BOOL bMaxSP;

};


class CBgnGA_ModSP:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_ModSP);

	CBgnGA_ModSP()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

