#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_RemoveBuff:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_RemoveBuff);

	virtual const char *GetTypeName()	{		return "清除Buff";	}
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
		if (_tpTarget==0)
		{
			if (_idBuff!=RecordID_Invalid)
				FormatString(s,"清除自己的Buff:%s",assist->GetBuffName(_idBuff));
		}
		else
		{
			if (_idBuff!=RecordID_Invalid)
				FormatString(s,"清除TalkPlayer的Buff:%s",assist->GetBuffName(_idBuff));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_RemoveBuff,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(DWORD,_tpTarget,0);
			GELEM_EDITVAR("对象类型",GVT_U,GSem(GSem_Interger,"自己,TalkPlayer"),"目标类型");
		GELEM_VAR_INIT(RecordID,_idBuff,RecordID_Invalid);
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"要清除的Buff");
	END_GOBJ();    

public: //当作protected
	int _tpTarget;
	RecordID _idBuff;
};


class CBgn_RemoveBuff:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_RemoveBuff);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
