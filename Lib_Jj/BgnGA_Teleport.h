#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "records/recordsdefine.h"


struct BP_TeleportSite
{
	BEGIN_GOBJ_PURE(BP_TeleportSite,1);

		GELEM_VAR_INIT(RecordID,idMap,RecordID_Invalid);
			GELEM_EDITVAR("地图名称",GVT_U,GSem(GSem_RecordID,"maps"),"要传送到哪张地图上");

		GELEM_VAR_INIT( StringID,nmSite,StringID_Invalid);	
			GELEM_EDITVAR( "位置名称", GVT_U, GSem(GSem_StringID,"地图传送点名称"), "传送到哪个传送点上" );

	END_GOBJ();

	RecordID idMap;
	StringID nmSite;

};


class CBgpGA_Teleport:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_Teleport);

	virtual const char *GetTypeName()	{		return "传送";	}
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
		FormatString(s,"将主角传送到指定地点");
	}

    BEGIN_GOBJ_PURE(CBgpGA_Teleport,1);


		GELEM_VAR_INIT(int,tpTarget,LevelTeleportTarget::None);
			GELEM_EDITVAR("目标类型",GVT_S,GSem(GSem_Interger,
				"指定目标:0"		","
				"返回点:1"	"|传送参数"
				),"传送目标");
			GELEM_BVR();
		GELEM_OBJ(BP_TeleportSite,param);
			GELEM_EDITOBJ("传送参数","传送参数");
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(LevelTeleportTarget::Type,tpTarget);

	DEFINE_BVR(BP_TeleportSite,param);

};


class CBgnGA_Teleport:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_Teleport);

	CBgnGA_Teleport()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

