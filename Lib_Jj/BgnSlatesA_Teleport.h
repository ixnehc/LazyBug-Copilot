#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

class CBgpSlatesA_Teleport:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpSlatesA_Teleport);

	virtual const char *GetTypeName()	{		return "石板传送";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_SlatesA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (idBuff!=RecordID_Invalid)
		{
			FormatString(s,"使用Buff[%s]传送当前玩家",assist->GetBuffName(idBuff));
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgpSlatesA_Teleport,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(BOOL,bLeave,FALSE);
			GELEM_EDITVAR("离开石板迷宫",GVT_S,GSem_Boolean,"离开石板迷宫");
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("Buff",GVT_U,GSem(GSem_RecordID,"buffs"),"Buff");
    END_GOBJ();    

public: //当作protected
	RecordID idBuff;
	BOOL bLeave;

};


class CBgnSlatesA_Teleport:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnSlatesA_Teleport);

	CBgnSlatesA_Teleport()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

};
