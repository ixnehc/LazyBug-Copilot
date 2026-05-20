#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_ModRes:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_ModRes);

	enum Op
	{
		Add=0,
		Sub=1,
	};

	virtual const char *GetTypeName()	{		return "修改资源";	}
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
		if (tp==LevelResource_None)
			s="n/a";
		else
		{
			std::string sOp;
			switch(op)
			{
				case Add:
					FormatString(s,"TalkPlayer的%s+=%s",NameFromResourceType(tp),GetBVRDesc_Int(BVR_ARG(nRef),assist));
					break;
				case Sub:
					FormatString(s,"TalkPlayer的%s-=%s",NameFromResourceType(tp),GetBVRDesc_Int(BVR_ARG(nRef),assist));
					break;
			}
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpGA_ModRes,1);
		GELEM_VAR_INIT(LevelResourceType,tp,LevelResource_None);
			GELEM_EDITVAR("资源类型",GVT_S,GSem(GSem_Interger,LevelResourceType_SemConstraint),"资源类型");
		GELEM_VAR_INIT(Op,op,Add);
			GELEM_EDITVAR("操作",GVT_S,GSem(GSem_Interger,"加,减"),"操作");
		GELEM_VAR_INIT(int,nRef,0);
			GELEM_EDITVAR("参考值",GVT_S,GSem_Interger,"参考值");
			GELEM_BVR();
	END_GOBJ();    

public: //当作protected
	LevelResourceType tp;
	Op op;
	DEFINE_BVR(int,nRef);

};


class CBgnGA_ModRes:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_ModRes);

	CBgnGA_ModRes()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

