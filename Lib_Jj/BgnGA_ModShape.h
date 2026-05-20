#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpGA_ModShape:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_ModShape);

	enum Op
	{
		Disable=0,
		Enable=1,
		SetName=2,
	};

	virtual const char *GetTypeName()	{		return "修改Shape";	}
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
		if (op==Disable)
			s="使自己的Shape无效";
		if (op==Enable)
			s="使自己的Shape有效";
		if (op==SetName)
			FormatString(s,"设置Shape名称为%s",StrLib_GetStr(nmShape));
	}

    BEGIN_GOBJ_PURE_UID(CBgpGA_ModShape,1);
		GELEM_VAR_INIT(Op,op,Disable);
			GELEM_EDITVAR("操作",GVT_S,GSem(GSem_Interger,
				"使无效:0"		"|Shape名称,"
				"使有效:1"	"|Shape名称,"
				"设置Shape名称:2"		""
				),"操作");
		GELEM_VAR_INIT( StringID,nmShape,StringID_Invalid);	
			GELEM_EDITVAR( "Shape名称", GVT_U, GSem(GSem_StringID,"Shape名称"), "Shape名称" );
	END_GOBJ();    

public: //当作protected
	Op op;
	StringID nmShape;

};


class CBgnGA_ModShape:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_ModShape);

	CBgnGA_ModShape()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

