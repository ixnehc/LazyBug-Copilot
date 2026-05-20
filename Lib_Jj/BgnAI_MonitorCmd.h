#pragma once

#include "LevelDefines.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgpCmd_Monitor:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgpCmd_Monitor);

	virtual const char *GetTypeName()	{		return "监控Command";	}
	virtual DWORD GetStubCount()	
	{	
		DWORD c=_cmds.size();
		if (c>9)
			c=9;
		return 1+c;	
	}
	virtual PadStub GetStub(DWORD idx)
	{
		if (idx==0)
			return PadStub("开始",PadStub_In,TRUE);

		idx-=1;

		static std::string str;

		if (idx<_cmds.size())
		{
			StringID cmd=_cmds[idx];
			if (cmd==StringID_Invalid)
				return PadStub("[NoCommand]",PadStub_Out,TRUE);
			FormatString(str,"!!%d",cmd);
			return PadStub(str.c_str(),PadStub_Out,TRUE);
		}

		return PadStub();
	}
	
	virtual BgpCategory GetCategory()	{		return BgpCtgr_AI;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="持续监控是否收到新的Command";
	}

    BEGIN_GOBJ_PURE_UID(CBgpCmd_Monitor,1);
		GELEM_BGP_BASE();

		GELEM_VARVECTOR_INIT(StringID,_cmds,StringID_Invalid);
			GELEM_EDITVAR("监控命令",GVT_U,GSem(GSem_StringID,"AI命令"),"监控各种命令");//XXXXX,More LevelAICmd type
		

	END_GOBJ();    

public: //当作protected

	std::vector<StringID> _cmds;

};

struct LevelAICmd;
class CBgnCmd_Monitor:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnCmd_Monitor);
	CBgnCmd_Monitor()
	{
		_idCurCmd=StringID_Invalid;
	}

	virtual void Destroy();
	virtual void Break(BGNOutputs &outputs);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	StringID _idCurCmd;

};
