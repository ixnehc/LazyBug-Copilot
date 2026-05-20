#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "records/recordsdefine.h"


class CBgpGA_RollGoods:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_RollGoods);

	virtual const char *GetTypeName()	{		return "随机Goods";	}
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

		if (mode==0)
			FormatString(s,"为TalkPlayer产生Goods,将结果保存在[%s]中,将价格保存在[%s]中",BVRDESC_StringID(awards),BVRDESC_StringID(prices));
		if (mode==1)
			FormatString(s,"(补货模式)为TalkPlayer产生Goods,将结果保存在[%s]中,将价格保存在[%s]中",BVRDESC_StringID(awards),BVRDESC_StringID(prices));
		if (mode==2)
			FormatString(s,"(更新价格)为TalkPlayer更新保存在[%s]中的价格",BVRDESC_StringID(prices));

	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_RollGoods,479,1);

		GELEM_VAR_INIT(int,mode,0);GELEM_UID(1);
			GELEM_EDITVAR("工作模式",GVT_S,GSem(GSem_Interger,
				"缺省模式:0"		","
				"补货模式:1"		"|个数,"
				"更新价格模式:2" "|个数&Goods保存变量"
				),"工作模式");
		GELEM_VAR_INIT(int,count,3);GELEM_BVR();GELEM_UID(2);
			GELEM_EDITVAR("个数",GVT_S,GSem_Interger,"个数");
		GELEM_VAR_INIT( StringID,awards,StringID_Invalid);GELEM_BVR();GELEM_UID(3);
			GELEM_EDITVAR( "Goods保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "Goods保存在哪个变量里");
		GELEM_VAR_INIT( StringID,prices,StringID_Invalid);GELEM_BVR();GELEM_UID(4);
			GELEM_EDITVAR( "Goods的价格保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "Goods价格保存在哪个变量里");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(StringID,awards);
	DEFINE_BVR(StringID,prices);
	DEFINE_BVR(DWORD,count);
	int mode;

};


class CBgnGA_RollGoods:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_RollGoods);

	CBgnGA_RollGoods()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:


};

