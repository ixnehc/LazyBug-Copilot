#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_CheckObstacle:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckObstacle);

	enum TargetType
	{
		Target_Custom,
		Target_TalkPlayer,

		ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "检测障碍";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"有障碍");
			STUB_OUT(2,"没有障碍");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (tpTarget==Target_Custom)
		{
			if (nmVar==StringID_Invalid)
				s="n/a";
			else
				FormatString(s,"判断与变量[%s]中的单位之间是否有Static障碍",assist->GetStr(nmVar));
		}
		else
			FormatString(s,"判断与锁定的Player的之间是否有Static障碍");
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckObstacle,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(TargetType,tpTarget,Target_Custom);//ImgCombo_Normal
			GELEM_EDITVAR("目标类型",GVT_S,GSem(GSem_Interger,"自定义,Talk的Player(仅用于GA)"),"目标类型");
		GELEM_BEHAVIORMEM_OBJID(nmVar,"变量名称","检查哪个变量中的对象与自己的距离")

	END_GOBJ();    

public: //当作protected

	TargetType tpTarget;
	StringID nmVar;
};

class CBgn_CheckObstacle:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckObstacle);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
