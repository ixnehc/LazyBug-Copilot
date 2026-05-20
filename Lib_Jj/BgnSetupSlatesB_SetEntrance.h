#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSetupSlatesB_SetEntrance:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSetupSlatesB_SetEntrance);

	virtual const char *GetTypeName()	{		return "设置出入口";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_SlatesB;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (grp!=RecordID_Invalid)
		{
			FormatString(s,"将石板组(%s)设为迷宫的出口",StrLib_GetStr(grp));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgpSetupSlatesB_SetEntrance,459,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,grp,StringID_Invalid);	
			GELEM_EDITVAR( "石板组名", GVT_U, GSem(GSem_StringID,"石板组名称"), "石板组名称" );
    END_GOBJ();    

public: //当作protected

	StringID grp;

};


class CBgnSetupSlatesB_SetEntrance:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSetupSlatesB_SetEntrance);

	CBgnSetupSlatesB_SetEntrance()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
