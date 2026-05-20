#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSetupSlatesA_SetEdgeLock:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSetupSlatesA_SetEdgeLock);

	virtual const char *GetTypeName()	{		return "设置Slate为EdgeLock";	}
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
			FormatString(s,"设置石板组变量{%s}内石板设为EdgeLock",StrLib_GetStr(varGrp));
	}

    BEGIN_GOBJ_PURE_UID(CBgpSetupSlatesA_SetEdgeLock,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,varGrp,StringID_Invalid);
			GELEM_EDITVAR( "石板组变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "石板组变量");
    END_GOBJ();    

public: //当作protected

	StringID varGrp;
};


class CBgnSetupSlatesA_SetEdgeLock:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSetupSlatesA_SetEdgeLock);

	CBgnSetupSlatesA_SetEdgeLock()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
