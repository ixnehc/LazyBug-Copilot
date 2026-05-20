#pragma once

#include "class/class.h"

#include "LevelDefines.h"

#include "LevelObj.h"

struct LevelUnitArg
{
	LevelUnitArg()
	{
		int v=0;
		v++;
	}
	virtual CClass *GetClass()=0;
	virtual GObjBase*GetGObj()=0;
	virtual BOOL IsNull()	{		return FALSE;	}
};

#define GELEM_DYNOBJPTR_UNITPARAM(type,name0,initclss,editname,editdesc)											\
{																												\
	GElem_DynObjPtr<type>*p=new GElem_DynObjPtr<type>;										\
	p->off=(DWORD)((BYTE*)&ptr->name0-(BYTE*)ptr);						\
	p->sz=sizeof(ptr->name0);																	\
	p->elemname=#name0;																		\
	extern CClass*GetClass_##initclss();														\
	p->init=GetClass_##initclss();																\
	p->sem=GSem(GSem_Unknown,"DynObjPtr");									\
	p->name=editname;																			\
	p->desc=editdesc;																				\
	p->bEditable=TRUE;																			\
	_ELEM_LINK;																						\
}

#define GELEM_DYNOBJPTR_CLASS_UNITPARAM(name,clss)																					\
	extern CClass*GetClass_##clss();														\
	((GElem_DynObjPtrBase*)curelem)->classes[std::string(#clss)]=GetClass_##clss();						\
	((GElem_DynObjPtrBase*)curelem)->names[std::string(#clss)]=std::string(name);


#define BIND_UNITPARAM(clss)													\
	CClass *GetClass_##clss()														\
{																													\
	return Class_Ptr2(clss);															\
}

#define GELEM_DYNOBJPTR_CLASS_UNITPARAM_LIST() \
GELEM_DYNOBJPTR_CLASS_UNITPARAM( "01.n/a", UnitArg_Null);			\
GELEM_DYNOBJPTR_CLASS_UNITPARAM( "02.地狱触手", UnitArg_地狱触手);


struct UnitArg_Null:public LevelUnitArg
{
	DEFINE_CLASS(UnitArg_Null);

	BEGIN_GOBJ_PURE(UnitArg_Null,1);
	END_GOBJ();

	virtual BOOL IsNull()	{		return TRUE;	}
};

struct UnitArg_地狱触手:public LevelUnitArg
{
	DEFINE_CLASS(UnitArg_地狱触手);

	BEGIN_GOBJ_PURE(UnitArg_地狱触手,1);
		GELEM_VARVECTOR(i_math::matrix43f,locsSrc); 
			GELEM_EDITVAR("源位点",GVT_Fx12,GSem(GSem_Unknown,"MatSet"),"源位点");
		GELEM_VARVECTOR(i_math::spheref,locsSense); 
			GELEM_EDITVAR("感知位点",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"感知位点");
		GELEM_VARVECTOR(i_math::spheref,locsAttack); 
			GELEM_EDITVAR("攻击位点",GVT_Fx4,GSem(GSem_Unknown,"SphereSet"),"攻击目标位点");
	END_GOBJ();

	std::vector<i_math::matrix43f> locsSrc;
	std::vector<i_math::spheref> locsSense;
	std::vector<i_math::spheref> locsAttack;


};