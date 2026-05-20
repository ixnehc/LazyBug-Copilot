#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_KnockDown:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_KnockDown);

	virtual const char *GetTypeName()	{		return "倒下";	}
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
		s="n/a";
		if (_idBuff!=RecordID_Invalid)
			FormatString(s,"原地倒下,使用Buff[%s]",assist->GetBuffName(_idBuff));
	}

    BEGIN_GOBJ_PURE_UID(CBgp_KnockDown,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(RecordID,_idBuff,RecordID_Invalid);
			GELEM_EDITVAR("倒下的Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"倒下的Buff,主要必须是\"击倒\"的Buff");
	END_GOBJ();    

public: //当作protected

	RecordID _idBuff;

};

class CBgn_KnockDown:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_KnockDown);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

};
