#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"



class CBgpGA_RegisterTeleportTarget:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_RegisterTeleportTarget);

	virtual const char *GetTypeName()	{		return "注册Teleport目标位置";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
//		FormatString(s,"注册Teleport目标位置");
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_RegisterTeleportTarget,470,1);


		GELEM_VAR_INIT(int,tpTarget,LevelTeleportTarget::ReturnPoint);
			GELEM_EDITVAR("目标类型",GVT_S,GSem(GSem_Interger,
				"返回点:1"	""
				),"传送目标");
		GELEM_VAR_INIT(float,xOff,0.0f);
			GELEM_EDITVAR("X偏移",GVT_F,GSem(GSem_Float,"-10.0,10.0,0.01"),"X偏移");
		GELEM_VAR_INIT(float,zOff,0.0f);
			GELEM_EDITVAR("Z偏移",GVT_F,GSem(GSem_Float,"-10.0,10.0,0.01"),"Z偏移");
    END_GOBJ();    

public: //当作protected

	LevelTeleportTarget::Type tpTarget;
	float xOff;
	float zOff;

};


class CBgnGA_RegisterTeleportTarget:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_RegisterTeleportTarget);

	CBgnGA_RegisterTeleportTarget()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

