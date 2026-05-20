#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "behaviorgraph/BehaviorCustomConst.h"
#include "behaviorgraph/BehaviorParam.h"

#include "LevelTroops.h"


class CBgp_MakeBuff:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_MakeBuff);

	virtual const char *GetTypeName()	{		return "添加Buff";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Buff;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_tpTarget==0)
			FormatString(s,"为自己添加Buff:%s",GetBVRDesc_BuffID(BVR_ARG(_idBuff),assist));
		if (_tpTarget==1)
			FormatString(s,"为TalkPlayer添加Buff:%s",GetBVRDesc_BuffID(BVR_ARG(_idBuff),assist));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_MakeBuff,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(DWORD,_tpTarget,0);
			GELEM_EDITVAR("对象类型",GVT_U,GSem(GSem_Interger,"为自己加,为TalkPlayer加"),"对谁加Buff");
		GELEM_VAR_INIT(RecordID,_idBuff,RecordID_Invalid);
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");
			GELEM_BVR();
		GELEM_VAR_INIT(AnimTick,_dur,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("持续时间",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"Buff持续时间,0表示永远");

    END_GOBJ();    

public: //当作protected

	int _tpTarget;

	DEFINE_BVR(RecordID,_idBuff);
	AnimTick _dur;
};


class CBgn_MakeBuff:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_MakeBuff);

	CBgn_MakeBuff()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


