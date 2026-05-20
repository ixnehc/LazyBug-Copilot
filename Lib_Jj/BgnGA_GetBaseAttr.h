#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"



class CBgpGA_GetBaseAttr:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_GetBaseAttr);

	virtual const char *GetTypeName()	{		return "得到基本属性";	}
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
		const char *sType="n/a";
		switch(tp)
		{
			case 0:
				sType="MaxHP";
				break;
		}
		FormatString(s,"得到TalkPlayer的%s数值,结果保存在变量[%s]中",sType,StrLib_GetStr(var));
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_GetBaseAttr,475,1);
		GELEM_VAR_INIT(DWORD,tp,0);
			GELEM_EDITVAR("取得的数据类型",GVT_U,GSem(GSem_Interger,"MaxHP"),"取得的数据类型");
		GELEM_BEHAVIORMEM_NUMBER(var,"结果变量","存入哪个变量中")
	END_GOBJ();    

public: //当作protected
	int tp;
	StringID var;


};


class CBgnGA_GetBaseAttr:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_GetBaseAttr);

	CBgnGA_GetBaseAttr()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


