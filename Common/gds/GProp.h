#pragma once

#include <string>


#include "../class/class.h"
#include "gobj.h"
#include "GStub.h"

//////////////////////////////////////////////////////////////////////////
//使用以下的macro来定义/存取各种基本类型的property

//函数: BOOL prop_##name(BOOL bSet)
//    GPropVoid(name,desc)																							
//    GStubVoid(name,desc)																							
//    GStubFireVoid(name)		

//函数: BOOL prop_##name(BOOL bSet,const char *&str)
//    GPropString(name,defval,sem,desc)		
//    GStubString(name,defval,sem,desc)		
//    GPropSetString(owner,name,value)		
//    GPropGetString(owner,name,value)		
//    GStubFireString(name,value)		

//函数: BOOL prop_##name(BOOL bSet,int &v)
//    GPropInt(name,defval,sem,desc)		
//    GStubInt(name,defval,sem,desc)		
//    GPropSetInt(owner,name,value)		
//    GPropGetInt(owner,name,value)		
//    GStubFireInt(name,value)		

//函数: BOOL prop_##name(BOOL bSet,DWORD &v)
//    GPropDword(name,defval,sem,desc)		
//    GStubDword(name,defval,sem,desc)		
//    GPropSetDword(owner,name,value)		
//    GPropGetDword(owner,name,value)		
//    GStubFireDword(name,value)		

//函数: BOOL prop_##name(BOOL bSet,float &v)
//    GPropFloat(name,defval,sem,desc)		
//    GStubFloat(name,defval,sem,desc)		
//    GPropSetFloat(owner,name,value)		
//    GPropGetFloat(owner,name,value)		
//    GStubFireFloat(name,value)		

//函数: BOOL prop_##name(BOOL bSet,i_math::vector2df &v)
//    GPropVector2(name,defval,sem,desc)		
//    GStubVector2(name,defval,sem,desc)		
//    GPropSetVector2(owner,name,value)		
//    GPropGetVector2(owner,name,value)		
//    GStubFireVector2(name,value)		

//函数: BOOL prop_##name(BOOL bSet,i_math::vector3df &v)
//    GPropVector3(name,defval,sem,desc)		
//    GStubVector3(name,defval,sem,desc)		
//    GPropSetVector3(owner,name,value)		
//    GPropGetVector3(owner,name,value)		
//    GStubFireVector3(name,value)		

//函数: BOOL prop_##name(BOOL bSet,i_math::vector4df &v)
//    GPropVector4(name,defval,sem,desc)		
//    GStubVector4(name,defval,sem,desc)		
//    GPropSetVector4(owner,name,value)		
//    GPropGetVector4(owner,name,value)		
//    GStubFireVector4(name,value)		

//函数: BOOL prop_##name(BOOL bSet,i_math::aabbox3df &v)
//    GPropVector6(name,defval,sem,desc)		
//    GStubVector6(name,defval,sem,desc)		
//    GPropSetVector6(owner,name,value)		
//    GPropGetVector6(owner,name,value)		
//    GStubFireVector6(name,value)		

//函数: BOOL prop_##name(BOOL bSet,i_math::matrix43f &v)
//    GPropMat43(name,defval,sem,desc)		
//    GStubMat43(name,defval,sem,desc)		
//    GPropSetMat43(owner,name,value)		
//    GPropGetMat43(owner,name,value)		
//    GStubFireMat43(name,value)		

//函数: BOOL prop_##name(BOOL bSet,i_math::matrix44f &v)
//    GPropMat44(name,defval,sem,desc)		
//    GStubMat44(name,defval,sem,desc)		
//    GPropSetMat44(owner,name,value)		
//    GPropGetMat44(owner,name,value)		
//    GStubFireMat44(name,value)		

//函数: BOOL prop_##name(BOOL bSet,i_math::vector4di &v)
//    GPropInt4(name,defval,sem,desc)		
//    GStubInt4(name,defval,sem,desc)		
//    GPropSetInt4(owner,name,value)		
//    GPropGetInt4(owner,name,value)		
//    GStubFireInt4(name,value)		

//函数: BOOL prop_##name(BOOL bSet,i_math::vector2di &v)
//    GPropInt2(name,defval,sem,desc)		
//    GStubInt2(name,defval,sem,desc)		
//    GPropSetInt2(owner,name,value)		
//    GPropGetInt2(owner,name,value)		
//    GStubFireInt2(name,value)		

//函数: BOOL prop_##name(BOOL bSet,i_math::vector4db &v)
//    GPropByte4(name,defval,sem,desc)		
//    GStubByte4(name,defval,sem,desc)		
//    GPropSetByte4(owner,name,value)		
//    GPropGetByte4(owner,name,value)		
//    GStubFireByte4(name,value)		


//XXXXX:more simple type property

//Sample:
//				class CSample
//				{
//				public:
//					CSample()
//					{
//						v1=0;
//						v2=0;
//					}
//				protected:
//					GStubBegin(CSample);
//
//						GStubDword(Test1,100,GSem_Unknown,"");
//						GStubInt(Test2,100,GSem_Unknown,"");
//
//						GStubVoid(Test3,"");
//						GStubString(Test4,"aaa",GSem_TexturePath,"");
//
//
//					GStubEnd();
//
//					BOOL prop_Test1(BOOL bSet,DWORD &v)
//					{
//						if (bSet)
//						{
//							v1=v;
//							GStubTrigger("Test1");
//						}
//						else
//							v=v1;
//						return TRUE;
//					}
//
//					BOOL prop_Test2(BOOL bSet,DWORD &v)
//					{
//						if (bSet)
//						{
//							v2=v;
//							GStubTrigger("Test2");
//						}
//						else
//							v=v2;
//						return TRUE;
//					}
//
//					BOOL prop_Test3(BOOL bSet)
//					{
//						return TRUE;
//					}
//
//					BOOL prop_Test4(BOOL bSet,const char *&str)
//					{
//						if (bSet)
//							path=str;
//						else
//							str=path.c_str();
//						return TRUE;
//					}
//
//
//					DWORD v1;
//					int v2;
//
//					std::string path;
//
//
//				};



#define GProp_SimpleType(type,name,defval,sem,desc)													\
{																																				\
	GPropDefine(name,type)																									\
	type prop;																															\
	prop.v=defval;																													\
	GPropSetDefault(prop);																										\
	GPropSetSem(sem);																											\
	GPropSetDesc(desc);																											\
}

#define GStub_SimpleType(type,name,defval,sem,desc)													\
{																																				\
	GStubDefine(name,type)																									\
	type prop;																															\
	prop.v=defval;																													\
	GPropSetDefault(prop);																										\
	GPropSetSem(sem);																											\
	GPropSetDesc(desc);																											\
}

#define GSignal_SimpleType(type,name,defval,sem,desc)													\
{																																				\
	GSignalDefine(name,type)																								\
	type prop;																															\
	prop.v=defval;																													\
	GPropSetDefault(prop);																										\
	GPropSetSem(sem);																											\
	GPropSetDesc(desc);																											\
}


#define Prop_SimpleType(classname,type,gvt)																\
struct classname:public GProperty																					\
{																																			\
	classname(type t)																											\
	{																																		\
		GConstructor();																											\
		v=t;																																\
	}																																		\
	type v;																																\
	virtual GVarType GetGVT()	{		return gvt;	}															\
	DECLARE_CLASS(classname);																						\
																																			\
	BEGIN_GOBJ_PURE(classname,1);																					\
		GELEM_VAR(type,v);																									\
			GELEM_EDITVAR("",gvt,GSem_Unknown,"")														\
	END_GOBJ();    																												\
};																																			\
inline BOOL GPropGet##classname(void *owner,GStubBase *stb,type &ret)				\
{																																			\
	if (!stb)																																\
		return FALSE;																												\
	classname *prop=(classname *)stb->GetProp(owner);												\
	if (!prop)																															\
		return FALSE;																												\
	ret=prop->v;																													\
	return TRUE;																													\
}

#define GPropSet_SimpleType(type,owner,name,value)													\
{																																			\
	type t;																																\
	t.v=value;																														\
	GPropSet(owner,name,t);																								\
}

#define GPropGet_SimpleType(type,owner,name,value)												\
				GPropGet##type(owner->GetStubOwner(),owner->FindStub(name),value);

#define GStubFire_SimpleType(type,name,value)															\
{																																			\
	type t;																																\
	t.v=value;																														\
	GStubFire(name,t);																											\
}


struct Prop_Void:public GProperty
{
	DECLARE_CLASS(Prop_Void);

	virtual GVarType GetGVT()	{		return GVT_Void;	}

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(Prop_Void,1);
	END_GOBJ();    
};
#define GPropVoid(name,desc)																							\
	GPropDefine(name,Prop_Void)																							\
	GPropSetDesc(desc);
#define GStubVoid(name,desc)																							\
	GStubDefine(name,Prop_Void)																							\
	GPropSetDesc(desc);
#define GSignalVoid(name,desc)																							\
	GSignalDefine(name,Prop_Void)																						\
	GPropSetDesc(desc);
#define GStubFireVoid(name)																								\
{																																				\
	Prop_Void t;																														\
	GStubFire(name,t);																												\
}

struct Prop_String:public GProperty
{
	Prop_String(const char *s)
	{
		GConstructor();
		v=s;
	}
	std::string v;
	virtual GVarType GetGVT()	{		return GVT_String;	}
	DECLARE_CLASS(Prop_String);

	// GObj Defination --------------------------------------------------
	BEGIN_GOBJ_PURE(Prop_String,1);
		GELEM_STRING_INIT(v,"");
			GELEM_EDITVAR("",GVT_String,GSem_Unknown,"")
	END_GOBJ();    
};
inline BOOL GPropGetProp_String(void *owner,GStubBase *stb,std::string&ret)				\
{																																			\
	if (!stb)																																\
		return FALSE;																												\
	Prop_String *prop=(Prop_String *)stb->GetProp(owner);												\
	if (!prop)																															\
		return FALSE;																												\
	ret=prop->v;																													\
	return TRUE;																													\
}

#define GPropString(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_String,name,defval,sem,desc)
#define GStubString(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_String,name,defval,sem,desc)
#define GSignalString(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_String,name,defval,sem,desc)
#define GPropSetString(owner,name,value)		\
	GPropSet_SimpleType(Prop_String,owner,name,value)
#define GPropGetString(owner,name,value)		\
	GPropGet_SimpleType(Prop_String,owner,name,value)
#define GStubFireString(name,value)		\
	GStubFire_SimpleType(Prop_String,name,value)


Prop_SimpleType(Prop_S,int,GVT_S);
#define GPropInt(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_S,name,defval,sem,desc)
#define GStubInt(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_S,name,defval,sem,desc)
#define GSignalInt(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_S,name,defval,sem,desc)
#define GPropSetInt(owner,name,value)		\
	GPropSet_SimpleType(Prop_S,owner,name,value)
#define GPropGetInt(owner,name,value)		\
	GPropGet_SimpleType(Prop_S,owner,name,value)
#define GStubFireInt(name,value)		\
	GStubFire_SimpleType(Prop_S,name,value)

Prop_SimpleType(Prop_U,DWORD,GVT_U);
#define GPropDword(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_U,name,defval,sem,desc)
#define GStubDword(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_U,name,defval,sem,desc)
#define GSignalDword(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_U,name,defval,sem,desc)
#define GPropSetDword(owner,name,value)		\
	GPropSet_SimpleType(Prop_U,owner,name,value)
#define GPropGetDword(owner,name,value)		\
	GPropGet_SimpleType(Prop_U,owner,name,value)
#define GStubFireDword(name,value)		\
	GStubFire_SimpleType(Prop_U,name,value)

Prop_SimpleType(Prop_F,float,GVT_F);
#define GPropFloat(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_F,name,defval,sem,desc)
#define GStubFloat(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_F,name,defval,sem,desc)
#define GSignalFloat(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_F,name,defval,sem,desc)
#define GPropSetFloat(owner,name,value)		\
	GPropSet_SimpleType(Prop_F,owner,name,value)
#define GPropGetFloat(owner,name,value)		\
	GPropGet_SimpleType(Prop_F,owner,name,value)
#define GStubFireFloat(name,value)		\
	GStubFire_SimpleType(Prop_F,name,value)

Prop_SimpleType(Prop_Fx2,i_math::vector2df,GVT_Fx2);
#define GPropVector2(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_Fx2,name,defval,sem,desc)
#define GStubVector2(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_Fx2,name,defval,sem,desc)
#define GSignalVector2(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_Fx2,name,defval,sem,desc)
#define GPropSetVector2(owner,name,value)		\
	GPropSet_SimpleType(Prop_Fx2,owner,name,value)
#define GPropGetVector2(owner,name,value)		\
	GPropGet_SimpleType(Prop_Fx2,owner,name,value)
#define GStubFireVector2(name,value)		\
	GStubFire_SimpleType(Prop_Fx2,name,value)

Prop_SimpleType(Prop_Fx3,i_math::vector3df,GVT_Fx3);
#define GPropVector3(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_Fx3,name,defval,sem,desc)
#define GStubVector3(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_Fx3,name,defval,sem,desc)
#define GSignalVector3(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_Fx3,name,defval,sem,desc)
#define GPropSetVector3(owner,name,value)		\
	GPropSet_SimpleType(Prop_Fx3,owner,name,value)
#define GPropGetVector3(owner,name,value)		\
	GPropGet_SimpleType(Prop_Fx3,owner,name,value)
#define GStubFireVector3(name,value)		\
	GStubFire_SimpleType(Prop_Fx3,name,value)

Prop_SimpleType(Prop_Fx4,i_math::vector4df,GVT_Fx4);
#define GPropVector4(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_Fx4,name,defval,sem,desc)
#define GStubVector4(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_Fx4,name,defval,sem,desc)
#define GSignalVector4(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_Fx4,name,defval,sem,desc)
#define GPropSetVector4(owner,name,value)		\
	GPropSet_SimpleType(Prop_Fx4,owner,name,value)
#define GPropGetVector4(owner,name,value)		\
	GPropGet_SimpleType(Prop_Fx4,owner,name,value)
#define GStubFireVector4(name,value)		\
	GStubFire_SimpleType(Prop_Fx4,name,value)


Prop_SimpleType(Prop_Fx6,i_math::aabbox3df,GVT_Fx6);
#define GPropVector6(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_Fx6,name,defval,sem,desc)
#define GStubVector6(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_Fx6,name,defval,sem,desc)
#define GSignalVector6(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_Fx6,name,defval,sem,desc)
#define GPropSetVector6(owner,name,value)		\
	GPropSet_SimpleType(Prop_Fx6,owner,name,value)
#define GPropGetVector6(owner,name,value)		\
	GPropGet_SimpleType(Prop_Fx6,owner,name,value)
#define GStubFireVector6(name,value)		\
	GStubFire_SimpleType(Prop_Fx6,name,value)


Prop_SimpleType(Prop_Fx12,i_math::matrix43f,GVT_Fx12);
#define GPropMat43(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_Fx12,name,defval,sem,desc)
#define GStubMat43(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_Fx12,name,defval,sem,desc)
#define GSignalMat43(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_Fx12,name,defval,sem,desc)
#define GPropSetMat43(owner,name,value)		\
	GPropSet_SimpleType(Prop_Fx12,owner,name,value)
#define GPropGetMat43(owner,name,value)		\
	GPropGet_SimpleType(Prop_Fx12,owner,name,value)
#define GStubFireMat43(name,value)		\
	GStubFire_SimpleType(Prop_Fx12,name,value)

Prop_SimpleType(Prop_Fx16,i_math::matrix44f,GVT_Fx16);
#define GPropMat44(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_Fx16,name,defval,sem,desc)
#define GStubMat44(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_Fx16,name,defval,sem,desc)
#define GSignalMat44(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_Fx16,name,defval,sem,desc)
#define GPropSetMat44(owner,name,value)		\
	GPropSet_SimpleType(Prop_Fx16,owner,name,value)
#define GPropGetMat44(owner,name,value)		\
	GPropGet_SimpleType(Prop_Fx16,owner,name,value)
#define GStubFireMat44(name,value)		\
	GStubFire_SimpleType(Prop_Fx16,name,value)

Prop_SimpleType(Prop_Sx4,i_math::vector4di,GVT_Sx4);
#define GPropInt4(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_Sx4,name,defval,sem,desc)
#define GStubInt4(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_Sx4,name,defval,sem,desc)
#define GSignalInt4(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_Sx4,name,defval,sem,desc)
#define GPropSetInt4(owner,name,value)		\
	GPropSet_SimpleType(Prop_Sx4,owner,name,value)
#define GPropGetInt4(owner,name,value)		\
	GPropGet_SimpleType(Prop_Sx4,owner,name,value)
#define GStubFireInt4(name,value)		\
	GStubFire_SimpleType(Prop_Sx4,name,value)

Prop_SimpleType(Prop_Sx2,i_math::vector2di,GVT_Sx2);
#define GPropInt2(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_Sx2,name,defval,sem,desc)
#define GStubInt2(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_Sx2,name,defval,sem,desc)
#define GSignalInt2(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_Sx2,name,defval,sem,desc)
#define GPropSetInt2(owner,name,value)		\
	GPropSet_SimpleType(Prop_Sx2,owner,name,value)
#define GPropGetInt2(owner,name,value)		\
	GPropGet_SimpleType(Prop_Sx2,owner,name,value)
#define GStubFireInt2(name,value)		\
	GStubFire_SimpleType(Prop_Sx2,name,value)

Prop_SimpleType(Prop_Bx4,i_math::vector4db,GVT_Bx4);
#define GPropByte4(name,defval,sem,desc)		\
	GProp_SimpleType(Prop_Bx4,name,defval,sem,desc)
#define GStubByte4(name,defval,sem,desc)		\
	GStub_SimpleType(Prop_Bx4,name,defval,sem,desc)
#define GSignalByte4(name,defval,sem,desc)		\
	GSignal_SimpleType(Prop_Bx4,name,defval,sem,desc)
#define GPropSetByte4(owner,name,value)		\
	GPropSet_SimpleType(Prop_Bx4,owner,name,value)
#define GPropGetByte4(owner,name,value)		\
	GPropGet_SimpleType(Prop_Bx4,owner,name,value)
#define GStubFireByte4(name,value)		\
	GStubFire_SimpleType(Prop_Bx4,name,value)

//XXXXX:more simple type property


inline BOOL Prop_ParseNumber(GProperty *prop,double &v)
{
	v=0.0;
	if (!prop)
		return FALSE;
	switch(prop->GetGVT())
	{
	case GVT_S:
		v=(double)(((Prop_S*)prop)->v);return TRUE;
	case GVT_U:
		v=(double)(((Prop_U*)prop)->v);return TRUE;
	case GVT_F:
		v=(double)(((Prop_F*)prop)->v);return TRUE;
	}
	return FALSE;
}

inline BOOL Prop_ParseDword(GProperty *prop,DWORD &v)
{
	v=0;
	if (!prop)
		return FALSE;
	switch(prop->GetGVT())
	{
		case GVT_S:
			v=(DWORD)(((Prop_S*)prop)->v);return TRUE;
		case GVT_U:
			v=(DWORD)(((Prop_U*)prop)->v);return TRUE;
	}
	return FALSE;
}