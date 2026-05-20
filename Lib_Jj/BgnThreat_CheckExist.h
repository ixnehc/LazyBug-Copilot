#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgpThreat_CheckExist:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpThreat_CheckExist);

	virtual const char *GetTypeName()	{		return "检测Threat是否存在";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Threat;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"检测Threat是否存在");
	}

    BEGIN_GOBJ_PURE_UID(CBgpThreat_CheckExist,1);
		GELEM_BGP_BASE();
    END_GOBJ();    

public: //当作protected

};


struct LevelRecordSkill;
class CBgnThreat_CheckExist:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnThreat_CheckExist);

	CBgnThreat_CheckExist()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:
};

