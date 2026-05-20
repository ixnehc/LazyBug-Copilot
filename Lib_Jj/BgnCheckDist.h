#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"


class CBgp_CheckDist:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckDist);

	enum SourceType
	{
		Source_Me,
		Source_Custom,

		Source_ForceDword=0xffffffff,
	};

	enum TargetType
	{
		Target_Custom,
		Target_TalkPlayer,
		Target_FirstPlayer,
		Target_Ground,
		Target_LockPlayer,

		Target_ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "检测距离";	}
	virtual DWORD GetStubCount()
	{
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"范围内");
			STUB_OUT(2,"范围外");
		END_STUB();
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Condition;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (tpSource==Source_Me)
		{
			if (tpTarget==Target_Custom)
			{
				if (nmVar==StringID_Invalid)
					s="n/a";
				else
					FormatString(s,"判断与变量[%s]中的单位的距离是否在%.2f米到%.2f米范围内",assist->GetStr(nmVar),radiusMin,radiusMax);
			}
			if (tpTarget==Target_TalkPlayer)
				FormatString(s,"判断与TalkPlayer的距离是否在%.2f米到%.2f米范围内",radiusMin,radiusMax);
			if (tpTarget==Target_LockPlayer)
				FormatString(s,"判断与LockPlayer的距离是否在%.2f米到%.2f米范围内",radiusMin,radiusMax);
			if (tpTarget==Target_FirstPlayer)
				FormatString(s,"判断与当前Player的距离是否在%.2f米到%.2f米范围内",radiusMin,radiusMax);
			if (tpTarget==Target_Ground)
				FormatString(s,"判断飞行高度是否在%.2f米到%.2f米范围内",radiusMin,radiusMax);
		}
		if (tpSource==Source_Custom)
		{
			if (tpTarget==Target_Custom)
			{
				if ((nmVar==StringID_Invalid)||(nmVarSrc==StringID_Invalid))
					s="n/a";
				else
					FormatString(s,"判断变量[%s]与变量[%s]中的单位的距离是否在%.2f米到%.2f米范围内",assist->GetStr(nmVarSrc),assist->GetStr(nmVar),radiusMin,radiusMax);
			}
			if (tpTarget==Target_TalkPlayer)
				FormatString(s,"判断变量[%s]与TalkPlayer的距离是否在%.2f米到%.2f米范围内",assist->GetStr(nmVarSrc),radiusMin,radiusMax);
			if (tpTarget==Target_LockPlayer)
				FormatString(s,"判断变量[%s]与LockPlayer的距离是否在%.2f米到%.2f米范围内",assist->GetStr(nmVarSrc),radiusMin,radiusMax);
			if (tpTarget==Target_FirstPlayer)
				FormatString(s,"判断变量[%s]与当前的Player的距离是否在%.2f米到%.2f米范围内",assist->GetStr(nmVarSrc),radiusMin,radiusMax);
			if (tpTarget==Target_Ground)
				FormatString(s,"判断变量[%s]的飞行高度是否在%.2f米到%.2f米范围内",assist->GetStr(nmVarSrc),radiusMin,radiusMax);
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckDist,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(SourceType,tpSource,Source_Me);//ImgCombo_Normal
			GELEM_EDITVAR("源类型",GVT_S,GSem(GSem_Interger,
				"自己:0"		"|源对象,"
				"指定对象:1"	""
				),"源类型");
		GELEM_BEHAVIORMEM_OBJID(nmVarSrc,"源对象","源对象")

		GELEM_VAR_INIT(TargetType,tpTarget,Target_Custom);//ImgCombo_Normal
			GELEM_EDITVAR("目标类型",GVT_S,GSem(GSem_Interger,
				"自定义:0"	"," 
				"Talk的Player(仅用于GA)"	"|目标对象,"
				"当前的Player"		"|目标对象,"
				"地表(用于飞行高度检测)"	"|目标对象,"
				"LockPlayer"	"|目标对象,"
				),"目标类型");
		GELEM_BEHAVIORMEM_OBJID(nmVar,"目标对象","目标对象")
		GELEM_VAR_INIT(float,radiusMin,0.0f);
			GELEM_EDITVAR("最小范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"最小范围");
		GELEM_VAR_INIT(float,radiusMax,5.0f);
			GELEM_EDITVAR("最大范围",GVT_F,GSem(GSem_Float,"0,100,0.1"),"最大范围");

	END_GOBJ();    

public: //当作protected

	SourceType tpSource;
	StringID nmVarSrc;

	TargetType tpTarget;
	StringID nmVar;
	float radiusMin;
	float radiusMax;
};

class CBgn_CheckDist:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckDist);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
