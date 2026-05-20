#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgpLichen:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpLichen);

	virtual const char *GetTypeName()	{		return "地丝控制";	}
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

	enum Op
	{
		Op_None,
		Op_CreateBound,
		Op_CreateTrail,
		Op_Destroy,
	};

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_op==Op_CreateBound)
			FormatString(s,"创建一个绑定在自己身上的地丝");
		if (_op==Op_CreateTrail)
			FormatString(s,"创建一个绑定在自己身上的拖尾地丝");
		if (_op==Op_Destroy)
			s="清除地丝";
	}

    BEGIN_GOBJ_PURE_UID(CBgpLichen,1);

		GELEM_VAR_INIT(int,_op,(int)Op_CreateBound);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,
				"创建绑定地丝:1"		","
				"创建拖尾地丝:2"		","
				"清除地丝:3"	"|半径"
				),"操作模式");

		GELEM_VAR_INIT(float,_radius,1.0f);
			GELEM_EDITVAR("半径",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"地丝半径");
		GELEM_BEHAVIORMEM_OBJID(_nmVar,"地丝句柄变量","地丝句柄变量")
    END_GOBJ();    

public: //当作protected

	int _op;
	float _radius;
	StringID _nmVar;

};


class CBgnLichen:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnLichen);

	CBgnLichen()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

