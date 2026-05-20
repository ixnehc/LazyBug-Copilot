#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

struct DailyGoldsDesc
{
	RecordID idItem;
	std::vector<i_math::matrix43f> mats;
	int nMin;
	int nMax;

	BEGIN_GOBJ_PURE(DailyGoldsDesc,1);

		GELEM_VARVECTOR(i_math::matrix43f,mats)
			GELEM_EDITVAR("位点",GVT_Fx12,GSem(GSem_Unknown,"MatSetLS"),"位点");

		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);
			GELEM_EDITVAR("道具ID",GVT_U,GSem(GSem_RecordID,"items"),"创建哪个道具");

		GELEM_VAR_INIT( int,nMin,20);	
			GELEM_EDITVAR( "最小数量", GVT_S, GSem_Interger, "最小值" );

		GELEM_VAR_INIT( int,nMax,30);	
			GELEM_EDITVAR( "最大数量", GVT_S, GSem_Interger, "最大值" );

	END_GOBJ();

};

struct DailyItemsParam
{
	DailyGoldsDesc descGold;
	BEGIN_GOBJ_PURE(DailyItemsParam,1);

		GELEM_OBJ(DailyGoldsDesc,descGold);
			GELEM_EDITOBJ("金子参数","金子参数");

	END_GOBJ();

};


class CBgpGA_SpawnDailyItems:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_SpawnDailyItems);

	virtual const char *GetTypeName()	{		return "创建每日资源";	}
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
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_SpawnDailyItems,433,1);

		GELEM_OBJ(DailyItemsParam,param); 
			GELEM_EDITOBJ("参数","参数");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(DailyItemsParam,param);
};


class CBgnGA_SpawnDailyItems:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_SpawnDailyItems);

	CBgnGA_SpawnDailyItems()
	{
	}

	void Start(DWORD iStb,BGNOutputs &outputs) override;
	void Update(BGNOutputs &outputs) override;

protected:
	void _Update(BGNOutputs &outputs);



};

