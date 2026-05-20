#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "Skill_GeneralAdvS.h"



class CBgp_SkillOp:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_SkillOp);

	virtual const char *GetTypeName()	{		return "设置SkillOp";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		switch(_op)
		{
			case SkillParam_GeneralAdvS::OpEntry::Op_Landing:
				s="着陆";
				break;
			case SkillParam_GeneralAdvS::OpEntry::Op_TakeOff:
				s="起飞";
				break;
			case SkillParam_GeneralAdvS::OpEntry::Op_OverrideWeaks:
				s="重载弱点";
				break;
			case SkillParam_GeneralAdvS::OpEntry::Op_AddWeaks:
				s="添加弱点";
				break;
			case SkillParam_GeneralAdvS::OpEntry::Op_RemoveWeaks:
				s="去除弱点";
				break;
			case SkillParam_GeneralAdvS::OpEntry::Op_CleanOverrideWeaks:
				s="取消弱点修改";
				break;
			case SkillParam_GeneralAdvS::OpEntry::Op_SetFacingMode_None:
				s="设置为不改变朝向";
				break;
			case SkillParam_GeneralAdvS::OpEntry::Op_SetPathFacingMode_None:
				s="设置为不改变路径朝向";
				break;
				//XXXXX: More SkillParam_GeneralAdvS::OpEntry::Op
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_SkillOp,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(SkillParam_GeneralAdvS::OpEntry::Op,_op,SkillParam_GeneralAdvS::OpEntry::Op_None);
			GELEM_EDITVAR("Op",GVT_U,GSem(GSem_Interger,
				"n/a:0" "|弱点,"
// 				"允许取消:1"  "|弱点,"
				"重载弱点:2"   "|,"
				"添加弱点:12"   "|,"
				"去除弱点:13"   "|,"
				"取消弱点修改:3"   "|弱点,"
				"着陆:4"   "|弱点,"
				"起飞:5"   "|弱点,"
				"设置为不改变朝向:6"   "|弱点,"
				"设置为不改变路径朝向:7"   "|弱点"
				),"操作");
			//XXXXX: More SkillParam_GeneralAdvS::OpEntry::Op
		GELEM_OBJ(WeaksEx,weaks);
			GELEM_EDITOBJ("弱点","弱点");


	END_GOBJ();    

public: //当作protected

	SkillParam_GeneralAdvS::OpEntry::Op _op;
	WeaksEx weaks;

};


class CBgn_SkillOp:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_SkillOp);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
