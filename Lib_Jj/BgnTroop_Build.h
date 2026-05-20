#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelTroops.h"

#include "records/recordsdefine.h"

#include "behaviorgraph/BehaviorCustomConst.h"
#include "behaviorgraph/BehaviorParam.h"


struct BuildTroopParam
{
	const char *GetBrief(void *param);

	BEGIN_GOBJ_PURE(BuildTroopParam,1);
		GOBJ_GETBRIEF_FUNC(GetBrief);

		GELEM_VAR_INIT(float,scoreTotal,150.0f);
			GELEM_EDITVAR("单位总分",GVT_F,GSem(GSem_Float,"0.1,10000.0,1.0f"),"总分");
		GELEM_OBJVECTOR(LevelTroopDesc,entries);
			GELEM_EDITOBJ("单位列表","单位列表")
	END_GOBJ();

	std::vector<LevelTroopDesc> entries;
	float scoreTotal;
};


class CBgpTroop_Build:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpTroop_Build);

	virtual const char *GetTypeName()	{		return "组建Troop";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}


	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
// 		extern const char *GetBgnRegName(StringID nm);
// 
// 		if ((_param.bRef)&&(_param.nmRef!=StringID_Invalid))
// 			FormatString(s,"参数[%s]\r\n,",assist->GetStr(_param.nmRef));
// 		else
// 			s="自定义参数\r\n";
// 		if (_troop!=StringID_Invalid)
// 			AppendFmtString(s,"组建Troop[%s]中",assist->GetStr(_troop));
	}

    BEGIN_GOBJ_PURE_UID(CBgpTroop_Build,1);

		GELEM_OBJ(BuildTroopParam,_param);
			GELEM_EDITOBJ("组建参数","组建参数");
			GELEM_BVR();
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","组建哪个Troop");
			GELEM_BVR();
		GELEM_VAR_INIT(BOOL,_bForceRecreate,TRUE);
			GELEM_EDITVAR("强制重新组建",GVT_S,GSem_Boolean,"是否强制重新组建");
		GELEM_VAR_INIT(int,_countOverride,-1);
			GELEM_EDITVAR("指定个数",GVT_S,GSem_Interger,"指定个数");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(BuildTroopParam,_param);
	DEFINE_BVR(StringID,_troop);
	BOOL _bForceRecreate;
	DEFINE_BVR(int,_countOverride);
};


class CBgnTroop_Build:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_Build);

	CBgnTroop_Build()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

};

