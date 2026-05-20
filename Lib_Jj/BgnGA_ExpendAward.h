#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_ExpendAward:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_ExpendAward);

	virtual const char *GetTypeName()	{		return "花费奖励";	}
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
				FormatString(s,"玩家花费掉 %s 中的第%d项奖励,",BVRDESC_StringID(award),idxAward);
			else
				FormatString(s,"玩家花费掉 %s 中的所有奖励",BVRDESC_StringID(award));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_ExpendAward,455,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,award,StringID_Invalid);GELEM_BVR();
			GELEM_EDITVAR( "奖励保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "哪个变量里的奖励");
		GELEM_VAR_INIT(int,idxAward,0);
			GELEM_EDITVAR("奖励序号",GVT_U,GSem(GSem_Interger,"所有:-1,0:0,1:1,2:2,3:3,4:4,5:5,6:6,7:7,8:8,9:9,10:10"),"第几个奖励");
    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(StringID,award);
	int idxAward;
};


class CBgnGA_ExpendAward:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_ExpendAward);

	CBgnGA_ExpendAward()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

	BOOL _Expend(CLevelPlayer *player,LevelAward *award);

};

