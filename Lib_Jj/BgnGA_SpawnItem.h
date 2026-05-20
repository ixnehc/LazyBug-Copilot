#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"
#include "behaviorgraph/BgpConstEntry.h"
#include "behaviorgraph/BehaviorParam.h"

struct BccSpawnItems:public BehaviorCustomConst
{
	DEFINE_CLASS(BccSpawnItems);

	BEGIN_GOBJ_PURE(BccSpawnItems,1);

		GELEM_VARVECTOR_INIT(RecordID,items,RecordID_Invalid);
			GELEM_EDITVAR("Items",GVT_U,GSem(GSem_RecordID,"items"),"Items");

	END_GOBJ();

	std::vector<RecordID> items;


};



class CBgpGA_SpawnItem:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_SpawnItem);

	virtual const char *GetTypeName()	{		return "创建道具";	}
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
		s="n/a";
		if((nmSites!=StringID_Invalid)&&(nmItems!=StringID_Invalid))
		{
			FormatString(s,"使用位点集[%s]在道具集[%s]中随机创建一件道具",assist->GetStr(nmSites),assist->GetStr(nmItems));
			if (nmVar!=StringID_Invalid)
				AppendFmtString(s,",结果保存在变量[%s]中",assist->GetStr(nmVar));
		}
		else
		{
			if((nmSites!=StringID_Invalid)&&(idItem!=RecordID_Invalid))
			{
				std::string nm=assist->GetItemName(idItem);
				if (!nm.empty())
				{
					FormatString(s,"使用位点集[%s]创建道具[ %s ]",assist->GetStr(nmSites),nm.c_str());
					if (nmVar!=StringID_Invalid)
						AppendFmtString(s,",结果保存在变量[%s]中",assist->GetStr(nmVar));
				}
			}
		}
	}

    BEGIN_GOBJ_PURE(CBgpGA_SpawnItem,1);

		GELEM_VAR_INIT( StringID,nmItems,StringID_Invalid);	
			GELEM_EDITVAR( "道具集名称", GVT_U, GSem(GSem_StringID,"道具集名称"), "使用哪个道具集创建道具" );

		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);
			GELEM_EDITVAR("道具",GVT_U,GSem(GSem_RecordID,"items"),"创建哪个道具");
		GELEM_VAR_INIT( StringID,nmSites,StringID_Invalid);	
			GELEM_EDITVAR( "位点集名称", GVT_U, GSem(GSem_StringID,"位点集名称"), "使用哪个位点集" );

		GELEM_BEHAVIORMEM_ITEMRECORD(nmVar,"创建道具保存到哪个变量","创建的道具保存在那个变量中")

		GELEM_BPR_CUSTOM(bprSpawnItem,BccSpawnItems,"创建道具参数","创建道具参数");

    END_GOBJ();    

public: //当作protected

	StringID nmSites;
	RecordID idItem;
	StringID nmItems;
	StringID nmVar;

	BPR_Custom bprSpawnItem;
};


class CBgnGA_SpawnItem:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_SpawnItem);

	CBgnGA_SpawnItem()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:


};

