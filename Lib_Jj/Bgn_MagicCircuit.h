#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgp_MagicCircuitOp:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_MagicCircuitOp);

	virtual const char *GetTypeName()	{		return "MagicCircuitOp";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Misc;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (mode==0)
			s="Activate Relay";
		if (mode==1)
			s="Deactivate Relay";
		if (mode==2)
		{
			s="n/a";
			if (nmVarObj!=StringID_Invalid)
				FormatString(s,"Spawn Crystal,结果保存在变量[%s]中",assist->GetStr(nmVarObj));
		}
		if (mode==3)
		{
			s="n/a";
			if (nmVarObj!=StringID_Invalid)
				FormatString(s,"Spawn RelayBird,结果保存在变量[%s]中",assist->GetStr(nmVarObj));
		}
		if (mode==4)
		{
			s="n/a";
			if (nmVarPos!=StringID_Invalid)
				FormatString(s,"Get RelayBird TargetPos,结果保存在变量[%s]中",assist->GetStr(nmVarPos));
		}
		if (mode==5)
			s="Check RelayBird At Home";
		if (mode==15)
			s="Check RelayBird At Rest";
		if (mode==6)
			s="Activate Focus";
		if (mode==7)
			s="Check Focus";
		if (mode==8)
			s="Commit Focus";
		if (mode==9)
			s="Check TailOrbs Can Reach";
		if (mode==10)
			s="Start TailOrbs Reach";
		if (mode==11)
			s="Check TailOrbs Reached";
		if (mode==12)
		{
			s="n/a";
			if (nmVarObj!=StringID_Invalid)
				FormatString(s,"Get TailOrbs Home,结果保存在变量[%s]中",assist->GetStr(nmVarObj));
		}
		if (mode==13)
			s="Can Spawn Crystal";
		if (mode==14)
			s="Can Activate Focus";
		if (mode == 16)
			s = "Spawn Rail Guards";
		if (mode == 17)
			s = "Despawn Rail Guards";
	}

    BEGIN_GOBJ_PURE_UID2(CBgp_MagicCircuitOp,495,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(int,mode,1); GELEM_UID(1);
			GELEM_EDITVAR("模式",GVT_U,GSem(GSem_Interger,
				"Activate Relay:0"  "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Deactivate Relay:1" "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Spawn Crystal:2" "|结果保存变量(Pos),"
				"Spawn RelayBird:3" "|结果保存变量(Pos),"
				"Get RelayBird TargetPos:4" "|结果保存变量(ObjID),"
				"Check RelayBird At Home:5"  "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Check RelayBird At Rest:15"  "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Activate Focus:6"  "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Check Focus:7"  "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Commit Focus:8"  "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Check TailOrbs Can Reach:9" "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Start TailOrbs Reach:10" "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Check TailOrbs Reached:11" "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Get TailOrbs Home:12"  "|结果保存变量(Pos),"
				"Can Spawn Crystal:13" "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Can Activate Focus:14" "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Spawn RailGuards:16" "|结果保存变量(ObjID)&结果保存变量(Pos),"
				"Despawn RailGuards:17" "|结果保存变量(ObjID)&结果保存变量(Pos)"
			), "工作模式");

		GELEM_BEHAVIORMEM_OBJID(nmVarObj,"结果保存变量(ObjID)","结果保存到哪个变量中");GELEM_UID(2);
		GELEM_BEHAVIORMEM_POS(nmVarPos,"结果保存变量(Pos)","结果保存到哪个变量中");GELEM_UID(3);
    END_GOBJ();    

public: //当作protected
	int mode;
	StringID nmVarObj;
	StringID nmVarPos;
	

};


class CBgn_MagicCircuitOp:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_MagicCircuitOp);

	CBgn_MagicCircuitOp()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs) override;

protected:
};
