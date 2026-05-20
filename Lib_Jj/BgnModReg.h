#pragma once

#include "LevelBehavior.h"
#include "resdata/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgp_ModReg:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_ModReg);

	virtual const char *GetTypeName()	{		return "修改寄存器";	}
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
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (nm!=StringID_Invalid)
		{
			if(mode==2)
				FormatString(s,"寄存器(%s)设为%d",StrLib_GetStr(nm),vRef);
			else
			{
				if (vRef>0)
				{
					if (mode==0)
						FormatString(s,"寄存器(%s)+%d",StrLib_GetStr(nm),vRef);
					if (mode==1)
						FormatString(s,"寄存器(%s)-%d",StrLib_GetStr(nm),vRef);
				}
			}
		}
	}

    BEGIN_GOBJ_PURE(CBgp_ModReg,1);
		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"行为图寄存器名称"), "行为图寄存器名称" );
		GELEM_VAR_INIT(int,mode,0);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,"增加,减少,设置"),"使用何种模式修改寄存器");
		GELEM_VAR_INIT(int,vRef,0);
			GELEM_EDITVAR("值",GVT_S,GSem_Interger,"值");
    END_GOBJ();    

public: //当作protected

	StringID nm;
	int vRef;
	int mode;
};


class CBgn_ModReg:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_ModReg);

	CBgn_ModReg()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
