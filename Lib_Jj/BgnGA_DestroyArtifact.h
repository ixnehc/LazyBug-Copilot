#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_DestroyArtifact:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_DestroyArtifact);

	virtual const char *GetTypeName()	{		return "销毁神器";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if(tpArtifact!=LevelArtifact_None)
		{
			extern const char *LevelUtil_GetArtifactName(LevelArtifactType tp);
			FormatString(s,"销毁神器[%s]",LevelUtil_GetArtifactName(tpArtifact));
		}
		else
		{
			FormatString(s,"销毁神器[%s]",GetBVRDesc_ItemID(BVR_ARG(idArtifact),assist));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_DestroyArtifact,431,1);

		GELEM_VAR_INIT(LevelArtifactType,tpArtifact,LevelArtifact_None);
			GELEM_EDITVAR("神器类型",GVT_S,GSem(GSem_Interger,LevelArtifactConstraintStr),"是哪一种神器");
		GELEM_VAR_INIT(RecordID,idArtifact,RecordID_Invalid);GELEM_BVR();
			GELEM_EDITVAR("神器道具",GVT_U,GSem(GSem_RecordID,"items"),"Item");
    END_GOBJ();    

public: //当作protected

	LevelArtifactType tpArtifact;
	DEFINE_BVR(RecordID,idArtifact);
};


class CBgnGA_DestroyArtifact:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_DestroyArtifact);

	CBgnGA_DestroyArtifact()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

