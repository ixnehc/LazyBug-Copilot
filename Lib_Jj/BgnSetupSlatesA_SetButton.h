#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSetupSlatesA_SetButton:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSetupSlatesA_SetButton);

	virtual const char *GetTypeName()	{		return "设置Slate为Button";	}
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
			FormatString(s,"将{%s}内石板的Button设为 %d 类型,\n并将{%s}内石板设为由这些Button打开",StrLib_GetStr(varGrp),channel,StrLib_GetStr(varLockGrp));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpSetupSlatesA_SetButton,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,varLockGrp,StringID_Invalid);
			GELEM_EDITVAR( "石板组变量(ButtonLock)", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "石板组变量");
		GELEM_VAR_INIT( StringID,varGrp,StringID_Invalid);
			GELEM_EDITVAR( "石板组变量(Button)", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "石板组变量");
		GELEM_VAR_INIT(LevelSlateA_ButtonChannel,channel,LevelSlateA_ButtonChannel_A);
			GELEM_EDITVAR("Button通道",GVT_U,GSem(GSem_Interger,GSemConstraint_LevelSlateA_ButtonChannel),"Button通道");
    END_GOBJ();    

public: //当作protected

	StringID varLockGrp;
	StringID varGrp;
	LevelSlateA_ButtonChannel channel;
};


class CBgnSetupSlatesA_SetButton:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSetupSlatesA_SetButton);

	CBgnSetupSlatesA_SetButton()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
