#pragma once

#include "BehaviorGraphPads.h"
#include "Behavior.h"
#include "BehaviorValue.h"

#include "BehaviorCustomConst.h"


class CBgp_Graph:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Graph);

	virtual const char *GetTypeName()	{		return "行为图";	};
	virtual DWORD GetStubCount()		{				return 1;	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_OUT(0,"启动");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)	
	{
		if (_nm==StringID_Invalid)
			s="n/a";
		else
		{
			if (_family==BgpFamily_Level)
				FormatString(s,"%s:%s","Server端行为图",StrLib_GetStr(_nm));
			if (_family==BgpFamily_Game)
				FormatString(s,"%s:%s","Client端行为图",StrLib_GetStr(_nm));
			if (_family==BgpFamily_MagicBoard)
				FormatString(s,"%s:%s","MagicBoard行为图",StrLib_GetStr(_nm));
		}
	}


	BEGIN_GOBJ_PURE_UID(CBgp_Graph,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "行为图名称", GVT_U, GSem(GSem_StringID,"行为图名称"), "当前的行为图的名称" );
		GELEM_VAR_INIT(BgpFamily,_family,BgpFamily_Level);
			GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,"Server端行为图:1,Client端行为图:2,魔法棋盘行为图:3"),"什么类型的行为图");
	END_GOBJ();    

public: //当作protected
	StringID _nm;
	BgpFamily _family;
};


class CBgn_Graph:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Graph);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};




class CBgp_Counter:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Counter);

	virtual const char *GetTypeName()	{		return "计数器";	};
	virtual DWORD GetStubCount()		{				return 0;	}
	virtual PadStub GetStub(DWORD idx)	{		return PadStub();	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)	
	{
		if (_nm==StringID_Invalid)
			s="n/a";
		else
		{
			if (_vary>0)
				FormatString(s,"%s,初始值:%d(+/-%d)",StrLib_GetStr(_nm),_init,_vary);
			else
				FormatString(s,"%s,初始值:%d",StrLib_GetStr(_nm),_init);
		}
	}


	BEGIN_GOBJ_PURE_UID(CBgp_Counter,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "计数器名称", GVT_U, GSem(GSem_StringID,"行为图计数器名称"), "计数器的名称" );
		GELEM_VAR_INIT(int,_init,0);
			GELEM_EDITVAR("初始值",GVT_S,GSem_Interger,"初始值");
		GELEM_VAR_INIT(int,_vary,0);
			GELEM_EDITVAR("初始值浮动值",GVT_S,GSem_Interger,"初始值浮动值");
	END_GOBJ();    

public: //当作protected
	StringID _nm;
	int _init;
	int _vary;
};


class CBgn_Counter:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Counter);

protected:

};


class CBgp_Timer:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Timer);

	virtual const char *GetTypeName()	{		return "计时器(obsolete)";	};
	virtual DWORD GetStubCount()		{				return 0;	}
	virtual PadStub GetStub(DWORD idx)	{		return PadStub();	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)	
	{
		if (_nm==StringID_Invalid)
			s="n/a";
		else
		{
			if (_vary>0)
				FormatString(s,"%s,初始值:%.2f(+/-%.2f)",StrLib_GetStr(_nm),ANIMTICK_TO_SECOND(_init),ANIMTICK_TO_SECOND(_vary));
			else
				FormatString(s,"%s,初始值:%.2f",StrLib_GetStr(_nm),ANIMTICK_TO_SECOND(_init));
		}
	}


	BEGIN_GOBJ_PURE_UID(CBgp_Timer,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "计时器名称", GVT_U, GSem(GSem_StringID,"行为图计时器名称"), "计时器的名称" );

		GELEM_VAR_INIT(AnimTick,_init,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("初始值",GVT_U,GSem(GSem_AnimTick,"0,1000,0.01"),"初始值,单位为秒");
		GELEM_VAR_INIT(AnimTick,_vary,ANIMTICK_FROM_SECOND(0.0f));
			GELEM_EDITVAR("浮动值",GVT_U,GSem(GSem_AnimTick,"0,1000,0.01"),"浮动值,单位为秒");
	END_GOBJ();    

public: //当作protected
	StringID _nm;
	AnimTick _init;
	AnimTick _vary;
};


class CBgn_Timer:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Timer);

protected:

};




class CBgp_Register:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Register);

	virtual const char *GetTypeName()	{		return "寄存器";	};
	virtual DWORD GetStubCount()		{				return 0;	}
	virtual PadStub GetStub(DWORD idx)	{		return PadStub();	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)	
	{
		if (_nm==StringID_Invalid)
			s="n/a";
		else
		{
			if (_vary>0)
				FormatString(s,"%s,初始值:%d(+/-%d)",StrLib_GetStr(_nm),_init,_vary);
			else
				FormatString(s,"%s,初始值:%d",StrLib_GetStr(_nm),_init);
		}
	}

	BEGIN_GOBJ_PURE_UID(CBgp_Register,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT( StringID,_nm,StringID_Invalid);	
			GELEM_EDITVAR( "变量名称", GVT_U, GSem(GSem_StringID,"行为图内存变量名称"), "寄存器的名称" );
		GELEM_VAR_INIT(BOOL,_bPersist,0);
			GELEM_EDITVAR("是否需要保存",GVT_S,GSem_Boolean,"该寄存器是否需要保存");
		GELEM_VAR_INIT(int,_init,0);
			GELEM_EDITVAR("初始值",GVT_S,GSem_Interger,"初始值");
		GELEM_VAR_INIT(int,_vary,0);
			GELEM_EDITVAR("初始值浮动值",GVT_S,GSem_Interger,"初始值浮动值");
	END_GOBJ();    

public: //当作protected
	StringID _nm;
	BOOL _bPersist;
	int _init;
	int _vary;
};


class CBgn_Register:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Register);

protected:

};


class CBgp_Consts:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Consts);

	virtual const char *GetTypeName()	{		return "常量声明";	};
	virtual DWORD GetStubCount()		{				return 0;	}
	virtual PadStub GetStub(DWORD idx)	{		return PadStub();	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist);

	BEGIN_GOBJ_PURE_UID(CBgp_Consts,1);
		GELEM_BGP_BASE();

		GELEM_OBJVECTOR(BhvConstDeclare,_declares2);
			GELEM_UID(1);
			GELEM_EDITOBJ("常量声明2","常量列表");

	END_GOBJ();    

public: //当作protected
	std::vector<BhvConstDeclare> _declares2;
};


class CBgn_Consts:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Consts);

protected:

};

struct BhvVarDeclare
{
	BEGIN_GOBJ_PURE(BhvVarDeclare,1);

		GELEM_VAR_INIT( StringID,nm,StringID_Invalid);	
			GELEM_EDITVAR( "名称", GVT_U, GSem(GSem_StringID,"行为图内存变量名称"), "变量名称" );
		GELEM_VAR_INIT(BehaviorMemFlag,_flag,BehaviorMemFlag_None);
			GELEM_EDITVAR("标志",GVT_U,GSem(GSem_Flags,"需要保存:1,需要网络同步:2"),"标志");
		GELEM_VAR_INIT(BehaviorMemType,tp,BehaviorMemType_Bit);
		GELEM_EDITVAR("类型",GVT_S,GSem(GSem_Interger,
			"n/a:0"		"|缺省值(布尔型)&缺省值(整数)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Buff)&缺省值(Item)&缺省值(Unit)&缺省值(Resource)&自定义参数,"
			"布尔型:1"	"|缺省值(整数)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Buff)&缺省值(Item)&缺省值(Unit)&缺省值(Resource)&自定义参数,"
			"整数:2"		"|缺省值(布尔型)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Buff)&缺省值(Item)&缺省值(Unit)&缺省值(Resource)&自定义参数,"
			"浮点数:3"	"|缺省值(布尔型)&缺省值(整数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Buff)&缺省值(Item)&缺省值(Unit)&缺省值(Resource)&自定义参数,"
			"字符串ID:4"	"|缺省值(布尔型)&缺省值(整数)&缺省值(浮点数)&缺省值(Skill)&缺省值(Buff)&缺省值(Item)&缺省值(Unit)&缺省值(Resource)&自定义参数,"
			"技能表格项:5"	"|缺省值(布尔型)&缺省值(整数)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Buff)&缺省值(Item)&缺省值(Unit)&缺省值(Resource)&自定义参数,"
			"Buff表格项:6"		"|缺省值(布尔型)&缺省值(整数)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Item)&缺省值(Unit)&缺省值(Resource)&自定义参数,"
			"道具表格项:7"		"|缺省值(布尔型)&缺省值(整数)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Buff)&缺省值(Unit)&缺省值(Resource)&自定义参数,"
			"单位表格项:8"		"|缺省值(布尔型)&缺省值(整数)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Buff)&缺省值(Item)&缺省值(Resource)&自定义参数,"
			"Resource表格项:13"		"|缺省值(布尔型)&缺省值(整数)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Buff)&缺省值(Item)&缺省值(Unit)&自定义参数,"
			"位置:9"		"|缺省值(布尔型)&缺省值(整数)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Buff)&缺省值(Item)&缺省值(Unit)&缺省值(Resource)&自定义参数,"
			"ObjID:10"		"|缺省值(布尔型)&缺省值(整数)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Buff)&缺省值(Item)&缺省值(Unit)&缺省值(Resource)&自定义参数,"
			"GUID:11"		"|缺省值(布尔型)&缺省值(整数)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Buff)&缺省值(Item)&缺省值(Unit)&缺省值(Resource)&自定义参数,"
			"Obj:12"		"|缺省值(布尔型)&缺省值(整数)&缺省值(浮点数)&缺省值(字符串ID)&缺省值(Skill)&缺省值(Buff)&缺省值(Item)&缺省值(Unit)&缺省值(Resource)&自定义参数"
			),"变量类型");
		GELEM_VAR_INIT(BOOL,b,TRUE);
			GELEM_EDITVAR("缺省值(布尔型)",GVT_S,GSem_Boolean,"缺省值");
		GELEM_VAR_INIT(int,n,0);
			GELEM_EDITVAR("缺省值(整数)",GVT_S,GSem_Interger,"缺省值");
		GELEM_VAR_INIT(float,f,0.0f);
			GELEM_EDITVAR("缺省值(浮点数)",GVT_F,GSem_Float,"缺省值");
		GELEM_VAR_INIT( StringID,idStr,StringID_Invalid);	
			GELEM_EDITVAR("缺省值(字符串ID)", GVT_U, GSem(GSem_StringID,"变量值字符串"), "缺省值" );
		GELEM_VAR_INIT(RecordID,idSkill,RecordID_Invalid);
			GELEM_EDITVAR("缺省值(Skill)", GVT_U, GSem(GSem_RecordID,"skills"), "缺省值" );
		GELEM_VAR_INIT(RecordID,idBuff,RecordID_Invalid);
			GELEM_EDITVAR("缺省值(Buff)", GVT_U, GSem(GSem_RecordID,"buffs"), "缺省值" );
		GELEM_VAR_INIT(RecordID,idItem,RecordID_Invalid);
			GELEM_EDITVAR("缺省值(Item)", GVT_U, GSem(GSem_RecordID,"items"), "缺省值" );
		GELEM_VAR_INIT(RecordID,idUnit,RecordID_Invalid);
			GELEM_EDITVAR("缺省值(Unit)", GVT_U, GSem(GSem_RecordID,"units"), "缺省值" );
		GELEM_VAR_INIT(RecordID,idRes,RecordID_Invalid);
			GELEM_EDITVAR("缺省值(Resource)", GVT_U, GSem(GSem_RecordID,"resources"), "缺省值" );
		//XXXXX:more BehaviorMemType

	END_GOBJ();  

	StringID nm;

	BehaviorMemType	tp;

	//Default value
	BOOL b;
	int n;
	float f;
	StringID idStr;
	RecordID idSkill;
	RecordID idBuff;
	RecordID idItem;
	RecordID idUnit;
	RecordID idRes;
	//XXXXX:more BehaviorMemType


	BehaviorMemFlag _flag;

};

class CBgp_Vars:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Vars);

	virtual const char *GetTypeName()	{		return "变量声明";	};
	virtual DWORD GetStubCount()		{				return 0;	}
	virtual PadStub GetStub(DWORD idx)	{		return PadStub();	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist);

	BEGIN_GOBJ_PURE_UID(CBgp_Vars,1);
		GELEM_BGP_BASE();

		GELEM_OBJVECTOR(BhvVarDeclare,_declares2);
			GELEM_UID(1);
			GELEM_EDITOBJ("变量声明2","变量声明");

	END_GOBJ();    

public: //当作protected
	std::vector<BhvVarDeclare> _declares2;

};


class CBgn_Vars:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Vars);

protected:

};


class CBgp_Troops:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Troops);

	virtual const char *GetTypeName()	{		return "Troop声明";	};
	virtual DWORD GetStubCount()		{				return 0;	}
	virtual PadStub GetStub(DWORD idx)	{		return PadStub();	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist);

	BEGIN_GOBJ_PURE_UID(CBgp_Troops,1);
		GELEM_BGP_BASE();

		GELEM_VARVECTOR_INIT( StringID,_declares,StringID_Invalid);
			GELEM_EDITVAR( "Troop声明", GVT_U, GSem(GSem_StringID,"Troop名称"), "");

	END_GOBJ();    

public: //当作protected
	std::vector<StringID> _declares;

};


class CBgn_Troops:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Troops);

protected:

};


class CBgp_Proxy:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_Proxy);

	virtual const char *GetTypeName()	{		return "转接口";	};
	virtual DWORD GetStubCount()		{				return 2;	}
	virtual PadStub GetStub(DWORD idx)	
	{		
		BEGIN_STUB()
			STUB_IN(0,"入口");
			STUB_OUT(1,"出口");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)	
	{
		s=_desc;
	}

	BEGIN_GOBJ_PURE_UID(CBgp_Proxy,1);
		GELEM_BGP_BASE();

		GELEM_STRING(_desc);
			GELEM_EDITVAR("描述文字",GVT_String,GSem_Name,"描述文字");

	END_GOBJ();    

public:
	std::string _desc;
};

class CBgn_Proxy:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Proxy);

	CBgn_Proxy()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);
protected:

};


class CBgp_Roll:public CBehaviorGraphPad
{
	DEFINE_CLASS(CBgp_Roll);

	virtual const char *GetTypeName()	{		return "掷骰器";	};
	virtual DWORD GetStubCount()	
	{		
		return 3;
	}
	virtual PadStub GetStub(DWORD idx)
	{
		BEGIN_STUB()
			STUB_IN(0,"开始");
			STUB_OUT(1,"成功");
			STUB_OUT(2,"失败");
		END_STUB()
	}
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Helper;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	BEGIN_GOBJ_PURE_UID(CBgp_Roll,1);
		GELEM_BGP_BASE();

		GELEM_VAR_INIT(float,_rate,0.5f);
			GELEM_EDITVAR("每秒成功几率",GVT_F,GSem(GSem_Float,"0.0,1.0,0.01"),"每秒成功几率");
		GELEM_VAR_INIT(BOOL,_bTimedRate,TRUE);
			GELEM_EDITVAR("考虑时间因素",GVT_S,GSem_Boolean,"考虑时间因素");
	END_GOBJ();    

public: //当作protected

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (_bTimedRate)
			FormatString(s,"每秒%.2f%%几率成功",_rate*100.0f);
		else
			FormatString(s,"%.2f%%几率成功",_rate*100.0f);
	}
	float _rate;
	BOOL _bTimedRate;
};

class CBgn_Roll:public CBehaviorGraphNode
{
public:
	DEFINE_CLASS(CBgn_Roll);

	CBgn_Roll()
	{
	}

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:
};
