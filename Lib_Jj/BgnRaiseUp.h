#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_RaiseUp:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_RaiseUp);

	virtual const char *GetTypeName()	{		return "爬起";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Action;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_idBuff!=RecordID_Invalid)
			FormatString(s,"爬起,中止Buff[%s]",assist->GetBuffName(_idBuff));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_RaiseUp,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,_idBuff,RecordID_Invalid);
			GELEM_EDITVAR("中止Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"爬起时中止哪个Buff");
	END_GOBJ();    

public: //当作protected

	RecordID _idBuff;

};

class CBgn_RaiseUp:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_RaiseUp);
	CBgn_RaiseUp()
	{
		_idBuff=LevelBuffID_Invalid;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

	LevelBuffID _idBuff;

};
