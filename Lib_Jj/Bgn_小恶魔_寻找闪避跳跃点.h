#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgp_小恶魔_寻找闪避跳跃点:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_小恶魔_寻找闪避跳跃点);

	virtual const char *GetTypeName()	{		return "小恶魔_寻找闪避跳跃点";	}
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
	}

    BEGIN_GOBJ_PURE_UID(CBgp_小恶魔_寻找闪避跳跃点,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(float,radius,3.0f);
			GELEM_EDITVAR("搜寻半径",GVT_F,GSem(GSem_Float,"0,100,0.1"),"搜寻半径");

		GELEM_VAR_INIT(float,radiusMin,1.0f);
			GELEM_EDITVAR("最小半径",GVT_F,GSem(GSem_Float,"0,100,0.1"),"最小半径");

		GELEM_VARVECTOR_INIT(LevelDetectTargetFlag,flagsDetect,LevelDetectTargetFlag_Default);
			GELEM_EDITVAR("侦测对象",GVT_U,GSem(GSem_Flags,LevelDetectTargetFlag_GetSemStr()),"侦测什么类型的单位");
			GELEM_BVR();
		GELEM_VARVECTOR_INIT(LevelObjRequire,requires,LevelObjRequire_Attackable);
			GELEM_EDITVAR("特定需求",GVT_S,GSem(GSem_Interger,LevelObjRequire_SemConstraint),"有哪些特定的需求");
		GELEM_VAR_INIT(float,rangeDetect,2.0f);
			GELEM_EDITVAR("侦测距离",GVT_F,GSem(GSem_Float,"0,100,0.1"),"侦测多远距离以内的单位");

		GELEM_BEHAVIORMEM_OBJID(varHammer,"锤子对象变量","锤子对象变量");
		GELEM_BEHAVIORMEM_POS(varPos,"[out]位置变量","找到的位置存放在哪里")

    END_GOBJ();    

public: //当作protected
	float radius;
	float radiusMin;

	DEFINE_BVR(std::vector<LevelDetectTargetFlag>,flagsDetect);
	std::vector<LevelObjRequire> requires;
	float rangeDetect;
	
	StringID varHammer;
	StringID varPos;

};


class CBgn_小恶魔_寻找闪避跳跃点:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_小恶魔_寻找闪避跳跃点);

	CBgn_小恶魔_寻找闪避跳跃点()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

