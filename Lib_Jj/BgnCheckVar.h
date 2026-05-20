#pragma once

#include "LevelBehavior.h"
#include "behaviorgraph/BehaviorGraphPads.h"



class CBgp_CheckVar:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckVar);

	enum Op
	{
		EQ=0,
		NE,
		GE,
		GT,
		LE,
		LT,
	};

	virtual const char *GetTypeName()	{		return "检测变量";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (nm==StringID_Invalid)
			s="n/a";
		else
		{
			std::string sOp;
			switch(op)
			{
				case EQ:
					sOp="等于";break;
				case NE:
					sOp="不等于";break;
				case GE:
					sOp="大于等于";break;
				case GT:
					sOp="大于";break;
				case LE:
					sOp="小于等于";break;
				case LT:
					sOp="小于";break;
			}
			FormatString(s,"%s%s%d ?",assist->GetStr(nm),sOp.c_str(),vRef);
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckVar,1);
		GELEM_BGP_BASE();
		GELEM_BEHAVIORMEM_NUMBER(nm,"名称","行为图内存变量名称")
		GELEM_VAR_INIT(Op,op,EQ);
			GELEM_EDITVAR("比较操作",GVT_S,GSem(GSem_Interger,"等于,不等于,大于等于,大于,小于等于,小于"),"比较操作");
		GELEM_VAR_INIT(int,vRef,0);
			GELEM_EDITVAR("比较值",GVT_S,GSem_Interger,"比较值");
	END_GOBJ();    

public: //当作protected
	StringID nm;
	Op op;
	int vRef;
};


class CBgn_CheckVar:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckVar);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};




class CBgp_CompareBool_Obsolete:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CompareBool_Obsolete);

	enum Op
	{
		EQ=0,
		NE,
	};

	virtual const char *GetTypeName()	{		return "Bool值比较(Obsolete)";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		std::string sOp;
		switch(op)
		{
		case EQ:
			sOp="等于";break;
		case NE:
			sOp="不等于";break;
		}
		FormatString(s,"%s%s%s ?",GetBPRDesc(bpr1,assist),sOp.c_str(),GetBPRDesc(bpr2,assist));
	}

	BEGIN_GOBJ_PURE_UID(CBgp_CompareBool_Obsolete,1);
		GELEM_BGP_BASE();
		GELEM_BPR_BOOL(bpr1,TRUE,"值1","值1");
		GELEM_VAR_INIT(Op,op,EQ);
			GELEM_EDITVAR("比较操作",GVT_S,GSem(GSem_Interger,"等于,不等于"),"比较操作");
		GELEM_BPR_BOOL(bpr2,TRUE,"值2","值2");
	END_GOBJ();    

public: //当作protected
	BPR_Bool bpr1;
	Op op;
	BPR_Bool bpr2;
};


class CBgn_CompareBool_Obsolete:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CompareBool_Obsolete);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

class CBgp_CompareInt_Obsolete:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CompareInt_Obsolete);

	enum Op
	{
		EQ=0,
		NE,
		GE,
		GT,
		LE,
		LT,
	};

	virtual const char *GetTypeName()	{		return "整数值比较(Obsolete)";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{

		std::string sOp;
		switch(op)
		{
		case EQ:
			sOp="等于";break;
		case NE:
			sOp="不等于";break;
		case GE:
			sOp="大于等于";break;
		case GT:
			sOp="大于";break;
		case LE:
			sOp="小于等于";break;
		case LT:
			sOp="小于";break;
		}
		std::string s1,s2;
		s1=GetBPRDesc(bpr1,assist);
		s2=GetBPRDesc(bpr2,assist);
		FormatString(s,"%s%s%s ?",s1.c_str(),sOp.c_str(),s2.c_str());
	}

	BEGIN_GOBJ_PURE_UID(CBgp_CompareInt_Obsolete,1);
		GELEM_BGP_BASE();
		GELEM_BPR_INT(bpr1,TRUE,"值1","值1");
		GELEM_VAR_INIT(Op,op,EQ);
			GELEM_EDITVAR("比较操作",GVT_S,GSem(GSem_Interger,"等于,不等于,大于等于,大于,小于等于,小于"),"比较操作");
		GELEM_BPR_INT(bpr2,TRUE,"值2","值2");
	END_GOBJ();    

public: //当作protected
	BPR_Int bpr1;
	Op op;
	BPR_Int bpr2;
};


class CBgn_CompareInt_Obsolete:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CompareInt_Obsolete);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};




class CBgp_CompareNumber:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CompareNumber);

	enum NumberType
	{
		Boolean,
		Int,
		Float,

		NumberType_ForceDword=0xffffffff,
	};

	enum Op
	{
		EQ=0,
		NE,
		GE,
		GT,
		LE,
		LT,

		Op_ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "数值比较";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		std::string sOp;
		switch(op)
		{
		case EQ:
			sOp="等于";break;
		case NE:
			sOp="不等于";break;
		case GE:
			sOp="大于等于";break;
		case GT:
			sOp="大于";break;
		case LE:
			sOp="小于等于";break;
		case LT:
			sOp="小于";break;
		}
		std::string s1,s2;
		switch(tpNum)
		{
			case Boolean:
				s1=GetBVRDesc_Bool(BVR_ARG(b1),assist);
				s2=GetBVRDesc_Bool(BVR_ARG(b2),assist);
				break;
			case Int:
				s1=GetBVRDesc_Int(BVR_ARG(i1),assist);
				s2=GetBVRDesc_Int(BVR_ARG(i2),assist);
				break;
			case Float:
				s1=GetBVRDesc_Float(BVR_ARG(f1),assist);
				s2=GetBVRDesc_Float(BVR_ARG(f2),assist);
				break;
		}
		FormatString(s,"%s%s%s ?",s1.c_str(),sOp.c_str(),s2.c_str());
	}

	BEGIN_GOBJ_PURE_UID(CBgp_CompareNumber,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(NumberType,tpNum,Float);
			GELEM_EDITVAR("数值类型",GVT_S,GSem(GSem_Interger,
				"布尔型" "|浮点数#1&浮点数#2&&整数#1&整数#2,"
				"整数" "|浮点数#1&浮点数#2&布尔值#1&布尔值#2,"
				"浮点数" "|布尔值#1&布尔值#2&整数#1&整数#2"
				),
				"数值类型");
		GELEM_VAR_INIT(float,f1,0.0f);
			GELEM_EDITVAR("浮点数#1",GVT_F,GSem_Float,"浮点数1");
			GELEM_BVR();
		GELEM_VAR_INIT(BOOL,b1,FALSE);
			GELEM_EDITVAR("布尔值#1",GVT_S,GSem_Boolean,"布尔值#1");
			GELEM_BVR();
		GELEM_VAR_INIT(int,i1,0);
			GELEM_EDITVAR("整数#1",GVT_S,GSem_Interger,"整数#1");
			GELEM_BVR();
		GELEM_VAR_INIT(Op,op,EQ);
			GELEM_EDITVAR("比较操作",GVT_S,GSem(GSem_Interger,"等于,不等于,大于等于,大于,小于等于,小于"),"比较操作");
		GELEM_VAR_INIT(float,f2,0.0f);
			GELEM_EDITVAR("浮点数#2",GVT_F,GSem_Float,"浮点数2");
			GELEM_BVR();
		GELEM_VAR_INIT(BOOL,b2,FALSE);
			GELEM_EDITVAR("布尔值#2",GVT_S,GSem_Boolean,"布尔值#2");
			GELEM_BVR();
		GELEM_VAR_INIT(int,i2,0);
			GELEM_EDITVAR("整数#2",GVT_S,GSem_Interger,"整数#2");
			GELEM_BVR();

	END_GOBJ();    

public: //当作protected
	NumberType tpNum;
	Op op;
	DEFINE_BVR(float,f1);
	DEFINE_BVR(float,f2);
	DEFINE_BVR(BOOL,b1);
	DEFINE_BVR(BOOL,b2);
	DEFINE_BVR(int,i1);
	DEFINE_BVR(int,i2);
};


class CBgn_CompareNumber:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CompareNumber);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

class CBgp_CompareStringID:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CompareStringID);


	enum Op
	{
		EQ=0,
		NE,

		Op_ForceDword=0xffffffff,
	};

	virtual const char *GetTypeName()	{		return "字符串ID比较";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		std::string sOp;
		switch(op)
		{
		case EQ:
			sOp="等于";break;
		case NE:
			sOp="不等于";break;
		}
		std::string s1,s2;
		s1=GetBVRDesc_StringID(BVR_ARG(v1),assist);
		s2=GetBVRDesc_StringID(BVR_ARG(v2),assist);
		FormatString(s,"%s %s %s ?",s1.c_str(),sOp.c_str(),s2.c_str());
	}

	BEGIN_GOBJ_PURE_UID2(CBgp_CompareStringID,439,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(StringID,v1,StringID_Invalid);
			GELEM_EDITVAR("字符串ID#1",GVT_F,GSem(GSem_StringID,"变量值字符串"),"字符串ID1");
			GELEM_BVR();
		GELEM_VAR_INIT(Op,op,EQ);
			GELEM_EDITVAR("比较操作",GVT_S,GSem(GSem_Interger,"等于,不等于"),"比较操作");
		GELEM_VAR_INIT(StringID,v2,StringID_Invalid);
			GELEM_EDITVAR("字符串ID#2",GVT_F,GSem(GSem_StringID,"变量值字符串"),"字符串ID2");
			GELEM_BVR();
	END_GOBJ();    

public: //当作protected
	Op op;
	DEFINE_BVR(StringID,v1);
	DEFINE_BVR(StringID,v2);
};


class CBgn_CompareStringID:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CompareStringID);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};


class CBgp_CheckVar_ID:public CBehaviorGraphPad
{
public:
	DEFINE_CLASS(CBgp_CheckVar_ID);

	virtual const char *GetTypeName()	{		return "检测游戏对象变量";	}
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
	virtual BgpCategory GetCategory()	{		return BgpCtgr_Var;	}
	virtual BgpFamily GetFamily()	{		return BgpFamily_Common;	}

	virtual void FillDesc(std::string &s,FillDescAssist *assist)
	{
		if (type==0)
		{
			if (nm==StringID_Invalid)
				s="n/a";
			else
			{
				FormatString(s,"检测游戏对象变量[%s]是否有值",assist->GetStr(nm));
			}
		}
		if (type==1)
		{
			if (nmBuff==StringID_Invalid)
				s="n/a";
			else
			{
				FormatString(s,"检测Buff表格项变量[%s]是否有值",assist->GetStr(nmBuff));
			}
		}
		if (type==2)
		{
			if (nmSkill==StringID_Invalid)
				s="n/a";
			else
			{
				FormatString(s,"检测Skill表格项变量[%s]是否有值",assist->GetStr(nmSkill));
			}
		}
		if (type==3)
		{
			if (nmItem==StringID_Invalid)
				s="n/a";
			else
			{
				FormatString(s,"检测Item表格项变量[%s]是否有值",assist->GetStr(nmItem));
			}
		}
		if (type==4)
		{
			if (nmObj==StringID_Invalid)
				s="n/a";
			else
			{
				FormatString(s,"检测游戏对象变量[%s]是否有值",assist->GetStr(nmObj));
			}
		}
	}

    BEGIN_GOBJ_PURE_UID(CBgp_CheckVar_ID,1);
		GELEM_BGP_BASE();
		GELEM_VAR_INIT(int,type,0)
			GELEM_EDITVAR("对象类型",GVT_S,GSem(GSem_Interger,
				"ObjID:0"		"|名称(Buff表格项)&名称(Skill表格项)&名称(Item表格项)&名称(Obj),"
				"Buff表格项:1"	"|名称(ObjID)&名称(Skill表格项)&名称(Item表格项)&名称(Obj),"
				"Skill表格项:2"	"|名称(ObjID)&名称(Buff表格项)&名称(Item表格项)&名称(Obj),"
				"Item表格项:3"	"|名称(ObjID)&名称(Buff表格项)&名称(Skill表格项)&名称(Obj),"
				"Obj:4"	"|名称(ObjID)&名称(Buff表格项)&名称(Skill表格项)&名称(Item表格项)"
				),"对象类型");
		GELEM_BEHAVIORMEM_OBJID(nm,"名称(ObjID)","行为图内存变量名称")
		GELEM_BEHAVIORMEM_BUFFRECORD(nmBuff,"名称(Buff表格项)","行为图内存变量名称")
		GELEM_BEHAVIORMEM_SKILLRECORD(nmSkill,"名称(Skill表格项)","行为图内存变量名称")
		GELEM_BEHAVIORMEM_ITEMRECORD(nmItem,"名称(Item表格项)","行为图内存变量名称")
		GELEM_BEHAVIORMEM_OBJ(nmObj,"名称(Obj)","行为图内存变量名称")
	END_GOBJ();    

public: //当作protected
	int type;
	StringID nm;
	StringID nmBuff;
	StringID nmSkill;
	StringID nmItem;
	StringID nmObj;
};


class CBgn_CheckVar_ID:public CLevelBgn
{
public:
	DEFINE_CLASS(CBgn_CheckVar_ID);

	virtual void Start(DWORD iStb,BGNOutputs &outputs);

protected:

};

