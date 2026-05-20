#pragma once

#include "LevelDefines.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpAI_CheckCmd:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgpAI_CheckCmd);

	virtual const char *GetTypeName()	{		return "检查Command";	}
	virtual DWORD GetStubCount()	
	{	
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"检测到");
			STUB_OUT(2,"未检测到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_AI;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_cmd!=StringID_Invalid)
			FormatString(s,"检测当前Command:%s",StrLib_GetStr(_cmd));
	}

    BEGIN_GOBJ_PURE_UID(CBgpAI_CheckCmd,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(StringID,_cmd,StringID_Invalid);
			GELEM_EDITVAR("检查命令",GVT_U,GSem(GSem_StringID,"AI命令"),"检查命令");
		

	END_GOBJ();    

public: //当作protected

	StringID _cmd;

};

class CBgnAI_CheckCmd:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnAI_CheckCmd);
	CBgnAI_CheckCmd()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

};
