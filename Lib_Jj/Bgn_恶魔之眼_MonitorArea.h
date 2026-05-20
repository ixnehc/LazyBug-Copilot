#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_DevilEye_MonitorArea:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_DevilEye_MonitorArea);

	virtual const char *GetTypeName()	{		return "恶魔之眼_监控区域";	}
	virtual DWORD GetStubCount()
	{
		return 2;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"监测到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="检测指定区域内是否有被恶魔之眼锁定的单位进入";
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_DevilEye_MonitorArea,426,1);
		GELEM_BGP_BASE();

		GELEM_OBJ(BccArea,area);;
			GELEM_EDITOBJ("区域","监控区域");
			GELEM_BVR();

		GELEM_VAR_INIT( StringID,varStatus,StringID_Invalid);
			GELEM_EDITVAR( "恶魔之眼状态保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称"), "恶魔之眼状态保存在哪个变量里");

		GELEM_VAR_INIT(RecordID,idDevilEye,RecordID_Invalid);
			GELEM_EDITVAR("恶魔之眼ID",GVT_U,GSem(GSem_RecordID,"agents"),"Agents");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(BccArea,area);

	StringID varStatus;
	RecordID idDevilEye;

};

struct BMO_DevilEyeStatus;
class CLoGeneralAgent;
class CBgn_DevilEye_MonitorArea:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_DevilEye_MonitorArea);

	CBgn_DevilEye_MonitorArea()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;
	virtual void Update(BGNOutputs &outputs) override;

public:

	BOOL _CheckDevilEye(CLevelObj *lo);
	BMO_DevilEyeStatus *_GetDevilEyeStatus(CLoGeneralAgent *loAgent);

	std::vector<LevelObjID> _devileyes;
	StringID _varStatus;

};

