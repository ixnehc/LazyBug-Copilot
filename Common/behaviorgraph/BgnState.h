#pragma once

#include "BehaviorGraphPads.h"
#include "../records/recordsdefine.h"
#include "../anim/animdefines.h"
#include "BehaviorDefines.h"


class CBgp_State:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_State);

	virtual const char *GetTypeName()	{		return "状态";	};
	virtual DWORD GetStubCount()	{		return 2;	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_OUT(0,"触发");
			STUB_C_OUT(1,"开始条件");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_State;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual const char *GetFolderShowName()
	{
		if (!_nameFolder.empty())
			return _nameFolder.c_str();
		
		if (_nm!=StringID_Invalid)
			return StrLib_GetStr(_nm);

		return "";
	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_nm==StringID_Invalid)
			s="n/a";
		else
		{
			s=StrLib_GetStr(_nm);
			if(_flag!=0)
			{
				s+="\n";
				if (_flag&BehaviorMemFlag_Persist)
					s+="需要保存";
				if (_flag&BehaviorMemFlag_Sync)
				{
					if (_flag&BehaviorMemFlag_Persist)
						s+=",";
					s+="需要网络同步";
				}
			}
		}
	}


    BEGIN_GOBJ_PURE_UID(CBgp_State,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "状态名称", GVT_U, GSem(GSem_StringID,"行为图状态名称"), "状态的名称" );
		GELEM_VAR_INIT(BehaviorMemFlag,_flag,BehaviorMemFlag_None);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"需要保存:1,需要网络同步:2"),"标志");
    END_GOBJ();    

public: //当作protected
	StringID _nm;
	BehaviorMemFlag _flag;
};

class CBgn_State:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_State);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void StartPending(DWORD iStb);
	virtual void Update(BGNOutputs &outputs);
};



class CBgp_SwitchState:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_SwitchState);

	virtual const char *GetTypeName()	{		return "切换状态";	};
	virtual DWORD GetStubCount()
	{
		return 1;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
		END_STUB()
		
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_State;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_nm!=StringID_Invalid)
			s=StrLib_GetStr(_nm);
	}


    BEGIN_GOBJ_PURE_UID(CBgp_SwitchState,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "状态名称", GVT_U, GSem(GSem_StringID,"行为图状态名称引用"), "状态的名称" );

    END_GOBJ();    

public: //当作protected
	StringID _nm;
};

class CBgn_SwitchState:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_SwitchState);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

};



class CBgp_ActivateStates:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_ActivateStates);

	virtual const char *GetTypeName()	{		return "激活状态";	};
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_State;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		BOOL bState=FALSE;
		for (int i=0;i<_nms.size();i++)
		{
			if (_nms[i]!=StringID_Invalid)
			{
				bState=TRUE;
				break;
			}
		}
		if (bState)
		{
			for (int i=0;i<_nms.size();i++)
			{
				if (_nms[i]!=StringID_Invalid)
					s=s+StrLib_GetStr(_nms[i])+"\n";
			}
		}
		else
			s="n/a";
	}

    BEGIN_GOBJ_PURE_UID(CBgp_ActivateStates,1);
		GELEM_BGP_BASE();

		GELEM_VARVECTOR_INIT( StringID,_nms,StringID_Invalid);	
			GELEM_EDITVAR( "状态名称", GVT_U, GSem(GSem_StringID,"行为图状态名称引用"), "状态的名称" );

    END_GOBJ();    

public: //当作protected
	std::vector<StringID> _nms;
};

class CBgn_ActivateStates:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_ActivateStates);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

};



class CBgp_TerminateState:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_TerminateState);

	virtual const char *GetTypeName()	{		return "中止状态";	};
	virtual DWORD GetStubCount()
	{
		return 1;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"中止");
		END_STUB()
		
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_State;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="";
	}


    BEGIN_GOBJ_PURE_UID(CBgp_TerminateState,1);
		GELEM_BGP_BASE();


    END_GOBJ();    

public: //当作protected
};


class CBgn_TerminateState:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_TerminateState);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

};

