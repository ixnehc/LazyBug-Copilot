#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckGrade:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckGrade);

	virtual const char *GetTypeName()	{		return "检测Grade";	}
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
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (gradeMin<gradeMax)
			FormatString(s,"检测当前Grade是否介于[%d,%d]之间",gradeMin,gradeMax);
		if (gradeMin==gradeMax)
			FormatString(s,"检测当前Grade是否为%d",gradeMin);
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckGrade,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(int,gradeMin,1);
			GELEM_EDITVAR("最小等级",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20"),"最大等级");
		GELEM_VAR_INIT(int,gradeMax,1);
			GELEM_EDITVAR("最大等级",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20"),"最大等级");
	END_GOBJ();    

public: //当作protected
	int gradeMin;
	int gradeMax;

};


class CBgn_CheckGrade:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckGrade);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
