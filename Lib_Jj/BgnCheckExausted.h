#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckExausted:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckExausted);

	enum Type
	{
		TalkPlayer=0,
		Me=1,

		ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "检测是否Exausted";	}
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
		if (tp==TalkPlayer)
			s="检测TalkPlayer是否Exausted";
		if (tp==Me)
			s="检测自己是否Exausted";
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_CheckExausted,417,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(Type,tp,TalkPlayer);
			GELEM_EDITVAR("取得类型",GVT_S,GSem(GSem_Interger,"TalkPlayer:0,Me:1"),"取得哪个游戏对象的ID");

	END_GOBJ();    

public: //当作protected
	Type tp;
};


class CBgn_CheckExausted:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckExausted);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


