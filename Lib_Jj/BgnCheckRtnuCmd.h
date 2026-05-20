#pragma once

#include "LevelDefines.h"

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_CheckRtnuCmd:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckRtnuCmd);

	virtual const char *GetTypeName()	{		return "监控是否有随从命令";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"收到");
			STUB_OUT(2,"未收到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckRtnuCmd,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(LevelRtnuCmdType,_tp,LevelRtnuCmd_None);
			GELEM_EDITVAR("命令类型",GVT_S,GSem(GSem_Interger,LevelRtnuCmdType_SemConstraint),"监控的命令类型");

		GELEM_VAR_INIT(RecordID,_idSkill,RecordID_Invalid);
			GELEM_EDITVAR("施放技能",GVT_U,GSem(GSem_RecordID,"skills"),"施放哪个技能");

		GELEM_BEHAVIORMEM_OBJID(_nmTarget,"对象变量","监控到的目标存放在哪个变量中")

	END_GOBJ();    

public: //当作protected

	LevelRtnuCmdType _tp;
	RecordID _idSkill;
	StringID _nmTarget;


};

class CBgn_CheckRtnuCmd:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckRtnuCmd);
	CBgn_CheckRtnuCmd()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
