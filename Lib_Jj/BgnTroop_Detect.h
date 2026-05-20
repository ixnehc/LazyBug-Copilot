#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "math/range.h"

struct AttrNodeFloat;
class CBgpTroop_Detect:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpTroop_Detect);

	virtual const char *GetTypeName()	{		return "Troop侦测";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"侦测到");
			STUB_OUT(2,"未侦测到");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		FormatString(s,"在Troop(%s)周围%s米范围内侦测%s",GetBVRDesc_StringID(BVR_ARG(_troop),assist),
			GetBVRDesc_Float(BVR_ARG(_range),assist),LevelDetectTargetFlags_GetName(BVR_ARG(_flags)));
	}

    BEGIN_GOBJ_PURE_UID(CBgpTroop_Detect,1);
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","侦测哪个Troop");
			GELEM_BVR();

		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,_flags,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位");
			GELEM_BVR();
		GELEM_VARVECTOR_INIT(LevelObjRequire,_requires,LevelObjRequire_Attackable);
			GELEM_EDITVAR("特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"有哪些特定的需求");
		GELEM_VAR_INIT(float,_range,15.0f);
			GELEM_EDITVAR("侦测范围",GVT_F,GSem(GSem_Float,"0.1,100.0,0.1"),"侦测范围");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	DEFINE_BVR(StringID,_troop);

	DEFINE_BVR(std::vector<LevelDetectTargetFlag>,_flags);
	std::vector<LevelObjRequire> _requires;
	DEFINE_BVR(float,_range);
};


class CBgnTroop_Detect:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_Detect);

	CBgnTroop_Detect()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

