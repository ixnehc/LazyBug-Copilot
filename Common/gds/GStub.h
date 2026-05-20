/********************************************************************
	created:	2008/5/6   11:13
	file path:	d:\IxEngine\Common\gds
	author:		cxi
	
	purpose:	gstub implement
*********************************************************************/

#pragma once

#include "GDefines.h"

#include "../datapacket/DataPacket.h"

#include "../class/class.h"
#include "../mempool/mempool.h"
#include "GObj.h"
#include "GStack.h"

#include <string>
#include <unordered_map>



//////////////////////////////////////////////////////////////////////////
//使用以下macro来定义/存取property
//GStub_Begin(classname)	--classname为需要定义stub/prop的类名
//GStub_BeginD(classname,baseclass)	--classname为需要定义stub/prop的类名,baseclass为基类的类名
//
//		GStubDefine(name,type)	--name为property的名字,type为property的类型,是一个派生自GProperty的类
//		GPropDefine(name,type)	--name为property的名字,type为property的类型,是一个派生自GProperty的类
//		GCallDefine(name,typeIn,typeOut)	--name为property的名字,typeIn/typeOut为property的类型,是派生自GProperty的类
//			GPropSetSem(sem)		--定义一个property的语义(数据的具体含义)
//			GPropSetDesc(desc)		--定义一个property的文字说明
//			GStubSetType(type)		--定义一个property的类型,类型为GStubType的枚举
//
//GStub_End

//property的Set/Get函数为如下形式:
//BOOL prop_<name>(BOOL bSet,<type> *&prop);--<name>为prop的名字,<type>为prop的类型

//call的函数为如下形式
//BOOL call_<name>(<typeIn> *param,<typeOut> *&);--<name>为prop的名字,<typeIn>为参数的类型,<typeOut>为返回值的类型

//用来触发一个stub 链接的的 ***向外*** 数据传递
//GStub_Trigger(name)

//用来触发一个stub 链接的的 ***向内*** 数据传递
//#define GStubAbsorb(name)

//将一个数据从一个stub的链接上发送出去
//GStub_Fire(name,data)

//从一个stub链接上得到数据
//GStub_Obtain(name,data)

//设置/读取一个对象的property的值
//GPropSet(obj,name,data) --obj为对象的指针,name为prop的名字,data为一个property数据
//GPropGet(obj,name) --obj为对象的指针,name为prop的名字,返回一个property数据的指针

//调用一个函数
//GCall(obj,name,data) --obj为对象的指针,name为prop的名字,data为一个property数据,作为参数,
//										--返回一个GProperty的指针作为函数调用的返回值,如果调用失败,返回NULL

//链接两个对象的stubs
//GStubConnect(objOut,nameOut,objIn,nameIn) --objOut/objIn为对象的指针,
//																				--nameOut/nameIn为property的名字
//																				--返回成功与否


//Note:Stub和Prop的区别: Stub可以看作是可以连接的Prop,对象可以将stub
//的输出连到另一个对象的stub的输入上,来进行对象间的Prop数据的传递
//Call是一个函数调用,可以有返回值

//		Sample
//
//			struct Prop_Color:public GProperty
//			{
//				DECLARE_CLASS(Prop_Color);
//				DWORD col;
//
//				// GObj Defination --------------------------------------------------
//				BEGIN_GOBJ_PURE(Prop_Color,1);
//					GELEM_VAR_INIT(DWORD,col,ColorAlpha(0xffffff,0xff));
//				END_GOBJ();    
//			};
//
//			class CSample
//			{
//			public:
//				CSample()
//				{
//					v1=0;
//					v2=0;
//				}
//			protected:
//				GStubBegin(CSample);
//
//					GPropDefine(Color1,Prop_Color);
//						GPropSetDesc("颜色1");
//					GStubDefine(Color2,Prop_Color);
//						GPropSetDesc("颜色2");
//
//				GStubEnd();
//
//				BOOL prop_Color1(BOOL bSet,Prop_Color *&prop)
//				{
//					if (bSet)
//					{
//						...
//					}
//					else
//					{
//						...
//					}
//					return TRUE;
//				}
//
//				BOOL prop_Color2(BOOL bSet,Prop_Color *&prop)
//				{
//					if (bSet)
//					{
//						...
//					}
//					else
//					{
//						...
//					}
//					return TRUE;
//				}
//			};


class CClass;  
struct GObjBase;
struct GProperty
{
	virtual CClass *GetClass()=0;
	virtual GObjBase *GetGObj()=0;

	virtual GVarType GetGVT()	{		return GVT_None;	}

	BOOL CheckClassName(const char *name)	{		return GetClass()->CheckName(name);	}

	virtual BOOL IsSuperb()	{		return FALSE;	}//所谓superb的property就是可以容纳各种类型的数据的property(百搭)
	virtual GProperty* To(GProperty *dest)	{		return NULL;	}//superb的property需要实现
	virtual BOOL From(GProperty *src)	{		return FALSE;	}//superb的property需要实现

	virtual BOOL IsRef(){return FALSE;}//是不是PropRef

	virtual GProperty *Clone()
	{
		GProperty *p=(GProperty *)GetClass()->New();
		p->GetGObj()->Copy(GetGObj());
		return p;
	}
	virtual void DeleteThis()	{		Class_Delete(this);	}
	virtual BOOL Equals(GProperty *prop)
	{
		return GetGObj()->Equals(prop->GetGObj());
	}
	virtual void Save(CDataPacket &dp)
	{
		SaveGObj(dp,GetGObj());
	}
	virtual BOOL Load(CDataPacket &dp,BOOL *bRepaired)
	{
		return LoadGObj(dp,GetGObj(),bRepaired);
	}

	//User data support
	virtual BOOL SupportUD()	{		return FALSE;	}
	virtual int GetUD()	{		return NULL;	}
	virtual void SetUD(int ud)	{}

};

#define GProp_SafeDeleteThis(p)		{if (p)	 (p)->DeleteThis(); (p)=NULL;}

template<typename T,typename T2>
struct GPropertyPtr
{
	GPropertyPtr()
	{
		ptr=NULL;
	}
	~GPropertyPtr()
	{
		Release();
	}
	void Release()
	{
		GProp_SafeDeleteThis(ptr);
	}
	BOOL IsEmpty()	{		return ptr==NULL;	}
	T2* Obtain()
	{
		if (!ptr)
			ptr=Class_New(T);
		return &ptr->v;
	}
	T*ptr;
};


typedef WORD GOwnerData;

enum GStubType
{
	GStub_Property=0,//通用的属性值,可读可写
	GStub_Signal,//向外部发出通知(调用)的stub,可以看成事件,或者回调函数的接口
	GStub_Slot,//被外部通知的调用的stub,可以看成事件处理函数
	GStub_Call,//用来给外部调用的接口,类似于一个函数
	GStub_Origin,//由外部提取数据的stub
	GStub_Pump,//从外部提取数据的stub

	GStub_Max,
};

struct GStubBase
{
public:
	virtual CClass *GetDataClass()=0;//
	virtual CClass *GetRetDataClass()=0;
	virtual const char *GetName()=0;
	virtual GProperty *GetDefVal()=0;
	virtual GOwnerData *GetOwnerData(void *owner)=0;
	virtual GProperty *GetProp(void *owner)=0;
	virtual BOOL SetProp(void *owner,GProperty *data)=0;

	virtual GProperty *Call(void *owner,GProperty *param)=0;
	BOOL UseSuperbProp()	{		return GetDefVal()->IsSuperb();}//如果一个stub使用super prop,它可以接受任何类型的
																							//property数据(也可以和任何数据类型的stub连接)

	BOOL IsConnectable()	{		return idxConn!=-1;	}

	std::string name;
	std::string desc;
	std::string ownername;
	GSem sem;
	GStubType type;
	int idx;//自己在自己owner的stub list里的序号
	int idxConn;
};

struct GStubConn;
struct GStubOther
{
	GStubConn *conn;
	GStubOther *next;

	CMemPool<GStubOther>*pool;

	static CMemPool<GStubOther>*GetPool()
	{
		static CMemPool<GStubOther> pool("GStubOther");
		return &pool;
	}

	static GStubOther *New()	
	{		
		GStubOther *p=GetPool()->Alloc();
		p->pool=GetPool();
		return p;
	}
	void Free()	{		pool->Free(this);	}
	
};

inline BOOL CheckStubDataCompatible(GStubBase *stb1,GStubBase *stb2)
{
	if ((!stb1->UseSuperbProp())&&(!stb2->UseSuperbProp()))
	{
		if (strcmp(stb1->GetDataClass()->GetName(),stb2->GetDataClass()->GetName())!=0)
			return FALSE;//not the same data type
	}
	return TRUE;
}


struct GStubConn
{
	GStubConn()
	{
		otherIn=otherOut=NULL;
		owner=NULL;
		org=NULL;
	}
	~GStubConn()
	{
		DisconnectAll();
	}
	CClass *GetDataClass()	{		return org->GetDataClass();	}
	const char *GetName()	{		return org->GetName();	}
	GProperty *GetDefVal()	{		return org->GetDefVal();	}
	BOOL SetProp(GProperty *data)	{		return org->SetProp(owner,data);	}
	GProperty* GetProp()	{		return org->GetProp(owner);	}
	BOOL IsEnable()	{		return owner!=NULL;	}
	BOOL IsConnecting(GStubConn*other)	
	{		
		GStubOther *p=otherOut;
		while(p)
		{
			if (p->conn==other)
				return TRUE;
			p=p->next;
		}
		return FALSE;	
	}
	BOOL IsConnectedBy(GStubConn*other)	
	{		
		if (!other)
			return FALSE;
		return other->IsConnecting(this);
	}
	BOOL Connect(GStubConn*other)
	{
		if (!CheckStubDataCompatible(org,other->org))
			return FALSE;
		if (TRUE)
		{
			BOOL bCanLink=FALSE;
			if ((org->type==GStub_Signal)&&
				((other->org->type==GStub_Slot)||(other->org->type==GStub_Property)))
				bCanLink=TRUE;
			if ((other->org->type==GStub_Pump)&&
				((org->type==GStub_Origin)||(org->type==GStub_Property)))
				bCanLink=TRUE;
			if (!bCanLink)
				return FALSE;
		}

		//尝试把other添加在自己的链表末尾,过程中检查是否已经加入过了
		if (TRUE)
		{
			GStubOther **pp=&otherOut;
			while(*pp)
			{
				if ((*pp)->conn==other)
					return TRUE;//已经加入过了,不用链接了
				pp=&((*pp)->next);
			}
			(*pp)=GStubOther::New();
			(*pp)->conn=other;
			(*pp)->next=NULL;
		}

		//把自己添加在other的头上
		GStubOther *p=GStubOther::New();
		p->conn=this;
		p->next=other->otherIn;
		other->otherIn=p;

		return TRUE;
	}
	BOOL Disconnect(GStubConn*other)
	{
		_RemoveFromList(otherOut,other);
		_RemoveFromList(otherIn,other);
		_RemoveFromList(other->otherOut,this);
		_RemoveFromList(other->otherIn,this);
		return TRUE;
	}
	void DisconnectAll()
	{
		ClearConnected();
		ClearConnecting();
	}
	//清除所有连接到我身上的conn
	void ClearConnected()
	{
		GStubOther *p=otherIn;

		while(p)
		{
			_RemoveFromList(p->conn->otherOut,this);
			
			GStubOther *t=p;
			p=p->next;
			t->Free();
		}

		otherIn=NULL;
	}
	void ClearConnecting()
	{
		GStubOther *p=otherOut;

		while(p)
		{
			_RemoveFromList(p->conn->otherIn,this);

			GStubOther *t=p;
			p=p->next;
			t->Free();
		}

		otherOut=NULL;
	}

	void _Fire(GProperty *data)
	{
		if(data)
		{
			GStubOther*p=otherOut;
			while(p)
			{
				if (p->conn->IsEnable())
					p->conn->org->SetProp(p->conn->owner,data);
				p=p->next;
			}
		}
	}

	void Fire(GProperty *data)
	{
		if ((!otherOut)||!IsEnable())
			return;
		_Fire(data);
	}

	void Trigger()
	{
		if ((!otherOut)||!IsEnable())
			return;
		GProperty *data=org->GetProp(owner);
		if (!data)
			return;
		GProperty *t=data->Clone();
		_Fire(t);
		t->DeleteThis();
	}

	//必须从一个Pump类型的stub上发出,用来触发外部数据进入这个stub
	void Absorb()
	{
		if ((!otherIn)||!IsEnable())
			return;

		GProperty *data;
		GStubOther*p=otherIn;
		while(p)
		{
			if (p->conn->IsEnable())
			{
				data=p->conn->org->GetProp(p->conn->owner);
				if (data)
					org->SetProp(owner,data);
			}
			p=p->next;
		}
	}

	//注意:必须从Pump类型的stub上调用,用来直接取得数据,如果有多个Origin连在这个
	//Pump上,只得到第一个Origin的数据,
	//注意,返回的指针不能保存,也不能删除
	GProperty *Obtain()
	{
		if ((!otherIn)||!IsEnable())
			return NULL;

		GProperty *data;
		GStubOther*p=otherIn;
		while(p)
		{
			if (p->conn->IsEnable())
			{
				data=p->conn->org->GetProp(p->conn->owner);
				if (data)
					return data;
			}
			p=p->next;
		}

		return NULL;
	}

	GStubBase *org;
	void *owner;
	GStubOther *otherIn,*otherOut;//otherIn记录所有连到这个conn上的conn,otherOther
														//记录这个conn所有(连出去)连到的conn

protected:
	void _RemoveFromList(GStubOther*&others,GStubConn*conn)
	{
		GStubOther**p=&others;
		while(*p)
		{
			if ((*p)->conn==conn)
			{
				GStubOther *t=(*p);
				(*p)=(*p)->next;
				t->Free();
				return;
			}
			p=&(*p)->next;
		}
		assert(FALSE);

	}

};


struct GStubInfo
{
	GStubInfo()
	{
		bLoaded=FALSE;
		bEnabled=TRUE;
		data=0;
	}
	WORD bLoaded:1;
	WORD bEnabled:1;
	GOwnerData data;
	std::vector<GStubConn>conns;
};

//////////////////////////////////////////////////////////////////////////
//注意:在GStub的SetProp(..)我们用了一个临时变量cache以避免重入,其实应该
//是可以使用一个成员变量的,只要保证使用这个stub对应的prop_XXXX(..)不将传给它的GProperty
//直接Fire出去,如果我们可以确保这一点,我们就可以把cache写成成员变量.
template<typename T_type,typename T_owner>
struct GStub:public GStubBase
{
	typedef BOOL (T_owner::*FuncSetGet)(BOOL bSet,T_type *&);

	GStub()
	{
		funcSetGet=NULL;
		name="";
		type=GStub_Property;
	}
	virtual CClass *GetDataClass()	{		return defval.GetClass();	}
	virtual CClass *GetRetDataClass()	{		return NULL;	}
	virtual const char *GetName()	{		return name.c_str();	}
	virtual GProperty *GetDefVal()	{		return &defval;	}
	virtual GOwnerData *GetOwnerData(void *owner)	{		return ((T_owner*)owner)->GetStubOwnerData();	}
	virtual GProperty *GetProp(void *owner)
	{
		if (!funcSetGet)
			return NULL;
		T_type *p=&ret;

		GStackPush_Stub(name.c_str(),this,owner);

		if (FALSE==((((T_owner*)owner)->*funcSetGet)(FALSE,p)))
			p=NULL;

		GStackPop();
		return p;
	}
	virtual BOOL SetProp(void *owner,GProperty *data)
	{
		if (!funcSetGet)
			return FALSE;
		BOOL bRet;
		BOOL bSuperb=data->IsSuperb();
		if (bSuperb==defval.IsSuperb())
		{
			GStackPush_Stub(name.c_str(),this,owner);
			bRet=(((T_owner*)owner)->*funcSetGet)(TRUE,(T_type*&)data);//superb情况一样,无需转换
			GStackPop();
			return bRet;
		}
		T_type *t;
		T_type cache;
		if (bSuperb)
			t=(T_type*)data->To(&cache);//cache 不是superb的,转换一下
		else
		{//cache 是superb的
			cache.From(data);
			t=&cache;
		}
		GStackPush_Stub(name.c_str(),this,owner);
		bRet=(((T_owner*)owner)->*funcSetGet)(TRUE,t);
		GStackPop();
		return bRet;
	}
	virtual GProperty *Call(void *owner,GProperty *param)	{		return NULL;	}//不支持

	FuncSetGet funcSetGet;

	T_type ret;
	T_type defval;
};

template<typename T_type,typename T_owner>
struct GSlot:public GStubBase
{
	typedef BOOL (T_owner::*FuncSet)(T_type *&);

	GSlot()
	{
		funcSet=NULL;
		name="";
		type=GStub_Slot;
	}
	virtual CClass *GetDataClass()	{		return defval.GetClass();	}
	virtual CClass *GetRetDataClass()	{		return NULL;	}
	virtual const char *GetName()	{		return name.c_str();	}
	virtual GProperty *GetDefVal()	{		return &defval;	}
	virtual GOwnerData *GetOwnerData(void *owner)	{		return ((T_owner*)owner)->GetStubOwnerData();	}
	virtual GProperty *GetProp(void *owner)
	{
		return NULL;
	}
	virtual BOOL SetProp(void *owner,GProperty *data)
	{
		if (!funcSet)
			return FALSE;
		BOOL bRet;
		BOOL bSuperb=data->IsSuperb();
		if (bSuperb==defval.IsSuperb())
		{
			GStackPush_Stub(name.c_str(),this,owner);
			bRet=(((T_owner*)owner)->*funcSet)((T_type*&)data);//superb情况一样,无需转换
			GStackPop();
			return bRet;
		}
		T_type *t;
		T_type cache;
		if (bSuperb)
			t=(T_type*)data->To(&cache);//cache 不是superb的,转换一下
		else
		{//cache 是superb的
			cache.From(data);
			t=&cache;
		}
		GStackPush_Stub(name.c_str(),this,owner);
		bRet=(((T_owner*)owner)->*funcSet)(t);
		GStackPop();
		return bRet;
	}
	virtual GProperty *Call(void *owner,GProperty *param)	{		return NULL;	}//不支持

	FuncSet funcSet;

	T_type ret;
	T_type defval;
};


template<typename T_in_type,typename T_out_type,typename T_owner>
struct GStub2:public GStubBase
{
	typedef BOOL (T_owner::*FuncCall)(T_in_type *,T_out_type *&);

	GStub2()
	{
		funcCall=NULL;
		name="";
		type=GStub_Call;
	}
	virtual CClass *GetDataClass()	{		return defval.GetClass();	}
	virtual CClass *GetRetDataClass()	{		return out.GetClass();	}
	virtual const char *GetName()	{		return name.c_str();	}
	virtual GProperty *GetDefVal()	{		return &defval;	}
	virtual GOwnerData *GetOwnerData(void *owner)	{		return ((T_owner*)owner)->GetStubOwnerData();	}
	virtual GProperty *GetProp(void *owner)
	{
		if (!funcCall)
			return NULL;
		T_out_type *p=&out;

		GStackPush_Stub(name.c_str(),this,owner);

		if (FALSE==((((T_owner*)owner)->*funcCall)(&defval,p)))
			p=NULL;

		GStackPop();

		return p;
	}
	virtual BOOL SetProp(void *owner,GProperty *data)
	{
		if (!funcCall)
			return FALSE;
		T_out_type *p=&out;
		BOOL bSuperb=data->IsSuperb();
		T_in_type *q;
		T_in_type cache;
		if (bSuperb==defval.IsSuperb())
			q=(T_in_type *)data;
		else
		{
			if (bSuperb)
			{//cache 不是superb的
				q=(T_in_type *)data->To(&cache);
			}
			else
			{
				cache.From(data);
				q=&cache;
			}
		}

		BOOL bRet;
		GStackPush_Stub(name.c_str(),this,owner);
		bRet=(((T_owner*)owner)->*funcCall)(q,p);
		GStackPop();

		return bRet;

	}

	virtual GProperty *Call(void *owner,GProperty *param)	
	{
		if (!funcCall)
			return FALSE;
		T_out_type *p=&out;
		BOOL bSuperb=param->IsSuperb();

		T_in_type *q;
		T_in_type cache;
		if (bSuperb==defval.IsSuperb())
			q=(T_in_type *)param;
		else
		{
			if (bSuperb)
			{//cache 不是superb的
				q=(T_in_type *)param->To(&cache);
			}
			else
			{
				cache.From(param);
				q=&cache;
			}
		}

		GStackPush_Stub(name.c_str(),this,owner);
		if (FALSE==(((T_owner*)owner)->*funcCall)(q,p))
			p=NULL;
		GStackPop();

		return p;
	}


	FuncCall funcCall;

	T_in_type defval;
	T_out_type out;
};

#define GStub_SimpleType_Template(type,typeSimple)															\
struct type;																																	\
template<typename T_owner>																								\
struct GStub<type,T_owner>:public GStubBase																		\
{																																					\
	typedef BOOL (T_owner::*FuncSetGet)(BOOL bSet,typeSimple &);									\
	GStub()																																	\
	{																																				\
		funcSetGet=NULL;																												\
		name="";																															\
	}																																				\
	virtual CClass *GetDataClass()	{		return defval.GetClass();	}										\
	virtual CClass *GetRetDataClass()	{		return NULL;	}													\
	virtual const char *GetName()	{		return name.c_str();	}												\
	virtual GProperty *GetDefVal()	{		return &defval;	}														\
	virtual GOwnerData *GetOwnerData(void *owner)																\
				{		return ((T_owner*)owner)->GetStubOwnerData();	}										\
	virtual GProperty *GetProp(void *owner)																				\
	{																																				\
		if (!funcSetGet)																													\
			return NULL;																													\
		type*p=(type*)&cache;																										\
																																					\
		GStackPush_Stub(name.c_str(),this,owner);																		\
		if (FALSE==((((T_owner*)owner)->*funcSetGet)(FALSE,p->v)))										\
			p=NULL;																															\
		GStackPop();																														\
		return p;																																\
	}																																				\
	virtual BOOL SetProp(void *owner,GProperty *data)															\
	{																																				\
		if (!funcSetGet)																													\
			return FALSE;																													\
		BOOL bRet;																															\
		if (!data->IsSuperb())																											\
		{																																			\
			GStackPush_Stub(name.c_str(),this,owner);																	\
			bRet=(((T_owner*)owner)->*funcSetGet)(TRUE,((type*)data)->v);							\
			GStackPop();																													\
			return bRet;																													\
		}																																			\
		type *t=(type *)data->To(&cache);																					\
		GStackPush_Stub(name.c_str(),this,owner);																		\
		bRet=(((T_owner*)owner)->*funcSetGet)(TRUE,t->v);													\
		GStackPop();																														\
		return bRet;																														\
	}																																				\
	virtual GProperty *Call(void *owner,GProperty *param)	{	return NULL;}							\
																																					\
	FuncSetGet funcSetGet;																											\
	type defval;																																\
	type cache;																																\
};



struct Prop_Void;
template<typename T_owner>
struct GStub<Prop_Void,T_owner>:public GStubBase
{
	typedef BOOL (T_owner::*FuncSetGet)(BOOL bSet);

	GStub()
	{
		funcSetGet=NULL;
		name="";
	}
	virtual CClass *GetDataClass()	{		return defval.GetClass();	}
	virtual CClass *GetRetDataClass()	{		return NULL;	}
	virtual const char *GetName()	{		return name.c_str();	}
	virtual GProperty *GetDefVal()	{		return &defval;	}
	virtual GOwnerData *GetOwnerData(void *owner)	{		return ((T_owner*)owner)->GetStubOwnerData();	}
	virtual GProperty *GetProp(void *owner)
	{
		if (!funcSetGet)
			return NULL;
		BOOL bRet;
		GStackPush_Stub(name.c_str(),this,owner);
		bRet=((((T_owner*)owner)->*funcSetGet)(FALSE));
		GStackPop();
		if (!bRet)
			return NULL;
		return &cache;
	}
	virtual BOOL SetProp(void *owner,GProperty *data)
	{
		if (!funcSetGet)
			return FALSE;
		BOOL bRet;
		GStackPush_Stub(name.c_str(),this,owner);
		bRet=(((T_owner*)owner)->*funcSetGet)(TRUE);
		GStackPop();
		return bRet;
	}
	virtual GProperty *Call(void *owner,GProperty *param)	{	return NULL;}

	FuncSetGet funcSetGet;

	Prop_Void defval;
	Prop_Void cache;
};

struct Prop_String;
template<typename T_owner>
struct GStub<Prop_String,T_owner>:public GStubBase
{
	typedef BOOL (T_owner::*FuncSetGet)(BOOL bSet,const char *&str);

	GStub()
	{
		funcSetGet=NULL;
		name="";
	}
	virtual CClass *GetDataClass()	{		return defval.GetClass();	}
	virtual CClass *GetRetDataClass()	{		return NULL;	}
	virtual const char *GetName()	{		return name.c_str();	}
	virtual GProperty *GetDefVal()	{		return &defval;	}
	virtual GOwnerData *GetOwnerData(void *owner)	{		return ((T_owner*)owner)->GetStubOwnerData();	}
	virtual GProperty *GetProp(void *owner)
	{
		if (!funcSetGet)
			return NULL;
		const char *str;
		Prop_String *p=&cache;
		BOOL bRet;
		GStackPush_Stub(name.c_str(),this,owner);
		bRet=((((T_owner*)owner)->*funcSetGet)(FALSE,str));
		GStackPop();
		if (!bRet)
			return NULL;
		p->v=str;
		return p;
	}
	virtual BOOL SetProp(void *owner,GProperty *data)
	{
		if (!funcSetGet)
			return FALSE;
		BOOL bRet;
		if (!data->IsSuperb())
		{
			const char *str=((Prop_String *)data)->v.c_str();
			GStackPush_Stub(name.c_str(),this,owner);
			bRet=(((T_owner*)owner)->*funcSetGet)(TRUE,str);
			GStackPop();
			return bRet;
		}
		Prop_String *t=(Prop_String *)data->To(&cache);
		const char *str=t->v.c_str();
		GStackPush_Stub(name.c_str(),this,owner);
		bRet=(((T_owner*)owner)->*funcSetGet)(TRUE,str);
		GStackPop();
		return bRet;
	}
	virtual GProperty *Call(void *owner,GProperty *param)	{	return NULL;}

	FuncSetGet funcSetGet;

	Prop_String defval;
	Prop_String cache;
};



GStub_SimpleType_Template(Prop_S,int);
GStub_SimpleType_Template(Prop_U,DWORD);
GStub_SimpleType_Template(Prop_F,float);
GStub_SimpleType_Template(Prop_Fx2,i_math::vector2df);
GStub_SimpleType_Template(Prop_Fx3,i_math::vector3df);
GStub_SimpleType_Template(Prop_Fx4,i_math::vector4df);
GStub_SimpleType_Template(Prop_Fx6,i_math::aabbox3df);
GStub_SimpleType_Template(Prop_Fx12,i_math::matrix43f);
GStub_SimpleType_Template(Prop_Fx16,i_math::matrix44f);
GStub_SimpleType_Template(Prop_Sx4,i_math::vector4di);
GStub_SimpleType_Template(Prop_Sx2,i_math::vector2di);
GStub_SimpleType_Template(Prop_Bx4,i_math::vector4db);
//XXXXX:more simple type property


struct GStubs
{
	GStubs()
	{
		bLoad=FALSE;
		nConn=0;
	}
	~GStubs()
	{
		stubs.clear();
		stubs2.clear();
		bLoad=FALSE;
	}
	BOOL bLoad;
	DWORD nConn;
	std::unordered_map<std::string,int>stubs;
	std::vector<GStubBase *>stubs2;
};

#define GStubBegin(__ownerclss)																								\
public:																																			\
	virtual void DisableStub()																										\
	{																																				\
		if (1==_stbinfo.bEnabled)																									\
		{																																			\
			_stbinfo.bEnabled=0;																										\
			for (int i=0;i<_stbinfo.conns.size();i++)																		\
				_stbinfo.conns[i].owner=NULL;																					\
		}																																			\
	}																																				\
	virtual int FindStubIdx(const char *name)																			\
	{																																				\
		if (0==_stbinfo.bEnabled)																									\
			return -1;																															\
		_LoadStubs();																														\
		std::unordered_map<std::string,int>::iterator it=_stubs().stubs.find(std::string(name));					\
		if (it==_stubs().stubs.end())																								\
			return -1;																															\
		return (*it).second;																												\
	}																																				\
	virtual GStubBase*FindStub(const char *name)																	\
	{																																				\
		int idx=FindStubIdx(name);																								\
		if (idx<0)																																\
			return NULL;																													\
		return _stubs().stubs2[idx];																								\
	}																																				\
	virtual GStubConn *FindConn(const char *name)																\
	{																																				\
		if (0==_stbinfo.bEnabled)																									\
			return NULL;																													\
		GStubBase *stub=FindStub(name);																					\
		if (!stub)																																\
			return NULL;																													\
		if (stub->idxConn==-1)																										\
			return NULL;																													\
		return &_stbinfo.conns[stub->idxConn];																			\
	}																																				\
	virtual GStubConn *GetConn(int idx)	{		return &_stbinfo.conns[idx];	}						\
	virtual int FindConnIdx(const char *name)																			\
	{																																				\
		if (0==_stbinfo.bEnabled)																									\
			return -1;																															\
		_LoadStubs();																														\
		std::unordered_map<std::string,int>::iterator it=_stubs().stubs.find(std::string(name));		\
		if (it==_stubs().stubs.end())																								\
			return -1;																															\
		GStubBase *stub=_stubs().stubs2[(*it).second];																\
		return stub->idxConn;																										\
	}																																				\
	virtual DWORD GetStubCount()																							\
	{																																				\
		if (0==_stbinfo.bEnabled)																									\
			return 0;																															\
		_LoadStubs();																														\
		return _stubs().stubs2.size();																								\
	}																																				\
	virtual GStubBase *GetStub(DWORD idx)																			\
	{																																				\
		if (0==_stbinfo.bEnabled)																									\
			return NULL;																													\
		_LoadStubs();																														\
		return _stubs().stubs2[idx];																								\
	}																																				\
	virtual void *GetStubOwner()	{		return this;	}																\
	GOwnerData *GetStubOwnerData()	{		return &_stbinfo.data	;}										\
protected:																																	\
	GStubs&_stubs()																														\
	{																																				\
		static GStubs stubs;																											\
		return stubs;																														\
	}																																				\
	GStubInfo _stbinfo;																													\
	void _LoadStubs()																													\
	{																																				\
		if (!_stubs().bLoad)																												\
		{																																			\
			typedef __ownerclss OwnerClassType;																			\
			_stubs().nConn=0;																											\
			const char *ownername=#__ownerclss;																		\
			GStubBase *last=NULL;

//"D" for deriving
#define GStubBeginD(__ownerclss,__baseclss)																		\
public:																																			\
	virtual GStubBase *FindStub(const char *name)																	\
	{																																				\
		if (0==_stbinfo.bEnabled)																									\
			return NULL;																													\
		_LoadStubs();																														\
		std::unordered_map<std::string,int>::iterator it=_stubs().stubs.find(std::string(name));					\
		if (it==_stubs().stubs.end())																								\
			return __baseclss::FindStub(name);																				\
		return _stubs().stubs2[(*it).second];																					\
	}																																				\
	virtual GStubConn *FindConn(const char *name)																\
	{																																				\
		if (0==_stbinfo.bEnabled)																									\
			return NULL;																													\
		_LoadStubs();																														\
		std::unordered_map<std::string,int>::iterator it=_stubs().stubs.find(std::string(name));		\
		if (it==_stubs().stubs.end())																								\
			return __baseclss::FindConn(name);																				\
		GStubBase *stub=_stubs().stubs2[(*it).second];																\
		if (stub->idxConn==-1)																										\
			return NULL;																													\
		return &_stbinfo.conns[stub->idxConn];																			\
	}																																				\
	virtual GStubConn *GetConn(int idx)	{		return &_stbinfo.conns[idx];	}						\
	virtual int FindConnIdx(const char *name)																			\
	{																																				\
		if (0==_stbinfo.bEnabled)																									\
			return -1;																															\
		_LoadStubs();																														\
		std::unordered_map<std::string,int>::iterator it=_stubs().stubs.find(std::string(name));		\
		if (it==_stubs().stubs.end())																								\
			return -1;																															\
		GStubBase *stub=_stubs().stubs2[(*it).second];																\
		return stub->idxConn;																										\
	}																																				\
	virtual DWORD GetStubCount()																							\
	{																																				\
		if (0==_stbinfo.bEnabled)																									\
			return 0;																															\
		_LoadStubs();																														\
		return _stubs().stubs2.size()+__baseclss::GetStubCount();												\
	}																																				\
	virtual GStubBase *GetStub(DWORD idx)																			\
	{																																				\
		if (0==_stbinfo.bEnabled)																									\
			return NULL;																													\
		_LoadStubs();																														\
		if (idx<_stubs().stubs2.size())																								\
			return _stubs().stubs2[idx];																							\
		return __baseclss::GetStub(idx-_stubs().stubs2.size());														\
	}																																				\
	virtual void *GetStubOwner()	{		return this;	}																\
	GOwnerData *GetStubOwnerData()	{		return &_stbinfo.data	;}										\
protected:																																	\
	GStubs&_stubs()																														\
	{																																				\
		static GStubs stubs;																											\
		return stubs;																														\
	}																																				\
	GStubInfo _stbinfo;																													\
	void _LoadStubs()																													\
	{																																				\
		if (!_stubs().bLoad)																												\
		{																																			\
			typedef __ownerclss OwnerClassType;																			\
			_stubs().nConn=0;																											\
			const char *ownername=#__ownerclss;																		\
			GStubBase *last=NULL;

//connectable property
#define GStubDefine(__name,__type)																						\
			{																																		\
				static GStub<__type,OwnerClassType>org;																\
				last=&org;																													\
				int idx=_stubs().stubs2.size();																					\
				_stubs().stubs[std::string(#__name)]=idx;																	\
				_stubs().stubs2.push_back(&org);																				\
				org.funcSetGet=&OwnerClassType::prop_##__name;												\
				org.name=#__name;																									\
				org.ownername=ownername;																					\
				org.idx=idx;																												\
				org.idxConn=_stubs().nConn;																					\
				_stubs().nConn++;																										\
			}

#define GPropDefine(__name,__type)																						\
			{																																		\
				static GStub<__type,OwnerClassType>org;																\
				last=&org;																													\
				int idx=_stubs().stubs2.size();																					\
				_stubs().stubs[std::string(#__name)]=idx;																	\
				_stubs().stubs2.push_back(&org);																				\
				org.funcSetGet=&OwnerClassType::prop_##__name;												\
				org.name=#__name;																									\
				org.ownername=ownername;																					\
				org.idx=idx;																												\
				org.idxConn=-1;																											\
			}

#define GCallDefine(__name,__typeIn,__typeOut)																	\
			{																																		\
				static GStub2<__typeIn,__typeOut,OwnerClassType>org;										\
				last=&org;																													\
				int idx=_stubs().stubs2.size();																					\
				_stubs().stubs[std::string(#__name)]=idx;																	\
				_stubs().stubs2.push_back(&org);																				\
				org.funcCall=&OwnerClassType::call_##__name;													\
				org.name=#__name;																									\
				org.ownername=ownername;																					\
				org.idx=idx;																												\
				org.idxConn=-1;																											\
			}

#define GSignalDefine(__name,__type)																						\
			{																																		\
				static GStub<__type,OwnerClassType>org;																\
				last=&org;																													\
				int idx=_stubs().stubs2.size();																					\
				_stubs().stubs[std::string(#__name)]=idx;																	\
				_stubs().stubs2.push_back(&org);																				\
				org.funcSetGet=NULL;																								\
				org.type=GStub_Signal;																								\
				org.name=#__name;																									\
				org.ownername=ownername;																					\
				org.idx=idx;																												\
				org.idxConn=_stubs().nConn;																					\
				_stubs().nConn++;																										\
			}

#define GPropSetSem(str)																											\
			if (last)																																\
				last->sem=str;

#define GPropSetDesc(str)																										\
			if (last)																																\
				last->desc=str;

#define GPropSetDefault(defval)																								\
			if (last)																																\
				last->GetDefVal()->GetGObj()->Copy(defval.GetGObj());	

#define GPropDefault(__type)																									\
			if (last)																																\
				((__type*)last->GetDefVal())

#define GStubSetType(tp)																											\
			if (last)																																\
				last->type=tp;

#define GStubEnd()																													\
			_stubs().bLoad=TRUE;																									\
		}																																			\
		if (!_stbinfo.bLoaded)																											\
		{																																			\
			_stbinfo.conns.resize(_stubs().nConn);																			\
			for (int i=0;i<_stubs().stubs2.size();i++)																		\
			{																																		\
				GStubBase *org=_stubs().stubs2[i];																			\
				if (org->idxConn!=-1)																								\
				{																																	\
					_stbinfo.conns[org->idxConn].owner=this;															\
					_stbinfo.conns[org->idxConn].org=org;																\
				}																																	\
			}																																		\
			_stbinfo.bLoaded=1;																										\
		}																																			\
	}


#define GStubTrigger(__name)																									\
{																																					\
	GStubConn *conn=FindConn(#__name);																				\
	if (conn)																																	\
		conn->Trigger();																													\
}

#define GStubAbsorb(__name)																									\
{																																					\
	GStubConn *conn=FindConn(#__name);																				\
	if (conn)																																	\
		conn->Absorb();																													\
}


#define GStubFire(__name,data)																								\
{																																					\
	GStubConn *conn=FindConn(#__name);																				\
	if (conn)																																	\
		conn->Fire(&data);																											\
}

#define GStubObtain(__name,data)																							\
{																																					\
	GStubConn *conn=FindConn(#__name);																				\
	if (conn)																																	\
		data=conn->Obtain();																										\
	else																																			\
		data=NULL;																														\
}

inline GProperty *_GPropGet(void *owner,GStubBase *stb)
{
	if ((!stb)||(!owner))
		return NULL;
	return stb->GetProp(owner);
}

inline BOOL _GPropSet(void *owner,GStubBase *stb,GProperty *data)
{
	if ((!stb)||(!owner))
		return FALSE;
	return stb->SetProp(owner,data);
}

#define GPropGet(__owner,__name)	_GPropGet(__owner->GetStubOwner(),__owner->FindStub(__name));
#define GPropSet(__owner,__name,__data)	_GPropSet(__owner->GetStubOwner(),__owner->FindStub(__name),&(__data));

inline GProperty *_GCall(void *owner,GStubBase *stb,GProperty *param)
{
	if (!stb)
		return NULL;
	return stb->Call(owner,param);
}
#define GCall(__owner,__name,__data)	_GCall(__owner->GetStubOwner(),__owner->FindStub(__name),&(__data));

inline BOOL _GStubConnect(GStubConn *conn1,GStubConn *conn2)
{
	if (conn1&&conn2)
		return conn1->Connect(conn2);
	return FALSE;
}
#define GStubConnect(__owner1,__name1,__owner2,__name2)								\
	_GStubConnect((__owner1)->FindConn(#__name1),(__owner2)->FindConn(#__name2));

inline void _GStubDisconnect(GStubConn *conn)
{
	if (conn)
		conn->DisconnectAll();
}

#define GStubDisconnect(__owner,__name)	_GStubDisconnect((__owner)->FindConn(#__name));

#define SimplePropAssign(v,arg)										\
	if (bSet)																			\
	{																						\
		v=arg;																			\
	}																						\
	else																					\
	{																						\
		arg=v;																			\
	}																						\
	return TRUE;
