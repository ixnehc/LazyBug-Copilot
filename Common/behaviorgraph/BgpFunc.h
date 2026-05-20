#pragma once

#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/Behavior.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "behaviorgraph/BehaviorValue.h"

class CBgp_Func:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Func);

	virtual const char *GetTypeName()	{		return "函数";	};
	virtual DWORD GetStubCount()	{		return 1;	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_OUT(0,"开始");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Func;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}
	virtual const char *GetShowName()
	{
		if (_nm==StringID_Invalid)
			return __super::GetShowName();
		return StrLib_GetStr(_nm);
	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_nm==StringID_Invalid)
			s="n/a";
		else
		{
			std::string ss;
			ss="";

			if (_declares2.size()>0)
			{
				ss="";
				extern void FillValuesDesc(std::string &s,FillDescAssist *assist,std::vector<BhvValDeclare*> &declares);
				std::vector<BhvValDeclare *> declares;
				declares.resize(_declares2.size());
				for (int i=0;i<_declares2.size();i++)
					declares[i]=&_declares2[i];
				FillValuesDesc(ss,assist,declares);
				s+=ss;
			}
		}

	}


    BEGIN_GOBJ_PURE_UID(CBgp_Func,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"行为图函数名称"), "函数的名称" );

		GELEM_OBJVECTOR(BhvParamDeclare,_declares2);
			GELEM_UID(1);
			GELEM_EDITOBJ("参数列表2","参数列表");

    END_GOBJ();    

public: //当作protected
	StringID _nm;
	std::vector<BhvParamDeclare> _declares2;
};


class CBgp_Call:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Call);

	virtual const char *GetTypeName()	{		return "函数调用";	};
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Func;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{

		if (_nm!=StringID_Invalid)
			s=s+StrLib_GetStr(_nm);
		else
			s="n/a";
	}


    BEGIN_GOBJ_PURE_UID(CBgp_Call,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"行为图函数名称引用"), "函数的名称" );

		GELEM_OBJVAR(BhvValues,_params)
			GELEM_VERSION(1);
			GELEM_EDITOBJ("参数","参数");

    END_GOBJ();    

public: //当作protected
	StringID _nm;

	BhvValues _params;

	//Runtime 
	BehaviorCall _call;
	std::vector<BhvVal> _valuesDef;
};

