#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_CheckMap:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckMap);

	virtual const char *GetTypeName()	{		return "检测当前地图";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (idMap==RecordID_Invalid)
			s="n/a";
		else
			FormatString(s,"检测当前地图是否为[%s]",assist->GetMapName(idMap));
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_CheckMap,424,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( RecordID,idMap,RecordID_Invalid);
			GELEM_EDITVAR( "地图", GVT_U, GSem(GSem_RecordID,"maps"), "地图" );

	END_GOBJ();    

public: //当作protected

	RecordID idMap;
};

class CBgn_CheckMap:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckMap);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
