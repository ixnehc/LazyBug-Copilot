#pragma once

#include "../fastdelegate/FastDelegate.h"
#include "../mempool/mempool.h"

#define MAX_STUB_NAME 32
#define MAX_STUB_TYPENAME 32

enum StubType
{
	StubType_None=0,

	StubType_Pos,
	StubType_Mat,
	StubType_Int,//interger

	StubType_UserDefinedBegin=4096,
//XXXXX:more StubType
};

class CStubCore
{
public:
	CStubCore();
	~CStubCore()	{		Break();	}

	BOOL IsOut()	{		return !_bIn;	}
	BOOL IsBreak()	{		return (_link==0);	}
	void *GetOwner()	{		return _owner;	}
	const char *GetName()	{		return _name;}
	void SetName(const char *name)
	{
		strncpy(_name,name,sizeof(_name)-1);
		_name[sizeof(_name)-1]=0;
	}
	virtual StubType GetType()	{		return StubType_None;	}
	virtual void SetModified()	{	}

	BOOL Link(CStubCore *other);
	void Break();


	CStubCore *GetLink()	{		return _link;	}
	CStubCore *GetNext()	{		return _next;	}

public://take it as protected


	virtual void *_GetStuff()	{		return NULL;	}
	virtual void _OnModify()	{}

	virtual void _DeleteThis()=0;

	typedef fastdelegate::FastDelegate1<CStubCore*>StubOnLink;
	typedef fastdelegate::FastDelegate1<CStubCore*>StubOnBreak;
	StubOnLink _dlgtLink;//called when this stub is linked to another stub
	StubOnBreak _dlgtBreak;//called when this stub is breaked from another stub

	BOOL _bIn;//whether this stub is for input or output data
	char _name[MAX_STUB_NAME];
	CStubCore *_link;
	CStubCore *_next;//for linkage of all the stubin for a single stubout

	void *_owner;

};


template <typename T,StubType tp>
class CStubOut:public CStubCore
{
public:
	CStubOut()
	{
		_bIn=FALSE;
		_bind=NULL;
		_bModified=TRUE;
		_dlgt=NULL;
	}
	virtual StubType GetType()	{		return tp;	}
	virtual void SetModified()
	{		
		_bModified=TRUE;	
		//Notify all the linked stubin
		//IMPORTANT: this is NOT very safe,because the target CStubCore may be destroyed 
		//during the _OnModify()
		CStubCore *p=_link;
		while(p)
		{
			p->_OnModify();
			p=p->_next;
		}
	}

	typedef fastdelegate::FastDelegate2<CStubCore*,T *>StubOutCallBack;
	void Bind(StubOutCallBack& dlgt)	{		_dlgt=dlgt;	_bind=NULL;	}
	void Bind(T *b)	{		_bind=b;	_dlgt=NULL;	}
public://take it as protected
	virtual void *_GetStuff()	
	{		
		if (_bind)
			return (void*)_bind;	
		if (!_bModified)
			return &_cache;
		if (!_dlgt)
			return NULL;
		_dlgt(this,&_cache);
		_bModified=FALSE;
		return &_cache;
	}

	static CMemPool_dq<CStubOut<T,tp> >*_GetMemPool()
	{
		static CMemPool_dq<CStubOut<T,tp> >pool;
		return &pool;
	}
	static CStubOut<T,tp>*_New()	{		return _GetMemPool()->Alloc();	}
	virtual void _DeleteThis()	{		_GetMemPool()->Free(this);	}

	T *_bind;
	StubOutCallBack _dlgt;
	BOOL _bModified;
	T _cache;
};



template <typename T,StubType tp>
class CStubIn:public CStubCore
{
public:
	CStubIn()
	{
		_bIn=TRUE;
		_bind=NULL;
		_dlgtModify=NULL;
	}
	virtual StubType GetType()	{		return tp;	}
	typedef fastdelegate::FastDelegate2<CStubCore*,T *>StubInOnModify;

	void Bind(T *bind,StubInOnModify &dlgtModify)
	{
		_bind=bind;
		_dlgtModify=dlgtModify;
	}

	virtual void _OnModify()
	{
		if (_link)
		{
			T *t=(T*)_link->_GetStuff();
			if (t)
			{
				if (_bind)
					*_bind=*t;
			}
			if (_dlgtModify)
				_dlgtModify(this,t);
		}
	}

public://take it as protected
	static CMemPool_dq<CStubIn<T,tp> >*_GetMemPool()
	{
		static CMemPool_dq<CStubIn<T,tp> >pool;
		return &pool;
	}
	static CStubIn<T,tp>*_New()	{		return _GetMemPool()->Alloc();	}
	virtual void _DeleteThis()	{		_GetMemPool()->Free(this);	}

	T *_bind;
	StubInOnModify _dlgtModify;
};



//Use the following macro to define stubs
//		DECLARE_STUB
//		StubID stub;
//
//		stbid=StubIn_XXX(stubname,&var)
//		stbid=StubIn_XXX(stubname,func)
//		stbid=StubOut_XXX(stubname,&var)
//		stbid=StubOut_XXX(stubname,func)
//
//		StubOut_NotifyMod(stbidOut);
//
//
//Example:
//in the class defination(.h file):
//		class CSample
//		{
//			...
//
//			DECLARE_STUB;
//			StubID in;
//			StubID in2;
//			StubID out;
//			StubID out2;
//
//			
//			i_math::vector3df _vIn;
//			i_math::vector3df _vOut;
//			i_math::vector3df _ResolveOut(CStubCore*);
//			void _NotifyIn(CStubCore*,i_math::vector3df *);
//
//			...
//		};

//in the cpp file
//			in=StubIn_pos("In",&_vIn);
//			in2=StubIn_pos("In2",_NotifyIn);
//			out=StubOut_pos("Out",&_vOut);
//			out2=StubOut_pos("Out2",_ResolveOut);
//


typedef CStubCore* StubID;
#define StubID_Null NULL;

struct Stubs:public std::vector<CStubCore*>
{
	~Stubs()
	{
		Clear();
	}
	void Clear()
	{
		for (int i=0;i<size();i++)
		{
			(*this)[i]->Break();
			(*this)[i]->_DeleteThis();
		}
		clear();
	}
};

#define DECLARE_STUB(classname)																		\
public:																														\
	virtual DWORD GetStubCount()																		\
					{		return _stubs.size();	}																\
	virtual CStubCore *GetStub(DWORD idx)														\
					{		return _stubs[idx];	}																	\
	virtual CStubCore *FindStub(const char *name)												\
	{																															\
		DWORD sz=_stubs.size();																				\
		for (int i=0;i<sz;i++)																						\
		{																														\
			if (strcmp(_stubs[i]->GetName(),name)==0)											\
				return _stubs[i];																						\
		}																														\
		return NULL;																									\
	}																															\
	void ClearStub(){	_stubs.Clear();}																	\
protected:																												\
	template<typename T,StubType tp>																\
	StubID _StubInBind(const char *name,T *var)													\
	{																															\
		CStubIn<T,tp> *p=CStubIn<T,tp>::_New();													\
		p->_owner=(void*)this;																					\
		p->SetName(name);																						\
		p->_bind=(var);																								\
		p->_dlgtModify=NULL;																					\
		_stubs.push_back(p);																						\
		return (StubID)p;																							\
	}																															\
	template<typename T,StubType tp>																\
	StubID _StubInBind(const char *name,																\
					void (classname::*callback)(CStubCore *,T *))									\
	{																															\
		CStubIn<T,tp> *p=CStubIn<T,tp>::_New();													\
		p->_owner=(void*)this;																					\
		p->SetName(name);																						\
		p->_bind=NULL;																							\
		p->_dlgtModify.bind(this,callback);																\
		_stubs.push_back(p);																						\
		return (StubID)p;																							\
	}																															\
	template<typename T,StubType tp>																\
	StubID _StubOutBind(const char *name,T *var)												\
	{																															\
		CStubOut<T,tp> *p=CStubOut<T,tp>::_New();											\
		p->_owner=(void*)this;																					\
		p->SetName(name);																						\
		p->Bind(var);																									\
		p->_dlgt=NULL;																								\
		_stubs.push_back(p);																						\
		return (StubID)p;																							\
	}																															\
	template<typename T,StubType tp>																\
	StubID _StubOutBind(const char *name,															\
			void (classname::*callback)(CStubCore *,T *))											\
	{																															\
		CStubOut<T,tp> *p=CStubOut<T,tp>::_New();											\
		p->_owner=(void*)this;																					\
		p->SetName(name);																						\
		p->_dlgt.bind(this,callback);																			\
		p->_bind=NULL;																							\
		_stubs.push_back(p);																						\
		return (StubID)p;																							\
	}																															\
	Stubs _stubs

//predefined stub types
#define StubIn_pos _StubInBind<i_math::vector3df,StubType_Pos>
#define StubOut_pos _StubOutBind<i_math::vector3df,StubType_Pos>

#define StubIn_mat _StubInBind<i_math::matrix43f,StubType_Mat>
#define StubOut_mat _StubOutBind<i_math::matrix43f,StubType_Mat>

#define StubIn_int _StubInBind<int,StubType_Int>
#define StubOut_int _StubOutBind<int,StubType_Int>

//XXXXX:more StubType


#define StubOut_NotifyMod(stubid)		(stubid)->SetModified();


#define LINK_STUB(pObj1,stubname1,pObj2,stubname2)											\
if ((pObj1)&&(pObj2))																									\
{																																		\
	CStubCore *stub1,*stub2;																							\
	stub1=pObj1->FindStub(stubname1);																		\
	stub2=pObj2->FindStub(stubname2);																		\
	if (stub1&&stub2)																										\
		stub1->Link(stub2);																								\
}





