#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckPlayerRes:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckPlayerRes);

	enum Op
	{
		EQ=0,
		NE,
		GE,
		GT,
		LE,
		LT,
	};

	virtual const char *GetTypeName()	{		return "检测Player资源";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"是");
			STUB_OUT(2,"否");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (tp==LevelResource_None)
			s="n/a";
		else
		{
			std::string sOp;
			switch(op)
			{
				case EQ:
					sOp="等于";break;
				case NE:
					sOp="不等于";break;
				case GE:
					sOp="大于等于";break;
				case GT:
					sOp="大于";break;
				case LE:
					sOp="小于等于";break;
				case LT:
					sOp="小于";break;
			}
			FormatString(s,"TalkPlayer的%s%s%s ?",NameFromResourceType(tp),sOp.c_str(),GetBVRDesc_Int(BVR_ARG(nRef),assist));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckPlayerRes,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelResourceType,tp,LevelResource_None);
			GELEM_EDITVAR("资源类型",GVT_S,GSem(GSem_Interger,LevelResourceType_SemConstraint),"资源类型");
		GELEM_VAR_INIT(Op,op,EQ);
			GELEM_EDITVAR("比较操作",GVT_S,GSem(GSem_Interger,"等于,不等于,大于等于,大于,小于等于,小于"),"比较操作");
		GELEM_VAR_INIT(int,nRef,0);
			GELEM_EDITVAR("参考值",GVT_S,GSem_Interger,"参考值");
			GELEM_BVR();
	END_GOBJ();    

public: //当作protected
	LevelResourceType tp;
	Op op;
	DEFINE_BVR(int,nRef);
};


class CBgn_CheckPlayerRes:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckPlayerRes);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


