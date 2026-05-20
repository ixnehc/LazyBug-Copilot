
#pragma once

#include "class/class.h"
#include "gds/GObj.h"
#include "gds/GStub.h"

//空的百搭,只能用于slot中
struct StbVoid:public GProperty
{
	DEFINE_CLASS(StbVoid);
	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ(StbVoid,1);
	END_GOBJ();    

	virtual BOOL IsSuperb()	{		return TRUE;	}
	virtual GProperty* To(GProperty *dest)	{		return dest;	}
	virtual BOOL From(GProperty *src)	{		return TRUE;	}
};


struct StbParam
{
	enum Type
	{
		None,
		Obj,
		Number,
		String,
	};

	Type type;
	GProperty *obj;
	double v;
	std::string str;
};

class CEntity;
//StbParams用来在stub之间传递参数
//StbParams里可以包含多个数据,每个数据可以有三种类型:Number,std::string,Data. Data就是
//UserData.在本套脚本系统中,所有的UserData的数值都为一个GProperty的指针.
//
//StbParams::To()和StbParams::From()用于和普通GProperty的转换,转换时不会复制
//数据的内容(只会复制数据指针)
//在CLuaMachine的PushParam()/ParseParam()里会进行StbParams和lua的数据转换,转换
//时会复制数据内容
struct StbParams:public GProperty
{
	DECLARE_CLASS(StbParams);

	StbParams()
	{
		bOwnObj=FALSE;
		count=0;
	}
	~StbParams()
	{
		Clear();
	}



	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ(StbParams,1);
	END_GOBJ();    

	virtual BOOL IsSuperb()	{		return TRUE;	}
	virtual GProperty* To(GProperty *dest);
	virtual BOOL From(GProperty *src);

	void Clear();

	void Copy(StbParams *src);

	void Fetch(StbParams *src);

	DWORD GetCount()
	{
		return count;
	}

	StbParam::Type GetType(DWORD idx)
	{
		if (idx>=count)
			return StbParam::None;
		return entries[idx].type;
	}
	BOOL ExistParam(DWORD idx,StbParam::Type tp)
	{
		if (idx>=count)
			return FALSE;
		return (entries[idx].type==tp);
	}
	double GetNumber(DWORD idx)
	{
		if(idx<count)
		{
			StbParam *e=&entries[idx];
			if (e->type==StbParam::Number)
				return e->v;
		}
		return 0;
	}
	const char *GetString(DWORD idx)
	{
		if(idx<count)
		{
			StbParam *e=&entries[idx];
			if (e->type==StbParam::String)
				return e->str.c_str();
		}
		return "";
	}
	GProperty *GetObj(DWORD idx)
	{
		if (idx<count)
		{
			StbParam *e=&entries[idx];
			if (e->type==StbParam::Obj)
				return e->obj;
		}
		return NULL;
	}


	GProperty *GetObj(DWORD idx,const char *classname)
	{
		GProperty *t=GetObj(idx);
		if (!t)
			return NULL;
		if (t->CheckClassName(classname))
			return t;
		return NULL;
	}

	template<typename T>
	T *GetObj(DWORD idx)
	{
		GProperty *t=GetObj(idx);
		if (!t)
			return NULL;

		if (t->CheckClassName(Class_Ptr2(T)->GetName()))
			return (T*)t;
		return NULL;
	}



	BOOL AddNull()
	{
		if (count>=ARRAY_SIZE(entries))
			return FALSE;
		StbParam *e=&entries[count++];
		e->type=StbParam::None;
		return TRUE;
	}

	BOOL Add(const char *str)
	{
		if (count>=ARRAY_SIZE(entries))
			return FALSE;
		StbParam *e=&entries[count++];
		e->type=StbParam::String;
		e->str=str;
		return TRUE;
	}
	BOOL Add(double v)
	{
		if (count>=ARRAY_SIZE(entries))
			return FALSE;
		StbParam *e=&entries[count++];
		e->type=StbParam::Number;
		e->v=v;
		return TRUE;
	}
	BOOL AddObj(GProperty *obj)
	{
		if (count>=ARRAY_SIZE(entries))
			return FALSE;
		StbParam *e=&entries[count++];
		e->type=StbParam::Obj;
		if (bOwnObj)
			e->obj=obj->Clone();
		else
			e->obj=obj;
		return TRUE;
	}

	BOOL SetObj(DWORD idx,GProperty *obj)
	{
		if (idx>=ARRAY_SIZE(entries))
			return FALSE;
		StbParam *e=&entries[idx];
		if (bOwnObj)
		{
			if ((e->obj)&&(e->type==StbParam::Obj))
			{
				e->obj->DeleteThis();
				e->obj=NULL;
			}
		}
		e->type=StbParam::Obj;
		if (bOwnObj)
			e->obj=obj->Clone();
		else
			e->obj=obj;
		return TRUE;
	}


	void SetOwnObj(BOOL bOwnObj_)	{		bOwnObj=bOwnObj_;	}


	BOOL IsEmpty()	{		return count==0;	}


	BOOL bOwnObj;//_Entry里面的data是否属于这个StbParams,(如果属于,这个StbParams Clear时要清除data)
	StbParam entries[8];
	int count;
};

#define StbParams_GetObj(param,idx,classname) (classname*)(param)->GetObj(idx,#classname)



#define StbParam_Begin() BOOL __bOk=TRUE;BOOL __bSilent=FALSE;BOOL __bAllOk=TRUE;

#define StbParam_EnableWarning(bEnable) __bSilent=!bEnable;

#define StbParam_Ok() (__bOk)
#define StbParam_AllOk() (__bAllOk)

#define StbParam_Obj_NoCheck(arg,classname,idx) classname *arg=(classname*)(params)->GetObj(idx,#classname);

#define StbParam_Obj(arg,classname,idx)																														\
classname*arg=NULL;																																						\
{																																															\
	__bOk=TRUE;																																									\
	GProperty*p = (GProperty*)(params)->GetObj(idx);																									\
	if (p&&p->CheckClassName(#classname))																													\
		arg=(classname*)(p);																																					\
	else																																													\
	{																																														\
		__bOk=FALSE;																																								\
		__bAllOk=FALSE;																																						\
		if (!__bSilent)																																								\
		{																																													\
			LuaDebugOutput("Warning","参数错误,第%d个参数必须为%s对象!",idx,#classname);										\
		}																																													\
	}																																														\
}


#define StbParam_float(arg,idx)																																		\
float arg=0;																																											\
{																																															\
	__bOk=TRUE;																																									\
	if (params->GetType(idx)!=StbParam::Number)																											\
	{																																														\
		if (!__bSilent)																																								\
			LuaDebugOutput("Warning","参数错误,第%d个参数必须为数值!",idx);																\
		__bOk=FALSE;																																								\
		__bAllOk=FALSE;																																						\
	}																																														\
	arg=(float)params->GetNumber(idx);																															\
}

#define StbParam_int(arg,idx)																																			\
int arg=0;																																											\
{																																															\
	__bOk=TRUE;																																									\
	if (params->GetType(idx)!=StbParam::Number)																											\
	{																																														\
		if (!__bSilent)																																								\
			LuaDebugOutput("Warning","参数错误,第%d个参数必须为数值!",idx);																\
		__bOk=FALSE;																																								\
		__bAllOk=FALSE;																																						\
	}																																														\
	arg=(int)params->GetNumber(idx);																																\
}

#define StbParam_Dword(arg,idx)																																	\
DWORD arg=0;																																									\
{																																															\
	__bOk=TRUE;																																									\
	if (params->GetType(idx)!=StbParam::Number)																											\
	{																																														\
		if (!__bSilent)																																								\
			LuaDebugOutput("Warning","参数错误,第%d个参数必须为数值!",idx);																\
		__bOk=FALSE;																																								\
		__bAllOk=FALSE;																																						\
	}																																														\
	arg=(DWORD)params->GetNumber(idx);																													\
}


#define StbParam_cstr(arg,idx)																																			\
const char *arg=0;																																								\
{																																															\
	__bOk=TRUE;																																									\
	if (params->GetType(idx)!=StbParam::String)																												\
	{																																														\
		if (!__bSilent)																																								\
			LuaDebugOutput("Warning","参数错误,第%d个参数必须为字符串!",idx+1);														\
		__bOk=FALSE;																																								\
		__bAllOk=FALSE;																																						\
	}																																														\
	arg=params->GetString(idx);																																		\
}

#define StbParam_vector3df(arg,idx)																																\
i_math::vector3df *arg=NULL;																																			\
{																																															\
	StbParam_Obj(t,Prop_Fx3,idx);																																		\
	if (StbParam_Ok())																																							\
		arg=&t->v;																																									\
	else																																													\
		arg=i_math::vector3df::zero();																																	\
}

#define StbParam_vector3df_NoDef(arg,idx)																																\
i_math::vector3df *arg=NULL;																																			\
{																																															\
	StbParam_Obj(t,Prop_Fx3,idx);																																		\
	if (StbParam_Ok())																																							\
		arg=&t->v;																																									\
}

#define StbParam_matrix43f(arg,idx)																																\
i_math::matrix43f *arg=NULL;																																			\
{																																															\
	StbParam_Obj(t,Prop_Fx12,idx);																																	\
	if (StbParam_Ok())																																							\
		arg=&t->v;																																									\
	else																																													\
		arg=i_math::matrix43f::identity();																																\
}
