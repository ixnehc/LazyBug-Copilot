#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"
#include "strlib/strlib.h"

#include "math/range.h"



class CBgp_CheckSkillTargetRange:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckSkillTargetRange);

	enum Mode
	{
		Mode_None=0,
		Mode_4Dir=1,
		Mode_FacingRange=2,
		Mode_DistRange=3,

		Mode_ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "检测Skill目标范围";	}
	virtual DWORD GetStubCount()
	{
		switch(mode)
		{
			case Mode_4Dir:
				return 4+1;
			case Mode_FacingRange:
			case Mode_DistRange:
				return 2+1;
		}
		return 1;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		if (idx==0)
			return PadStub("开始",PadStub_In,1);

		if (mode==Mode_4Dir)
		{
			switch(idx)
			{
				case 1: return PadStub("前",PadStub_Out,1);
				case 2: return PadStub("后",PadStub_Out,1);
				case 3: return PadStub("左",PadStub_Out,1);
				case 4: return PadStub("右",PadStub_Out,1);
			}
		}
		if ((mode==Mode_FacingRange)||(mode==Mode_DistRange))
		{
			switch(idx)
			{
				case 1: return PadStub("是",PadStub_Out,1);
				case 2: return PadStub("否",PadStub_Out,1);
			}
		}

		return PadStub("n/a",PadStub_Out,1);
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Skill;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (mode==Mode_4Dir)
			s="前后左右模式";
		if (mode==Mode_FacingRange)
			FormatString(s,"检测技能目标是否在朝向范围[ %.2f度 ~ %.2f度 ]内",_rngFace.low,_rngFace.hi);
		if (mode==Mode_DistRange)
			FormatString(s,"检测技能目标是否在距离范围[ %.2f米 ~ %.2f米 ]内",_rngDist.low,_rngDist.hi);
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckSkillTargetRange,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(Mode ,mode,Mode_4Dir);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,
				"前后左右模式:1"	 "|朝向范围&距离范围,"
				"朝向范围模式:2"	 "|距离范围,"
				"距离范围模式:3"	 "|朝向范围"
				),"工作模式");
		GELEM_VAR_INIT( i_math::rangef,_rngFace,i_math::rangef(-180.0f,180.0f));
			GELEM_EDITVAR( "朝向范围", GVT_Fx2,GSem_Range,"朝向范围");
		GELEM_VAR_INIT( i_math::rangef,_rngDist,i_math::rangef(0.0f,5.0f));
			GELEM_EDITVAR( "距离范围", GVT_Fx2,GSem_Range,"距离范围");

	END_GOBJ();    

public: //当作protected

	Mode mode;
	i_math::rangef _rngFace;
	i_math::rangef _rngDist;


};


class CBgn_CheckSkillTargetRange:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckSkillTargetRange);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
