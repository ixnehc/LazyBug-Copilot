#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"

#include "records/recordsdefine.h"

#include "gds/GObjUID.h"


class CBgpGA_Env:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_Env);

	virtual const char *GetTypeName()	{		return "战斗环境";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Env;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	enum Op
	{
		Op_None,
		Op_Create,
		Op_Destroy,
	};

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="n/a";
		if (_op==Op_Create)
			FormatString(s,"创建战斗环境(%s)",GetBVRDesc_EoID(BVR_ARG(_idEo),assist));
		if (_op==Op_Destroy)
			s="清除战斗环境";
	}

    BEGIN_GOBJ_PURE_UID(CBgpGA_Env,1);

		GELEM_VAR_INIT(int,_op,(int)Op_Create);
			GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,
				"创建战斗环境:1"		","
				"清除战斗环境:2"	"|战斗环境Eo|战斗环境Eo [引用]|战斗环境区域|战斗环境区域 [引用]"
				),"操作模式");

		GELEM_VAR_INIT(RecordID,_idEo,RecordID_Invalid);
			GELEM_EDITVAR("战斗环境Eo",GVT_U,GSem(GSem_RecordID,"eos"),"战斗环境Eo");
			GELEM_BVR();

		GELEM_OBJ(BccArea,_area);
			GELEM_EDITOBJ("战斗环境区域","战斗环境区域");
			GELEM_BVR();

    END_GOBJ();    

public: //当作protected

	int _op;
	DEFINE_BVR(RecordID,_idEo);
	DEFINE_BVR(BccArea,_area);
};


class CBgnGA_Env:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_Env);

	CBgnGA_Env()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgpGA_Env_SetFenceDestroyed:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_Env_SetFenceDestroyed);

	virtual const char *GetTypeName()	{		return "战斗环境_设置围墙被毁";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Env;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		s="设置战斗环境的围墙被毁";
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_Env_SetFenceDestroyed,418,1);
		GELEM_BGP_BASE();


    END_GOBJ();    

public: //当作protected

};


class CBgnGA_Env_SetFenceDestroyed:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_Env_SetFenceDestroyed);

	CBgnGA_Env_SetFenceDestroyed()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

class CBgpGA_Env_Check:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgpGA_Env_Check);

	virtual const char *GetTypeName()	{		return "战斗环境_检测";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Env;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Level;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (op==0)
			s="检测战斗环境的围墙是否被毁";
		if (op==1)
			s="检测战斗环境是否存在";
	}

    BEGIN_GOBJ_PURE_UID2(CBgpGA_Env_Check,419,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(int,op,0);
		GELEM_EDITVAR("模式",GVT_S,GSem(GSem_Interger,
			"检测围墙是否被毁:0,"
			"检测战斗环境是否存在:1"
			),"操作模式");

    END_GOBJ();    

public: //当作protected
	int op;

};


class CBgnGA_Env_Check:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgnGA_Env_Check);

	CBgnGA_Env_Check()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};
