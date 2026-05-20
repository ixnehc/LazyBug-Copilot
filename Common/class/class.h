#pragma once

#include <vector>
#include <map>

#include "../mempool/classpool.h"

#define MAX_CLASS_LIB 16

typedef void* (*CLASS_NEW)();		//Alloc memory for an asset
typedef void (*CLASS_DELETE)(void*);		//free the memory for the asset

typedef DWORD ClassID;
#define ClassID_Null 0

typedef DWORD ClassUID;

class CClass
{
public:
	CClass();
	const char *GetName()	{		return _classname.c_str();	}
	void SetName(const char *classname);
	BOOL CheckName(const char *name)	{		return _classname==name;	}
	void SetBaseName(const char *basename);
	BOOL CheckBaseName(const char *name)	{		return _basename==name;	}
	ClassID GetID()	{		return _classid;	}
	ClassID GetUID()	{		return _uid;	}
	ClassID GetBaseID();
	CClass *GetBase();
	DWORD GetDepth();
	BOOL IsSameWith(CClass *other)	{		return _classname==other->_classname;	}

	void *New();//new an instance of this class

	void **GetInstances(DWORD &c)	{		return _getinstances(c);	}
	BOOL CheckInstance(void *p);//检查p是不是这个class的一个实例
	void FreeInstances()	{		_delall();	}		//释放所有的实例

	//static functions
	static int AllocLib();//return -1 if no more lib to alloc
	static void FreeLib(int lib);

	static CClass *Find(ClassID idClass);
	static CClass *Find(const char *classname,int lib=0);//if could not find in the given lib,
																							//will find it in the default lib

	static void Enum(int lib,DWORD flag,					//enumerate all the class that fully
							std::vector<std::string>&result);	//matching the flag in the given lib
	static void Enum(int lib,DWORD flag,					//enumerate all the class that fully
							std::vector<CClass*>&result);	//matching the flag in the given lib


	static void *New(const char *classname,int lib=0);//
	static void *New(ClassID idClass);

public://take it as protected

	virtual void *_new()=0;
	virtual void _del(void *)=0;
	virtual void _delall()=0;
	virtual void **_getinstances(DWORD &c)=0;

	std::string _classname;
	ClassID _classid;
	std::string _basename;//base class name
	ClassID _baseid;//base class id

	ClassUID _uid;
	
	int _depth;//inherit depth,the most base class is 0,the child class of base class is 1,
					  //and the grand-child of the base class is 2,and so on. a -1 value indicates
					  //that not being calculated yet

	DWORD _flag;//for user-defined flag
	int _lib;//Belongs to which lib,default is 0,representing the global lib


	static std::vector<CClass *>& _classes();
	static std::map<std::string,CClass*> *CClass::_classmap();
	static BOOL *_libempty();
	static DWORD &_nlib();
};

#define Class_Ptr(classname) (classname::_clss)
#define Class_Ptr2(classname) (classname::_class())

#define Class_New(classname) (classname*)(classname::_clss->New())
#define Class_New2(classname) (classname*)(classname::_class()->New())
#define Class_Delete(p) (p)->GetClass()->_del(p);
#define Safe_Class_Delete(p) {if (p) Class_Delete(p);(p)=NULL;}

#define IsClass(p,classname) (Class_Ptr(classname)==(p)->GetClass())
#define IsClass2(p,classname) (Class_Ptr2(classname)==(p)->GetClass())


#define CNull void

#define _DECLARE_CLASS_BEGIN(__c,poolclss,clss,baseclss)											\
public:																														\
	typedef poolclss<clss> MemPoolClass;														\
	class CClass_##clss:public __c																			\
	{																															\
	public:																													\
	virtual void *_new()		{			extern MemPoolClass g_pool##clss;return g_pool##clss.Alloc();		}							\
	virtual void _del(void *p)		{	extern MemPoolClass g_pool##clss;g_pool##clss.Free((clss*)p);		}					\
	virtual void _delall()		{			extern MemPoolClass g_pool##clss;g_pool##clss.FreeAll();		}									\
	virtual void **_getinstances(DWORD &c)		{			extern MemPoolClass g_pool##clss;return (void**)g_pool##clss.GetInstances(c);		}		\
	};																															\
	static CClass_##clss *_instantiate()																	\
	{																															\
		static CClass_##clss instance;																		\
		instance.SetName(#clss);																				\
		if (strcmp(#baseclss,"void")!=0)																	\
			instance.SetBaseName(#baseclss);															\
		clss *q=NULL;	baseclss*p=q;	q=(clss *)p;

#define _DECLARE_CLASS_END(clss)																	\
		return &instance;																							\
	}																															\
	static CClass_##clss *_clss;																				\
	static CClass *_class()																							\
	{																															\
		return _clss;																										\
	}																															\
	CClass *GetClass(){	return _clss;}


#define DECLARE_CLASS_DERIVED(clss,baseclss)																		\
		_DECLARE_CLASS_BEGIN(CClass,CClassPool,clss,baseclss)		\
		_DECLARE_CLASS_END(clss)		   

#define DECLARE_CLASS(clss)		DECLARE_CLASS_DERIVED(clss,void)



#define _DEFINE_CLASS_BEGIN(__c,poolclss,clss,baseclss)									\
public:																														\
	typedef poolclss<clss> MemPoolClass;															\
	class CClass_##clss:public __c																			\
	{																															\
	public:																													\
		MemPoolClass &_pool()	{		static MemPoolClass pool("Pool_"#clss);		return pool;	}									\
		virtual void *_new()		{			return _pool().Alloc();		}							\
		virtual void _del(void *p)		{	_pool().Free((clss*)p);		}					\
		virtual void _delall()		{			_pool().FreeAll();		}									\
		virtual void **_getinstances(DWORD &c)		{			return (void **)_pool().GetInstances(c);		}									\
	};																															\
	static CClass_##clss *_instantiate()																	\
	{																															\
		static CClass_##clss instance;																		\
		instance.SetName(#clss);																				\
		if (strcmp(#baseclss,"void")!=0)																	\
			instance.SetBaseName(#baseclss);															\
		clss *q=NULL;	baseclss*p=q;	q=(clss *)p;

#define _DEFINE_CLASS_END(clss)																		\
		return &instance;																							\
	}																															\
	static CClass *_class()																							\
	{																															\
		static CClass *clss=_instantiate();																	\
		return clss;																										\
	}																															\
	virtual CClass *GetClass()																					\
	{																															\
		return _class();																								\
	}
	template <typename T>
	T *ToPtr()
	{
		if (GetClass()->IsSameWith(Class_Ptr2(T)))
			return (T*)this;
		return NULL;
	}

//注意:DEFINE_CLASS()和DECLARE_CLASS()的区别:
//DEFINE_CLASS()不需要在CPP文件里写IMPLEMENT_CLASS()
//但是,你无法使用CClass::New(...)来分配这样一个对象(你可以使用Class_New2()),CClass::Enum()也枚举不到它
#define DEFINE_CLASS_DERIVED(clss,baseclss)																		\
		_DEFINE_CLASS_BEGIN(CClass,CClassPool,clss,baseclss)																	\
		_DEFINE_CLASS_END(clss)		   

#define DEFINE_CLASS(clss)		DEFINE_CLASS_DERIVED(clss,void)

//MP代表MemPool,使用CMemPool作为实例的分配池
#define DEFINE_CLASS_MP(clss)																\
_DEFINE_CLASS_BEGIN(CClass,CMemPool,clss,void)																	\
_DEFINE_CLASS_END(clss)		   



#define IMPLEMENT_CLASS(clss)																			\
	clss::MemPoolClass g_pool##clss("Pool_"#clss);											\
	clss::CClass_##clss * clss::_clss=clss::_instantiate();

#define IMPLEMENT_NEST_CLASS(clssOwner,clss)																							\
	clssOwner::clss::MemPoolClass g_pool##clss("Pool_"#clss);												\
	clssOwner::clss::CClass_##clss *clssOwner::clss::_clss=clssOwner::clss::_instantiate();




