#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

struct AttrNodeFloat;
class CBgpTroop_CheckDist:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpTroop_CheckDist);

	enum Op
	{
		LessThan,
		GreatThan,

		Op_ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "检测Troop距离";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Troop;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		static const char *ss;
		if (_op==LessThan)
			ss="小于";
		else
			ss="大于";
		FormatString(s,"检测Troop(%s)中的单位离路径的距离是否%s%s",
			GetBVRDesc_StringID(BVR_ARG(_troop),assist),
			ss,
			GetBVRDesc_Float(BVR_ARG(_distRef),assist));
	}

    BEGIN_GOBJ_PURE_UID(CBgpTroop_CheckDist,1);
		GELEM_VAR_INIT(Op,_op,GreatThan);
			GELEM_EDITVAR("检测方式",GVT_S,GSem(GSem_Interger,"距离小于,距离大于"),"如何检测");
		GELEM_OBJ(BccRoute,_route);
			GELEM_EDITOBJ("路径","路径");
			GELEM_BVR();
		GELEM_BEHAVIOR_TROOPREF(_troop,"Troop名称","检测哪个Troop");
			GELEM_BVR();
		GELEM_VAR_INIT(float,_distRef,10.0f);
			GELEM_EDITVAR("参考距离",GVT_F,GSem(GSem_Float,"0,200,0.1"),"参考距离");
			GELEM_BVR();
    END_GOBJ();    

public: //当作protected

	Op _op;
	DEFINE_BVR(StringID,_troop);
	DEFINE_BVR(BccRoute,_route);
	DEFINE_BVR(float,_distRef);

};


class CBgnTroop_CheckDist:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnTroop_CheckDist);

	CBgnTroop_CheckDist()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:



};

