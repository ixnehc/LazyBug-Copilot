#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckCounter:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckCounter);

	enum Op
	{
		EQ=0,
		NE,
		GE,
		GT,
		LE,
		LT,
	};

	virtual const char *GetTypeName()	{		return "检测计数器";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (nm==StringID_Invalid)
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
			FormatString(s,"%s%s%d ?",StrLib_GetStr(nm),sOp.c_str(),vRef);
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckCounter,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"行为图计数器名称"), "行为图计数器名称" );

		GELEM_VAR_INIT(Op,op,EQ);
			GELEM_EDITVAR("比较操作",GVT_S,GSem(GSem_Interger,"等于,不等于,大于等于,大于,小于等于,小于"),"比较操作");
		GELEM_VAR_INIT(int,vRef,0);
			GELEM_EDITVAR("比较值",GVT_S,GSem_Interger,"比较值");
	END_GOBJ();    

public: //当作protected
	StringID nm;
	Op op;
	int vRef;
};


class CBgn_CheckCounter:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_CheckCounter);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
