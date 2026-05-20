#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_Signal:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Signal);

	virtual const char *GetTypeName()	{		return "发送信号";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		extern const char *GetBgnRegName(StringID nm);

		s="n/a";
		if (nm!=StringID_Invalid)
		{
			if (varTarget==StringID_Invalid)
			{
				if (_BVR(area)==StringID_Invalid)
					FormatString(s,"在%.2f米范围内发送一个信号(%s)",radius,StrLib_GetStr(nm));
				else
					FormatString(s,"在%.2f米范围内或指定范围内发送一个信号(%s)",radius,StrLib_GetStr(nm));
			}
			else
				FormatString(s,"向[%s]发送一个信号(%s)",StrLib_GetStr(varTarget),StrLib_GetStr(nm));
		}

	}

    BEGIN_GOBJ_PURE_UID(CBgp_Signal,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "信号名称", GVT_U, GSem(GSem_StringID,"信号名称"), "要侦测的信号名称" );
		GELEM_BEHAVIORMEM_OBJID(varTarget,"信号发送对象变量","将信号发送给指定的对象")
		GELEM_VAR_INIT(float,radius,4.0f);
			GELEM_EDITVAR("发送范围",GVT_F,GSem(GSem_Float,"0.1,100.0,0.05"),"发送范围");
		GELEM_OBJ(BccArea,area);
			GELEM_EDITOBJ("信号发送区域","信号发送区域");
			GELEM_BVR();
	END_GOBJ();    

public: //当作protected
	StringID nm;
	StringID varTarget;
	float radius;
	DEFINE_BVR(BccArea,area);

};


class CBgn_Signal:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Signal);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

