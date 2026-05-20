#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "LevelEvents.h"




class CBgp_CheckFlies:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckFlies);

	virtual const char *GetTypeName()	{		return "检测飞虫群";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Buff;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		extern const char *GetBVRDesc_AnimTick(AnimTick v,StringID nmRef,FillDescAssist *assist);

		s="n/a";
		if (idBuff!=RecordID_Invalid)
		{
			if (!bWait)
				FormatString(s,"检测%s是否为Enchant状态",assist->GetBuffName(idBuff));
			else
				FormatString(s,"持续检测%s是否为Enchant状态",assist->GetBuffName(idBuff));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckFlies,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);GELEM_UID(2);
			GELEM_EDITVAR("检测Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"检测Buff");
		GELEM_VAR_INIT(BOOL,bWait,TRUE);;GELEM_UID(5);
			GELEM_EDITVAR("持续检测",GVT_S,GSem_Boolean,"持续检测直至检测到");
	END_GOBJ();    

public: //当作protected

	RecordID idBuff;
	BOOL bWait;


};


class CBgn_CheckFlies:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckFlies);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:
	BOOL _Update(BGNOutputs &outputs);
};
