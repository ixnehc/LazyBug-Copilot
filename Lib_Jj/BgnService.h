#pragma once

#include "LevelDefines.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_MonitorService:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_MonitorService);

	virtual const char *GetTypeName()	{		return "监控各种服务";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"监控到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if((_tp!=StringID_Invalid)&&(_nmVar!=StringID_Invalid))
		{
			std::string ss=GetServiceName(_tp);

			if (!ss.empty())
				FormatString(s,"监控[%s]服务,提供服务的单位保存到变量[%s]中",ss.c_str(),assist->GetStr(_nmVar));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_MonitorService,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"监控哪个服务");
		GELEM_BEHAVIORMEM_OBJID(_nmVar,"服务提供者变量","服务提供者保存到哪个变量中")
	END_GOBJ();    

public: //当作protected

	LevelServiceType _tp;

	StringID _nmVar;


};

class CBgn_MonitorService:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_MonitorService);
	CBgn_MonitorService()
	{
		_bMonitoring=FALSE;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:
	BOOL _bMonitoring;

};

class CBgp_FindNearbyService:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_FindNearbyService);

	virtual const char *GetTypeName()	{		return "寻找邻近服务";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_tp!=StringID_Invalid)
		{
			std::string ss=GetServiceName(_tp);

			if (!ss.empty())
			{
				if (_nmPos==StringID_Invalid)
					FormatString(s,"寻找自己附近%.2f米内的[%s]服务\n至少有%d个剩余配额",_radius,ss.c_str(),_nMinAvailableQuato);
				else
					FormatString(s,"寻找[%s]附近%.2f米内的[%s]服务\n至少有%d个剩余配额",assist->GetStr(_nmPos),_radius,ss.c_str(),_nMinAvailableQuato);

				if (_nmServer!=StringID_Invalid)
					AppendFmtString(s,"\n提供服务的单位保存到变量[%s]中",assist->GetStr(_nmServer));

				if (_nmServerPos!=StringID_Invalid)
					AppendFmtString(s,"\n提供服务的单位的位置保存到变量[%s]中",assist->GetStr(_nmServerPos));
			}
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_FindNearbyService,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"寻找哪种服务");
		GELEM_VAR_INIT(float,_radius,2.0f);
			GELEM_EDITVAR("寻找范围",GVT_F,GSem(GSem_Float,"0.1,10.0,0.05"),"寻找范围");
		GELEM_VAR_INIT(DWORD,_nMinAvailableQuato,1);
			GELEM_EDITVAR("至少有多少配额",GVT_U,GSem(GSem_Interger,"0,1,2,3,4,5,6,7,8,9"),"至少有多少配额");
		GELEM_BEHAVIORMEM_POS(_nmPos,"指定位置变量","从哪个变量中取得指定位置");
		GELEM_BEHAVIORMEM_OBJID(_nmServer,"(out)服务提供者变量","服务提供者保存到哪个变量中")
		GELEM_BEHAVIORMEM_POS(_nmServerPos,"(out)服务提供者位置变量","服务提供者保存到哪个变量中");
	END_GOBJ();    

public: //当作protected

	LevelServiceType _tp;
	float _radius;
	int _nMinAvailableQuato;

	StringID _nmPos;

	StringID _nmServer;
	StringID _nmServerPos;

};

class CBgn_FindNearbyService:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_FindNearbyService);
	CBgn_FindNearbyService()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

};


class CBgp_AddService:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_AddService);

	virtual const char *GetTypeName()	{		return "提供各种服务";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_tp!=StringID_Invalid)
		{
			std::string ss=GetServiceName(_tp);

			if (!ss.empty())
			{
				if (!_bGlobal)
					FormatString(s,"提供[%s]服务,有%d个名额",ss.c_str(),_nQuota);
				else
					FormatString(s,"提供全局[%s]服务,有%d个名额",ss.c_str(),_nQuota);
			}
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_AddService,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"提供哪种服务");

		GELEM_VAR_INIT(BOOL,_bGlobal,FALSE);
			GELEM_EDITVAR("全局服务",GVT_S,GSem_Boolean,"是否全局服务");
			
		GELEM_VAR_INIT(int,_nQuota,3);
			GELEM_EDITVAR("名额",GVT_S,GSem(GSem_Interger,"1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9"),"提供几个名额的服务");

	END_GOBJ();    

public: //当作protected

	LevelServiceType _tp;
	BOOL _bGlobal;

	int _nQuota;

};

class CBgn_AddService:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_AddService);
	CBgn_AddService()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

class CBgp_RemoveService:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_RemoveService);

	virtual const char *GetTypeName()	{		return "撤销提供的服务";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_tp!=StringID_Invalid)
		{
			std::string ss=GetServiceName(_tp);
			if (!ss.empty())
				FormatString(s,"撤销提供的[%s]服务",ss.c_str());
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_RemoveService,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"哪种服务");

	END_GOBJ();    

public: //当作protected

	LevelServiceType _tp;

};

class CBgn_RemoveService:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_RemoveService);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



class CBgp_ClaimServiceWithSkill:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_ClaimServiceWithSkill);

	virtual const char *GetTypeName()	{		return "认领服务(使用技能)";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if((_nmVar!=StringID_Invalid)&&(_idSkill!=RecordID_Invalid))
		{
			FormatString(s,"向变量[%s]中的单位认领服务\n使用技能:%s",assist->GetStr(_nmVar),assist->GetSkillName(_idSkill));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_ClaimServiceWithSkill,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"认领哪种服务");

		GELEM_BEHAVIORMEM_OBJID(_nmVar,"服务提供者变量","向哪个服务提供者认领服务")

		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);
			GELEM_EDITVAR("施放技能",GVT_U,GSem(GSem_RecordID,"skills"),"用来认领服务的技能");

		GELEM_VAR_INIT(AnimTick,_durCDOnOK,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("成功后冷却时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"认领服务成功后,多久以后才可再次领取服务,单位为秒");

		GELEM_VAR_INIT(AnimTick,_durCDOnFail,ANIMTICK_FROM_SECOND(3.0f));
			GELEM_EDITVAR("失败后冷却时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"认领服务失败后,多久以后才可再次领取服务,单位为秒");

	END_GOBJ();    

public: //当作protected

	LevelServiceType _tp;

	StringID _nmVar;

	RecordID _idSkill;

	AnimTick _durCDOnOK;
	AnimTick _durCDOnFail;


};

class CBgn_ClaimServiceWithSkill:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_ClaimServiceWithSkill);
	CBgn_ClaimServiceWithSkill()
	{
		_verCasting=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);
	virtual void Break(BGNOutputs &outputs);
	virtual void Destroy();

protected:

	AResult _UpdateClaim();

	LevelObjID _idTarget;
	DWORD _verCasting;

};

class CBgp_ClaimService:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_ClaimService);

	virtual const char *GetTypeName()	{		return "认领服务";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_tp!=LevelServiceType_None)
		{
			FormatString(s,"认领名为[%s]的服务",assist->GetStr(_tp));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_ClaimService,441,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"认领哪种服务");

		GELEM_VAR_INIT(AnimTick,_durCD,ANIMTICK_FROM_SECOND(2.0f));
			GELEM_EDITVAR("冷却时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"认领服务成功后,多久以后才可再次领取服务,单位为秒");

	END_GOBJ();    

public: //当作protected

	LevelServiceType _tp;

	AnimTick _durCD;


};

class CBgn_ClaimService:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_ClaimService);
	CBgn_ClaimService()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgp_ServiceQuota:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_ServiceQuota);

	virtual const char *GetTypeName()	{		return "记录服务剩余名额";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if((_tp!=StringID_Invalid)&&(_nmVar!=StringID_Invalid))
		{
			std::string ss=GetServiceName(_tp);

			if (!ss.empty())
				FormatString(s,"得到[%s]服务的剩余名额,保存到变量[%s]中",ss.c_str(),assist->GetStr(_nmVar));
		}
	}

	BEGIN_GOBJ_PURE_UID(CBgp_ServiceQuota,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"监控哪个服务");

		GELEM_BEHAVIORMEM_INTERGER(_nmVar,"保存变量","得到的服务剩余名额数保存在哪个变量中")

	END_GOBJ();    

public: //当作protected

	LevelServiceType _tp;

	StringID _nmVar;


};

class CBgn_ServiceQuota:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_ServiceQuota);
	CBgn_ServiceQuota()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgp_PreserveService:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_PreserveService);

	virtual const char *GetTypeName()	{		return "占用各种服务";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_tp!=StringID_Invalid)
		{
			std::string ss=GetServiceName(_tp);
			if (_nmServer!=StringID_Invalid)
				FormatString(s,"尝试占用[%s]提供的[%s]类型服务",assist->GetStr(_nmServer),ss.c_str());
			else
				FormatString(s,"尝试占用[%s]类型服务",ss.c_str());
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_PreserveService,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"监控哪个服务");
		GELEM_BEHAVIORMEM_OBJID(_nmServer,"服务提供者变量","服务提供者变量")

	END_GOBJ();    

public: //当作protected
	LevelServiceType _tp;

	StringID _nmServer;
};

class CBgn_PreserveService:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_PreserveService);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

class CBgp_CanPreserveService:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CanPreserveService);

	virtual const char *GetTypeName()	{		return "检测能否占用各种服务";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_tp!=StringID_Invalid)
		{
			std::string ss=GetServiceName(_tp);
			if (_nmServer!=StringID_Invalid)
				FormatString(s,"检测是否能够占用[%s]提供的[%s]类型服务",assist->GetStr(_nmServer),ss.c_str());
			else
				FormatString(s,"检测是否能够占用[%s]类型服务",ss.c_str());
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CanPreserveService,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"监控哪个服务");
		GELEM_BEHAVIORMEM_OBJID(_nmServer,"服务提供者变量","服务提供者变量")

	END_GOBJ();    

public: //当作protected
	LevelServiceType _tp;

	StringID _nmServer;
};

class CBgn_CanPreserveService:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CanPreserveService);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};



class CBgp_CheckPreserveService:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckPreserveService);

	virtual const char *GetTypeName()	{		return "检测是否占用各种服务";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_tp!=StringID_Invalid)
		{
			std::string ss=GetServiceName(_tp);
			if (_nmServer!=StringID_Invalid)
				FormatString(s,"检测自己是否占用[%s]提供的[%s]类型服务",assist->GetStr(_nmServer),ss.c_str());
			else
				FormatString(s,"检测自己是否占用[%s]类型服务",ss.c_str());
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckPreserveService,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"哪个服务");
		GELEM_BEHAVIORMEM_OBJID(_nmServer,"服务提供者变量","服务提供者变量")

	END_GOBJ();    

public: //当作protected
	LevelServiceType _tp;

	StringID _nmServer;
};

class CBgn_CheckPreserveService:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckPreserveService);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgp_GetPreserveServer:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_GetPreserveServer);

	virtual const char *GetTypeName()	{		return "得到占用的服务提供者";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_tp!=StringID_Invalid)
		{
			std::string ss=GetServiceName(_tp);
			if (_nmServer!=StringID_Invalid)
				FormatString(s,"得到自己当前占用的[%s]类型服务的提供者保存在[%s]中",ss.c_str(),assist->GetStr(_nmServer));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_GetPreserveServer,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"哪个服务");
		GELEM_BEHAVIORMEM_OBJID(_nmServer,"(out)服务提供者变量","服务提供者变量")

	END_GOBJ();    

public: //当作protected
	LevelServiceType _tp;

	StringID _nmServer;
};

class CBgn_GetPreserveServer:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_GetPreserveServer);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

class CBgp_GetServicePreserveClient:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_GetServicePreserveClient);

	virtual const char *GetTypeName()	{		return "得到服务的占用者";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_tp!=StringID_Invalid)
		{
			std::string ss=GetServiceName(_tp);
			if (_nmClient!=StringID_Invalid)
				FormatString(s,"得到自己提供的[%s]类型服务的占用者,保存在[%s]中",ss.c_str(),assist->GetStr(_nmClient));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_GetServicePreserveClient,440,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"哪个服务");
		GELEM_BEHAVIORMEM_OBJID(_nmClient,"(out)服务占用者变量","服务占用者变量")

	END_GOBJ();    

public: //当作protected
	LevelServiceType _tp;

	StringID _nmClient;
};

class CBgn_GetServicePreserveClient:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_GetServicePreserveClient);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgp_GetServiceServerCount:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_GetServiceServerCount);

	virtual const char *GetTypeName()	{		return "得到服务提供者数量";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if((_tp!=StringID_Invalid)&&(_nmVar!=StringID_Invalid))
		{
			std::string ss=GetServiceName(_tp);
			FormatString(s,"得到[%s]类型服务的提供者数量,保存到变量[%s]中",ss.c_str(),assist->GetStr(_nmVar));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_GetServiceServerCount,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"哪个服务");
		GELEM_BEHAVIORMEM_INTERGER(_nmVar,"保存变量","得到的数量值保存在哪个变量中")

	END_GOBJ();    

public: //当作protected
	LevelServiceType _tp;

	StringID _nmVar;


};

class CBgn_GetServiceServerCount:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_GetServiceServerCount);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgp_DiscardService:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_DiscardService);

	virtual const char *GetTypeName()	{		return "放弃各种服务";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Service;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(_tp!=StringID_Invalid)
		{
			std::string ss=GetServiceName(_tp);
			FormatString(s,"放弃[%s]类型服务",ss.c_str());
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_DiscardService,1);
		GELEM_BGP_BASE();
		
		GELEM_VAR_INIT(LevelServiceType,_tp,StringID_Invalid);
			GELEM_EDITVAR("服务类型",GVT_U,GSem(GSem_StringID,"服务类型"),"哪个服务");

	END_GOBJ();    

public: //当作protected
	LevelServiceType _tp;
};

class CBgn_DiscardService:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_DiscardService);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
