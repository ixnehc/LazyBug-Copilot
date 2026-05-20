#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_Vendor:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_Vendor);

	virtual const char *GetTypeName()	{		return "索尔";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (op==0)
			s="初始化索尔";
		if (op==1)
			s="玩家请求刷新商品列表时更新索尔的心情";
		if (op==2)
		{
			if(price!=StringID_Invalid)
				FormatString(s,"玩家购买第%d项商品时更新索尔的心情",idxPrice);
		}
		if (op==3)
			s="计算索尔的心情等级";
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_Vendor,480,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,op,0);
			GELEM_EDITVAR("操作",GVT_S,GSem(GSem_Interger,
				"初始化:0"	"|商品价格保存变量&商品序号&心情等级变量,"
				"计算等级:3"	"|商品价格保存变量&商品序号,"
				"刷新商品列表:1"		"|商品价格保存变量&商品序号&心情等级变量,"
				"购买商品:2"	"|心情等级变量"
				),"操作");
		GELEM_VAR_INIT( StringID,price,StringID_Invalid);
			GELEM_EDITVAR( "商品价格保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "那个变量保存商品的价格");
		GELEM_VAR_INIT(int,idxPrice,0);
			GELEM_EDITVAR("商品序号",GVT_U,GSem(GSem_Interger,"0:0,1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10"),"第几个商品");
		GELEM_VAR_INIT( StringID,lvlMood,StringID_Invalid);
			GELEM_EDITVAR( "心情等级变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Interger"), "那个变量保存索尔的心情等级");
    END_GOBJ();    

public: //当作protected
	
	int op;
	StringID price;
	int idxPrice;
	StringID lvlMood;
};


class CBgnGA_Vendor:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_Vendor);

	CBgnGA_Vendor()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:


};

