#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSetupSlatesA_SetSwitch:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSetupSlatesA_SetSwitch);

	virtual const char *GetTypeName()	{		return "设置Slate为Switch";	}
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
		if (varGrp!=StringID_Invalid)
		{
			FormatString(s,"将{%s}内石板的Switch设为 %d 类型,\n并将{%s}内石板设为由这些Switch打开",StrLib_GetStr(varGrp),channel,StrLib_GetStr(varLockGrp));
			if (varPointerGrp!=StringID_Invalid)
				AppendFmtString(s,"\n同时将{%s}内的石板作为这些Switch的指针",StrLib_GetStr(varPointerGrp));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpSetupSlatesA_SetSwitch,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,varLockGrp,StringID_Invalid);
			GELEM_EDITVAR( "石板组变量(SwitchLock)", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "石板组变量");
		GELEM_VAR_INIT( StringID,varGrp,StringID_Invalid);
			GELEM_EDITVAR( "石板组变量(Switch)", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "石板组变量");
		GELEM_VAR_INIT( StringID,varPointerGrp,StringID_Invalid);
			GELEM_EDITVAR( "石板组变量(SwitchPointer)", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "石板组变量");
		GELEM_VAR_INIT(LevelSlateA_SwitchChannel,channel,LevelSlateA_SwitchChannel_A);
			GELEM_EDITVAR("Switch通道",GVT_U,GSem(GSem_Interger,GSemConstraint_LevelSlateA_SwitchChannel),"Switch通道");
    END_GOBJ();    

public: //当作protected

	StringID varLockGrp;
	StringID varGrp;
	StringID varPointerGrp;
	LevelSlateA_SwitchChannel channel;
};


class CBgnSetupSlatesA_SetSwitch:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSetupSlatesA_SetSwitch);

	CBgnSetupSlatesA_SetSwitch()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
