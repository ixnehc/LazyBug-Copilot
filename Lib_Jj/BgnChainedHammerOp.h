#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "Skill_GeneralAdvS.h"



class CBgp_ChainedHammerOp:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_ChainedHammerOp);

	enum Op
	{
		Op_None,
		Op_Withdraw,
		Op_CheckStuck,
		Op_CheckCanWithdraw,
		Op_CheckWithdrawn,
		Op_CheckInHand,
		Op_Pull,
		Op_CheckDetached,
		Op_Grab,
		Op_Break,
	};

	virtual const char *GetTypeName()	{		return "锁链锤Op";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"Ok");
			STUB_OUT(2,"NotOk");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		switch(_op)
		{
			case Op_Withdraw:
				s="收回";
				break;
			case Op_CheckStuck:
				s="检测是否嵌在地里";
				break;
			case Op_CheckCanWithdraw:
				s="检测是否可以收回";
				break;
			case Op_CheckWithdrawn:
				s="检测是否已收回";
				break;
			case Op_CheckInHand:
				s="检测是否在手里";
				break;
			case Op_Pull:
				s="拔出";
				break;
			case Op_CheckDetached:
				s="检测锁链是否已断";
				break;
			case Op_Grab:
				s="抓起";
				break;
			case Op_Break:
				s="打断";
				break;
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_ChainedHammerOp,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(Op,_op,Op_Withdraw);
			GELEM_EDITVAR("Op",GVT_U,GSem(GSem_Interger,
				"检测是否在手里:5"   ","
				"收回:1"   ","
				"检测是否嵌在地里:2"   ","
				"检测是否可以收回:3"   ","
				"检测是否已收回:4"   ","
				"拔出:6"   ","
				"检测锁链是否已断:7"   ","
				"抓起:8"   ","
				"打断:9"   ""
				),"操作");

		GELEM_BEHAVIORMEM_OBJID(_nmVar,"锁链锤对象变量","锁链锤对象保存在哪个变量中")

	END_GOBJ();    

public: //当作protected

	Op _op;
	StringID _nmVar;

};

class EoChainedHammer;
class CBgn_ChainedHammerOp:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_ChainedHammerOp);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

	BOOL _CheckCanWithdraw(EoChainedHammer *eoHammer);

};
