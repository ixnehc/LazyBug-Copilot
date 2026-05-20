#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_ModHP:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_ModHP);

	virtual const char *GetTypeName()	{		return "修改HP";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"结束");
			STUB_OUT(2,"中断");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (tpTarget==0)
		{
			if (!bMaxHP)
				FormatString(s,"TalkPlayer的HP+=%s",GetBVRDesc_Int(BVR_ARG(nMod),assist));
			else
				FormatString(s,"TalkPlayer的MaxHP+=%s",GetBVRDesc_Int(BVR_ARG(nMod),assist));
		}
		if (tpTarget==1)
		{
			if (!bMaxHP)
				FormatString(s,"自己的HP+=%s",GetBVRDesc_Int(BVR_ARG(nMod),assist));
			else
				FormatString(s,"自己的MaxHP+=%s",GetBVRDesc_Int(BVR_ARG(nMod),assist));
		}
		if (dur>0)
			AppendFmtString(s,",花%.2f秒修改完",ANIMTICK_TO_SECOND(dur));
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_ModHP,422,1);
		GELEM_VAR_INIT(int,tpTarget,0);
			GELEM_EDITVAR("修改目标",GVT_U,GSem(GSem_Interger,"TalkPlayer,自己"),"修改目标");
		GELEM_VAR_INIT(int,nMod,0);
			GELEM_EDITVAR("修改值",GVT_S,GSem_Interger,"修改值");
			GELEM_BVR();
		GELEM_VAR_INIT(BOOL,bMaxHP,FALSE);
			GELEM_EDITVAR("修改HP最大值",GVT_S,GSem_Boolean,"修改HP最大值");
		GELEM_VAR_INIT(AnimTick,dur,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("时长",GVT_U,GSem(GSem_AnimTick,"0,100,0.1"),"在多长时间内修改完");
	END_GOBJ();    

public: //当作protected
	int tpTarget;
	DEFINE_BVR(int,nMod);
	BOOL bMaxHP;
	AnimTick dur;

};


class CBgnGA_ModHP:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_ModHP);

	CBgnGA_ModHP()
	{
		_tStartMod=0;
		_nModed=0;
	}

	void Start(DWORD iStb,BGNOutputs &outputs) override;
	void Update(BGNOutputs &outputs) override;

protected:
	BOOL _DoMod(int nMod);

	AnimTick _tStartMod;
	int _nModed;

};

