#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_Centipede_PlayAct:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Centipede_PlayAct);

	virtual const char *GetTypeName()	{		return "巨蜗_蜈蚣_PlayAct";	}
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
		if (_nmAct==StringID_Invalid)
			s="n/a";
		else
		{
			if (_bLoop)
				FormatString(s,"循环播放:%s",assist->GetStr(_nmAct));
			else
				FormatString(s,"播放:%s",assist->GetStr(_nmAct));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_Centipede_PlayAct,413,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(StringID,_nmAct,StringID_Invalid);
			GELEM_EDITVAR("Act名称",GVT_U,GSem(GSem_StringID,"蜈蚣Act名称"),"蜈蚣Act名称");
		GELEM_VAR_INIT(BOOL,_bLoop,FALSE);
			GELEM_EDITVAR("循环播放",GVT_S,GSem_Boolean,"循环播放");
    END_GOBJ();    

public: //当作protected
	StringID _nmAct;
	BOOL _bLoop;

	

};


class CBgn_Centipede_PlayAct:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_Centipede_PlayAct);

	CBgn_Centipede_PlayAct()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;
	virtual void Update(BGNOutputs &outputs) override;

protected:
};

