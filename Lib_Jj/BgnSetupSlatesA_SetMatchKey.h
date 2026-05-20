#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSetupSlatesA_SetMatchKey:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSetupSlatesA_SetMatchKey);

	virtual const char *GetTypeName()	{		return "设置Slate的MatchKey";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_SlatesA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (varGrp!=RecordID_Invalid)
			FormatString(s,"设置石板组变量{%s}内石板的MatchKey设为 %d 类型",StrLib_GetStr(varGrp),key);
	}

    BEGIN_GOBJ_PURE_UID(CBgpSetupSlatesA_SetMatchKey,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,varGrp,StringID_Invalid);
			GELEM_EDITVAR( "石板组变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "石板组变量");
		GELEM_VAR_INIT(int,key,0);
			GELEM_EDITVAR("MatchKey",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"MatchKey");
    END_GOBJ();    

public: //当作protected

	StringID varGrp;
	int key;
};


class CBgnSetupSlatesA_SetMatchKey:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSetupSlatesA_SetMatchKey);

	CBgnSetupSlatesA_SetMatchKey()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
