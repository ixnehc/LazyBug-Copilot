#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "behaviorgraph/BehaviorParam.h"
#include "behaviorgraph/BehaviorCustomConst.h"

#include "records/recordsdefine.h"

#include "BgnGA_RollAwards.h"


class CBgpGA_RollPoemAwards:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_RollPoemAwards);

	virtual const char *GetTypeName()	{		return "随机Poem奖励";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"不需要选择");
			STUB_OUT(2,"需要选择");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";

		if (tpArtifact!=LevelArtifact_None)
		{
			extern const char *LevelUtil_GetArtifactName(LevelArtifactType tp);

			FormatString(s,"随机产生[%s]名为[%s]的奖励,保存在变量[%s]中",
				LevelUtil_GetArtifactName(tpArtifact),BVRDESC_StringID(nm),BVRDESC_StringID(awards));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_RollPoemAwards,430,1);

		GELEM_VAR_INIT(LevelArtifactType,tpArtifact,LevelArtifact_None);GELEM_BVR();
			GELEM_EDITVAR("神器类型",GVT_S,GSem(GSem_Interger,LevelArtifactConstraintStr_Poems),"是哪一种神器");

		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);GELEM_BVR();
			GELEM_EDITVAR( "奖励名称", GVT_U, GSem(GSem_StringID,"Poem奖励名称"), "奖励名称" );

		GELEM_VAR_INIT( StringID,awards,StringID_Invalid);GELEM_BVR();
			GELEM_EDITVAR( "奖励保存变量", GVT_U, GSem(GSem_StringID,"行为图内存变量名称:Obj"), "奖励保存在哪个变量里");

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(LevelArtifactType,tpArtifact);

	DEFINE_BVR(StringID,nm);

	DEFINE_BVR(StringID,awards);


};


class CBgnGA_RollPoemAwards:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_RollPoemAwards);

	CBgnGA_RollPoemAwards()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

