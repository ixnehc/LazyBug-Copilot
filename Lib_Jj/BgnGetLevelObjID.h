#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_GetLevelObjID:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_GetLevelObjID);

	enum Type
	{
		TalkPlayer=0,
		Me=1,
		LockPlayer=2,

		ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "取得Player单位ID";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (nmLo==StringID_Invalid)
			s="n/a";
		else
		{
			if (tp==TalkPlayer)
				FormatString(s,"取得TalkPlayer的单位ID,存入变量%s",assist->GetStr(nmLo));
			if (tp==Me)
				FormatString(s,"取得自己的单位ID,存入变量%s",assist->GetStr(nmLo));
			if (tp==LockPlayer)
				FormatString(s,"取得LockPlayer的单位ID,存入变量%s",assist->GetStr(nmLo));

		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_GetLevelObjID,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(Type,tp,TalkPlayer);
			GELEM_EDITVAR("取得类型",GVT_S,GSem(GSem_Interger,"TalkPlayer:0,Me:1,LockPlayer:2"),"取得哪个游戏对象的ID");

		GELEM_BEHAVIORMEM_OBJID(nmLo,"游戏对象变量","存入哪个变量中")
	END_GOBJ();    

public: //当作protected
	Type tp;
	StringID nmLo;
};


class CBgn_GetLevelObjID:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_GetLevelObjID);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


