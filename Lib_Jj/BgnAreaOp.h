#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorCustomConst.h"




class CBgp_AreaOp:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_AreaOp);

	virtual const char *GetTypeName()	{		return "区域操作";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		BOOL bCheck=FALSE;
		std::string ss;

		if (op==CheckEmpty)
		{
			s="检测区域是否为空";
			return;
		}
		
		if (dur==0.0f)
			bCheck=TRUE;
		else
		{
			if (dur<0.0f)
				ss="[永远]";
			else
				FormatString(ss,"%.2f秒",dur);
		}
		std::string s2;
		switch(op)
		{
			case PlayerEnter:
				s2="玩家是否进入区域";
				break;
			case PlayerLeave:
				s2="玩家是否离开区域";
				break;
		}
		FormatString(s,"%s%s,%s",bCheck?"检测":"监控",s2.c_str(),ss.c_str());
	}

	enum Op
	{
		PlayerEnter,
		PlayerLeave,
		CheckEmpty,
	};

    BEGIN_GOBJ_PURE_UID(CBgp_AreaOp,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(Op,op,PlayerEnter);
			GELEM_EDITVAR("操作",GVT_S,GSem(GSem_Interger,
				"玩家进入区域"		","
				"玩家离开区域"	","
				"检测区域是否为空"	"|监控时间"
				),"操作类型");
		GELEM_VAR_INIT(float,dur,0); GELEM_VERSION(2);
			GELEM_EDITVAR("监控时间",GVT_F,GSem(GSem_Float,"-1.0,10.0,0.05"),"监控多久,-1.0表示永久监控");
		GELEM_OBJ(BccArea,area);GELEM_VERSION(2);
			GELEM_EDITOBJ("区域","监控区域");
			GELEM_BVR();
	END_GOBJ();    

public: //当作protected

	Op op;
	float dur;
	DEFINE_BVR(BccArea,area);
};

class CBgn_AreaOp:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_AreaOp);
	CBgn_AreaOp()
	{
		_tStart=0;
		_dur=0;
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
	virtual void Update(BGNOutputs &outputs);

protected:

	BOOL _Check();

	AnimTick _tStart;
	AnimTick _dur;

};
