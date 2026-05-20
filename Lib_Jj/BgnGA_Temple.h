#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"


class CBgpGA_ModTemple:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_ModTemple);

	virtual const char *GetTypeName()	{		return "修改圣殿修复等级";	}
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
		if (tp==LevelTemple_None)
			s="n/a";
		else
			FormatString(s,"修复TalkPlayer的%s的某个祭坛",NameFromTempleType(tp));
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_ModTemple,465,1);
		GELEM_VAR_INIT(LevelTempleType,tp,LevelTemple_None);
			GELEM_EDITVAR("圣殿类型",GVT_S,GSem(GSem_Interger,LevelTempleType_SemConstraint),"圣殿类型");
			GELEM_BVR();
		GELEM_VAR_INIT(int,iAltar,0);
			GELEM_EDITVAR("祭坛序号",GVT_S,GSem_Interger,"祭坛序号");
			GELEM_BVR();
	END_GOBJ();    

public: //当作protected
	DEFINE_BVR(LevelTempleType,tp);
	DEFINE_BVR(int,iAltar);

};


class CBgnGA_ModTemple:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_ModTemple);

	CBgnGA_ModTemple()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};




class CBgpGA_GetTemple:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_GetTemple);

	virtual const char *GetTypeName()	{		return "得到圣殿修复等级";	}
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
		if ((tp==LevelTemple_None)||(var==StringID_Invalid))
			s="n/a";
		else
			FormatString(s,"得到TalkPlayer的%s的有几个祭坛修复了,结果保存在变量[%s]中",NameFromTempleType(tp),StrLib_GetStr(var));
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_GetTemple,472,1);
		GELEM_VAR_INIT(LevelTempleType,tp,LevelTemple_None);
			GELEM_EDITVAR("圣殿类型",GVT_S,GSem(GSem_Interger,LevelTempleType_SemConstraint),"圣殿类型");
			GELEM_BVR();
		GELEM_BEHAVIORMEM_NUMBER(var,"结果变量","存入哪个变量中")
	END_GOBJ();    

public: //当作protected
	DEFINE_BVR(LevelTempleType,tp);
	StringID var;


};


class CBgnGA_GetTemple:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_GetTemple);

	CBgnGA_GetTemple()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};




class CBgpGA_TempleSwitcher:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_TempleSwitcher);

	virtual const char *GetTypeName()	{		return "圣殿类型Switcher";	}
	virtual DWORD GetStubCount()
	{
		return 8;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB();
			STUB_IN(0,"开始");
			STUB_OUT(1,"无效");
			STUB_OUT(2,"光之圣殿");
			STUB_OUT(3,"月之圣殿");
			STUB_OUT(4,"火之圣殿");
			STUB_OUT(5,"星之圣殿");
			STUB_OUT(6,"沙之圣殿");
			STUB_OUT(7,"工匠圣殿");
		END_STUB();
		//XXXXX:More LevelTempleType
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_GA;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="";
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_TempleSwitcher,473,1);
		GELEM_VAR_INIT(LevelTempleType,tp,LevelTemple_None);
			GELEM_EDITVAR("圣殿类型",GVT_S,GSem(GSem_Interger,LevelTempleType_SemConstraint),"圣殿类型");
			GELEM_BVR();
	END_GOBJ();    

public: //当作protected
	DEFINE_BVR(LevelTempleType,tp);

};


class CBgnGA_TempleSwitcher:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_TempleSwitcher);

	CBgnGA_TempleSwitcher()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

