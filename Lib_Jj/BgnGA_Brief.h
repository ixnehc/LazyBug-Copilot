#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_Brief:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_Brief);

	virtual const char *GetTypeName()	{		return "AgentBrief";	}
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
		const char *ss=bInitial?"初始Brief":"当前Brief";
		if (op==2)
		{
			FormatString(s,"针对%s,设置Brief类型(\"%s\")","缺省");
		}
		if (op==0)
		{
			extern const char *GetBVRDesc_String(std::string &ss,StringID nmRef,FillDescAssist *assist);
			FormatString(s,"针对%s,覆盖模式设置图标(%s)",ss,GetBVRDesc_String(BVR_ARG(pathIcon),assist));
		}
		if (op==3)
		{
			extern const char *GetBVRDesc_String(std::string &ss,StringID nmRef,FillDescAssist *assist);
			FormatString(s,"针对%s,不覆盖模式设置图标(%s)",ss,GetBVRDesc_String(BVR_ARG(pathIcon),assist));
		}
		if (op==1)
		{
			if (tip!=StringID_Invalid)
				FormatString(s,"针对%s,设置提示(\"%s\")",ss,StrLib_GetStr(tip));
		}
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_Brief,483,1);

		GELEM_VAR_INIT(BOOL ,bInitial,1);
			GELEM_EDITVAR("是否为初始Brief",GVT_S,GSem(GSem_Interger,"初始Brief:1,当前Brief:0"),"是否为初始Brief");
		GELEM_VAR_INIT(int,op,0);
			GELEM_EDITVAR("操作",GVT_S,GSem(GSem_Interger,
				"设置类型:2"		"|Icon路径&Icon锚点(归一化坐标)&Tip,"
				"设置Icon(覆盖已有路径):0"		"|Tip&Brief类型,"
				"设置Icon(不覆盖已有路径):3"		"|Tip&Brief类型,"
				"设置Tip:1"	"|Icon路径&Icon锚点(归一化坐标)&Brief类型"
				),"操作");
		GELEM_VAR_INIT(LevelAgentBrief::Type,tp,LevelAgentBrief::Default);
			GELEM_EDITVAR("Brief类型",GVT_U,GSem(GSem_Interger,"缺省:1"),"Brief类型");
		GELEM_STRING_INIT(pathIcon,AgentBriefIcon_TexPath",0,0,64,64");
			GELEM_EDITVAR("Icon路径",GVT_String,GSem_TexturePartPath,"Icon路径");
			GELEM_BVR();
		GELEM_VAR_INIT(i_math::pos2df,ptIconAnchor,i_math::pos2df(0.5f,0.8f));
			GELEM_EDITVAR("Icon锚点(归一化坐标)",GVT_Fx2,GSem_Point,"Icon的基点在什么位置");
		GELEM_VAR_INIT( StringID,tip,StringID_Invalid);	
			GELEM_EDITVAR( "Tip", GVT_U, GSem(GSem_StringID,"AgentTip文本"), "Tip" );
	END_GOBJ();    

public: //当作protected
	BOOL bInitial;
	int op;
	LevelAgentBrief::Type tp;
	DEFINE_BVR(std::string,pathIcon);
	i_math::pos2df ptIconAnchor;
	StringID tip;

};


class CBgnGA_Brief:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_Brief);

	CBgnGA_Brief()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
