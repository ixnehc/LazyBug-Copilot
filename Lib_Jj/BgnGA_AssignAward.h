#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LoAgentRef.h"

#include "records/recordsdefine.h"


class CBgpGA_AssignAward:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_AssignAward);

	virtual const char *GetTypeName()	{		return "授予奖励";	}
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
		if(award!=StringID_Invalid)
		{
			if (idxAward>=0)
				FormatString(s,"将 %s 中的第%d项奖励,赋予玩家",BVRDESC_StringID(award),idxAward);
			else
				FormatString(s,"将 %s 中的所有奖励,赋予玩家",BVRDESC_StringID(award));
		}
	}

    BEGIN_GOBJ_PURE(CBgpGA_AssignAward,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,award,StringID_Invalid);GELEM_BVR();
			GELEM_EDITVAR( "奖励保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "哪个变量里的奖励");
		GELEM_VAR_INIT( StringID,price,StringID_Invalid);
			GELEM_EDITVAR( "奖励价格保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "那个变量保存奖励的价格");
		GELEM_VAR_INIT(int,idxAward,0);
			GELEM_EDITVAR("奖励序号",GVT_U,GSem(GSem_Interger,"所有:-1,0:0,1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10"),"第几个奖励");
		GELEM_OBJVAR(LoAgentRef,refTreasurePickAgent);
			GELEM_EDITOBJ_EX("TreasurePick的Agent引用","TreasurePick的Agent引用",GSem_Unknown);
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(StringID,award);
	StringID price;
	int idxAward;
	DEFINE_BVR(LoAgentRef,refTreasurePickAgent);
};


class CBgnGA_AssignAward:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_AssignAward);

	CBgnGA_AssignAward()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

	BOOL _Assign(CLevelPlayer *player,LevelAward *award,LevelAwardPrice *price);

};

