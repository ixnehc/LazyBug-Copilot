/********************************************************************
	created:	2011/7/22   16:22
	file path:	e:\IxEngine\Interfaces\WorldSystem
	author:		chenxi
	
	purpose:	ProtoNode,Asset,Entity,LuaObj的封装类接口
*********************************************************************/
#pragma once

#include "log/LogDump.h"
#include "../stubparams/stubparams.h"

class IAsset;
class IEntity;
class ILuaObj;
class CAssetCtrl;
struct GProperty;
class IAnimNode;
//Nael是 protoNode,Asset,Entity,Luaobj的缩写
class INael
{
public:
	INTERFACE_REFCOUNT;
	virtual BOOL IsEmpty()=0;
	virtual BOOL IsPN()=0;
	virtual IAsset *GetAst()=0;
	virtual IEntity *GetEntity()=0;
	virtual ILuaObj *GetLO()=0;

	virtual const char *GetDebugName()=0;//返回一个字符串用来描述这个nael,主要用于调试输出

	virtual BOOL Set(const char *name,GProperty *prop)=0;
	virtual BOOL Set(const char *name,int v)=0;
	virtual BOOL Set(const char *name,float v)=0;
	virtual BOOL Set(const char *name,const char *str,float v)=0;
	virtual BOOL Set(const char *name,const char *str)=0;
	virtual BOOL Set(const char *name,IAnimNode *an)=0;
	virtual BOOL Set(const char *name,i_math::matrix43f &mat)=0;
	virtual BOOL Set(const char *name,i_math::recti &rc)=0;
	virtual GProperty *Get(const char *name)=0;
	virtual BOOL Get(const char *name,int &v)=0;
	virtual BOOL Get(const char *name,DWORD &v)=0;
	virtual BOOL Get(const char *name,float&v)=0;
	virtual BOOL Get(const char *name,i_math::matrix43f &mat)=0;
	virtual BOOL Get(const char *name,i_math::recti &rc)=0;
	virtual GProperty *Call(const char *name,GProperty *prop)=0;

	virtual BOOL PushParam(int v)=0;
	virtual BOOL PushParam(float v)=0;
	virtual BOOL PushParam(const char *str)=0;
	virtual BOOL PushParam(IAnimNode *an)=0;
	virtual BOOL PushParam(INael *ael)=0;
	virtual BOOL PushParam(i_math::vector3df &v)=0;
	virtual BOOL PushParam(i_math::matrix43f &v)=0;
	virtual BOOL PushParam(GProperty *prop)=0;
	virtual BOOL Set(const char *name)=0;
	virtual GProperty *Call(const char *name)=0;
	virtual BOOL Call(const char *name,int &ret)=0;
	virtual BOOL Call(const char *name,float &ret)=0;

	virtual BOOL IsAlive()=0;
	virtual GStubBase *FindStub(const char *name,void *&owner)=0;
	virtual BOOL ExistStub(const char *name)=0;
	virtual GStubConn*FindConn(const char *name)=0;
	virtual BOOL Destroy()=0;
	virtual CAssetCtrl *GetCtrl()=0;

	//返回的nael指针带一个引用计数
	virtual INael *ObtainSubNael(const char *nmPN,BOOL bSilent=FALSE)=0;//nmPN是proto node的名字(注意不是路径)

	//Create返回的INael指针都带一个引用计数
	virtual BOOL BeginCreate(i_math::matrix43f *mat=NULL,const char *pathOverride="",IEntity *owner=NULL)=0;
	virtual BOOL AddCreateArg(const char *nameProp,double v)=0;
	virtual BOOL AddCreateArg(const char *nameProp,const char *str)=0;
	virtual BOOL AddCreateArg(const char *nameProp,GProperty *prop)=0;
	virtual INael *EndCreate()=0;
	virtual INael *Create(i_math::matrix43f *mat=NULL,const char *pathOverride="",IEntity *owner=NULL)=0;
};



struct StbParams;
template <typename T_OwnerClass>
class CSlotPool
{
public:
	typedef BOOL (T_OwnerClass::*FuncSet)(StbParams*&);
	GStubConn *ObtainConn(T_OwnerClass *owner,FuncSet func)
	{
		std::deque<GSlot<StbParams,T_OwnerClass> >*stbs=_GetStubs();
		int i;
		for (i=0;i<stbs->size();i++)
		{
			if ((*stbs)[i].funcSet==func)
				break;
		}
		GSlot<StbParams,T_OwnerClass> *stb=NULL;
		if (i>=stbs->size())
		{
			stbs->resize(i+1);
			stb=&(*stbs)[i];

			stb->funcSet=func;
			stb->type=GStub_Slot;	
			stb->name="";
			stb->ownername="";
			stb->idx=i;
			stb->idxConn=i;
		}
		else
			stb=&(*stbs)[i];

		_conns.resize(stbs->size());
		GStubConn *conn=&_conns[i];
		if (!conn->org)
		{
			conn->org=stb;
			conn->owner=owner;
		}
		return conn;
	}

	BOOL BindNaelSignal(INael *nael,const char *nmSignal,T_OwnerClass *owner,FuncSet func,BOOL bDump)
	{
		if (!nael)
			return FALSE;
		GStubConn *conn=ObtainConn(owner,func);
		GStubConn *connOut=nael->FindConn(nmSignal);
		if (!connOut)
		{
			if (bDump)
			{
				LOG_DUMP_2P("Client",Log_Error,"无法找到名为\"%s\"的Stub(%s)",nmSignal,nael->GetDebugName());
			}
		}

		if (connOut&&conn)
			return connOut->Connect(conn);
		return FALSE;
	}

protected:
	std::deque<GSlot<StbParams,T_OwnerClass> >*_GetStubs()
	{
		static std::deque<GSlot<StbParams,T_OwnerClass> >t;
		return &t;
	}

	std::deque<GStubConn> _conns;

};


#define DEFINE_SIGNAL_HANDLER(clss)																					\
public:																																			\
GOwnerData *GetStubOwnerData()	{		return NULL;	}														\
typedef clss __ThisType;																												\
CSlotPool<clss> __pool;

#define SET_SIGNAL_HANDLER(nael,nmSignal,handler)														\
	__pool.BindNaelSignal(nael,nmSignal,this,&__ThisType::handler,TRUE)

//ND代表No Dump
#define SET_SIGNAL_HANDLER_ND(nael,nmSignal,handler)														\
	__pool.BindNaelSignal(nael,nmSignal,this,&__ThisType::handler,FALSE)

//Signal Handler的函数形态:
//BOOL OnXXXXX(StbParams *&params);
